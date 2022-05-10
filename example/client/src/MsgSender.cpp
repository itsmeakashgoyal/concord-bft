#include "MsgSender.hpp"
#include <TimeUtils.hpp>
using namespace concord::kvbc;

MsgSender::MsgSender(std::shared_ptr<SetupClient> setupClient) : setupClient_(setupClient) {
  // Create MsgReader object
  msgReader_ = std::make_unique<MsgReader>(setupClient->getSampleMsgFile());

  // Create communication and start bft client
  std::unique_ptr<bft::communication::ICommunication> comm_ptr(setupClient_.lock()->createCommunication());
  if (!comm_ptr) {
    throw std::runtime_error("Failed to create communication");
  }

  // bft::client::Client* bftClient = new bft::client::Client(std::move(comm_ptr), setupClient->setupClientConfig());
  bftClient_ = std::make_unique<bft::client::Client>(std::move(comm_ptr), setupClient_.lock()->setupClientConfig());
  bftClient_->setAggregator(std::make_shared<concordMetrics::Aggregator>());

  // Create ClientRequestMsg and send it to bft client
  createAndSendClientRequestMsg();
}

// This function is used to create a ClientRequestMsg from SampleMsg
void MsgSender::createAndSendClientRequestMsg() {
  // Create a sample ClientRequestMsg
  // SampleMsg* sampleMsg = msgReader_->getSampleMsg();
  /*auto requestSeqNum =
  std::chrono::duration_cast<std::chrono::microseconds>(getMonotonicTime().time_since_epoch()).count();

  std::string strMsg = "";
  std::string msgCid = std::to_string(sampleMsg->reqSeqNum) + "-" + std::to_string(clientParams.clientId);

  // TODO: commenting because ClientRequestMsg is internally created by bft::client::Client send function.
  unique_ptr<MessageBase> clientRequestMsg = make_unique<ClientRequestMsg>(clientParams.clientId,
                                                                           sampleMsg->clientMsgFlag,
                                                                           requestSeqNum,
                                                                           strMsg.size(),
                                                                           // sampleMsg->request,
                                                                           strMsg.c_str(),
                                                                           sampleMsg->reqTimeoutMilli,
                                                                           msgCid);*/

  // TODO: Create a bft client config because it is internally used to create a client request msg
  LOG_INFO(getLogger(), "akash:: Sending msg: ");
  bft::client::WriteConfig config = createWriteConfig();
  LOG_INFO(getLogger(), "akash:: cid: " << config.request.correlation_id);
  auto reply = bftClient_->send(config, bft::client::Msg({'h', 'e', 'l', 'l', 'o'}));
  for (auto& val : reply.matched_data) {
    LOG_INFO(getLogger(), "akash:: Reply msg: " << val);
  }

  // Stop the bftclient
  // bftClient_->stop();
}

bft::client::WriteConfig MsgSender::createWriteConfig() {
  std::shared_ptr<SetupClient> setupClient = setupClient_.lock();
  if (setupClient == nullptr) {
    throw std::runtime_error("SetupClient is nullptr");
  }
  SampleMsg* sampleMsg = msgReader_->getSampleMsg();
  ClientParams clientParams = setupClient->getClientParams();

  std::string msgCid = std::to_string(sampleMsg->reqSeqNum) + "-" + std::to_string(clientParams.clientId);

  bft::client::WriteConfig single_config;
  single_config.request.timeout = 500ms;
  single_config.request.sequence_number = sampleMsg->reqSeqNum;
  single_config.request.correlation_id = msgCid;
  single_config.quorum = bft::client::LinearizableQuorum{};

  return single_config;
}