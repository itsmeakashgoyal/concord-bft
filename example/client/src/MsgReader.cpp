#include "MsgReader.hpp"

using namespace concord::kvbc;

// Parsing sample msg file to create a ClientRequestMsg
void MsgReader::parseSampleMsg() {
  auto yaml = YAML::LoadFile(sampleMsgFileName_);

  sampleMsg_ = std::make_unique<SampleMsg>();

  // Preparing SampleMsg object
  sampleMsg_->senderNodeId = yaml["sender_node_id"].as<std::uint16_t>();
  sampleMsg_->clientMsgFlag = yaml["client_msg_flag"].as<std::uint64_t>();
  sampleMsg_->reqSeqNum = yaml["req_seqNum"].as<std::uint64_t>();
  sampleMsg_->requestLength = yaml["request_length"].as<std::uint32_t>();
  sampleMsg_->request = yaml["request"].as<std::string>();
  sampleMsg_->reqTimeoutMilli = yaml["req_timeout_milli"].as<std::uint64_t>();
}
