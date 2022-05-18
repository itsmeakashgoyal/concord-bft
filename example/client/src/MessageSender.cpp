#include "MessageSender.hpp"

using namespace concord::kvbc;

MessageSender::MessageSender(std::shared_ptr<SetupClient> setupClient) : setupClient_(setupClient) {
  // Create MessageParser object
  msgParser_ = std::make_unique<MessageParser>(setupClient->getSampleMsgFile());

  // Create communication and start bft client
  std::unique_ptr<bft::communication::ICommunication> comm_ptr(setupClient_.lock()->createCommunication());
  if (!comm_ptr) {
    throw std::runtime_error("Failed to create communication");
  }

  bftClient_ = std::make_unique<bft::client::Client>(std::move(comm_ptr), setupClient_.lock()->setupClientConfig());
  bftClient_->setAggregator(std::make_shared<concordMetrics::Aggregator>());
}

uint64_t MessageSender::generateUniqueSequenceNumber() {
  std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
  uint64_t milli = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

  if (milli > lastMilliOfUniqueFetchID_) {
    lastMilliOfUniqueFetchID_ = milli;
    lastCountOfUniqueFetchID_ = 0;
  } else {
    if (lastCountOfUniqueFetchID_ == last_count_limit) {
      LOG_WARN(getLogger(), "SeqNum Counter reached max value");
      lastMilliOfUniqueFetchID_++;
      lastCountOfUniqueFetchID_ = 0;
    } else {  // increase last count to preserve uniqueness.
      lastCountOfUniqueFetchID_++;
    }
  }
  // shift lastMilli by 22 (0x3FFFFF) in order to 'bitwise or' with lastCount
  // and preserve uniqueness and monotonicity.
  uint64_t r = (lastMilliOfUniqueFetchID_ << (64 - 42));
  ConcordAssert(lastCountOfUniqueFetchID_ <= 0x3FFFFF);
  r = r | ((uint64_t)lastCountOfUniqueFetchID_);

  return r;
}

void MessageSender::sendClientRequest() {
  LOG_INFO(getLogger(),
           "START MessageSender::sendClientRequest(), message type:  " << msgParser_->getSampleMessage()->messageType);
  if (msgParser_->getSampleMessage()->messageType == MessageType::READ) {
    createAndSendKVReadRequest();
  } else if (msgParser_->getSampleMessage()->messageType == MessageType::WRITE) {
    createAndSendKVWriteRequest();
  } else if (msgParser_->getSampleMessage()->messageType == MessageType::BOTH) {
    createAndSendKVWriteRequest();
    createAndSendKVReadRequest();
  } else {
    throw std::runtime_error("Invalid message type");
  }
  LOG_INFO(getLogger(), "END MessageSender::sendClientRequest() ");
}

void MessageSender::createAndSendKVWriteRequest() {
  LOG_INFO(getLogger(), "START MessageSender::createAndSendKVWriteRequest() ");
  // Create write KVRequest based on number of operations
  uint16_t numberOfRequest = msgParser_->getSampleMessage()->numberOfOperation;
  LOG_INFO(getLogger(), "MessageSender::createAndSendKVWriteRequest() " << KVLOG(numberOfRequest));
  for (uint16_t index = 0; index < numberOfRequest; index++) {
    LOG_INFO(getLogger(), "MessageSender::createAndSendKVWriteRequest(), request:  " << index);
    KVRequest request;
    msgParser_->createKVWriteRequestMessage(request);

    sendKVWriteRequestMessage(request);
  }
  LOG_INFO(getLogger(), "END MessageSender::createAndSendKVWriteRequest() ");
}

void MessageSender::createAndSendKVReadRequest() {
  LOG_INFO(getLogger(), "START::MessageSender::createAndSendKVReadRequest() ");
  // Create read KVRequest based on number of operations
  uint16_t numberOfRequest = msgParser_->getSampleMessage()->numberOfOperation;
  LOG_INFO(getLogger(), "MessageSender::createAndSendKVReadRequest() " << KVLOG(numberOfRequest));
  for (uint16_t index = 0; index < numberOfRequest; index++) {
    LOG_INFO(getLogger(), "MessageSender::createAndSendKVReadRequest(), request:  " << index);
    KVRequest request;
    msgParser_->createKVReadRequestMessage(request);

    sendKVReadRequestMessage(request);
  }
  LOG_INFO(getLogger(), "END MessageSender::createAndSendKVReadRequest() ");
}

bft::client::WriteConfig MessageSender::createBftClientWriteConfig() {
  std::shared_ptr<SetupClient> setupClient = setupClient_.lock();
  if (setupClient == nullptr) {
    throw std::runtime_error("SetupClient is nullptr");
  }

  bft::client::RequestConfig request_config;
  uint64_t clientReqSeqNum = generateUniqueSequenceNumber();
  LOG_INFO(getLogger(), "START MessageSender::createBftClientWriteConfig() " << KVLOG(clientReqSeqNum));
  request_config.correlation_id =
      std::to_string(clientReqSeqNum) + "-" + std::to_string(msgParser_->getSampleMessage()->senderNodeId);
  request_config.sequence_number = clientReqSeqNum;
  bft::client::WriteConfig write_config{request_config, bft::client::LinearizableQuorum{}};
  return write_config;
}

void MessageSender::sendKVWriteRequestMessage(KVRequest& request) {
  bft::client::Msg msg;
  ::kvbc::messages::serialize(msg, request);

  bft::client::Reply rep;

  try {
    bft::client::WriteConfig write_config = createBftClientWriteConfig();
    rep = bftClient_->send(write_config, std::move(msg));

  } catch (std::exception& e) {
    LOG_WARN(getLogger(), "error while initiating bft request " << e.what());
  }
}

bft::client::ReadConfig MessageSender::createBftClientReadConfig() {
  std::shared_ptr<SetupClient> setupClient = setupClient_.lock();
  if (setupClient == nullptr) {
    throw std::runtime_error("SetupClient is nullptr");
  }

  bft::client::RequestConfig request_config;
  uint64_t clientReqSeqNum = generateUniqueSequenceNumber();
  LOG_INFO(getLogger(), "START MessageSender::createBftClientReadConfig() " << KVLOG(clientReqSeqNum));
  request_config.correlation_id =
      std::to_string(clientReqSeqNum) + "-" + std::to_string(msgParser_->getSampleMessage()->senderNodeId);
  request_config.sequence_number = clientReqSeqNum;
  bft::client::ReadConfig read_config{request_config, bft::client::LinearizableQuorum{}};

  return read_config;
}

void MessageSender::sendKVReadRequestMessage(KVRequest& request) {
  bft::client::Msg msg;
  ::kvbc::messages::serialize(msg, request);

  bft::client::Reply rep;

  try {
    bft::client::ReadConfig read_config = createBftClientReadConfig();
    rep = bftClient_->send(read_config, std::move(msg));

  } catch (std::exception& e) {
    LOG_WARN(getLogger(), "error while initiating bft request " << e.what());
  }
}

bool MessageSender::verifyRequestAndReply() {
  LOG_INFO(getLogger(),
           "MessageSender::verifyRequestAndReply(), allRequests: " << getMessageParser()->getAllRequests().size()
                                                                   << ", allReplies: "
                                                                   << getMessageParser()->getAllReplies().size());
  if (getMessageParser()->getAllRequests().size() != getMessageParser()->getAllReplies().size()) {
    return false;
  }
  return true;
}