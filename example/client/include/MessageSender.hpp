#pragma once

#include "MessageParser.hpp"
#include "SetupClient.hpp"

namespace concord::kvbc {

using namespace ::kvbc::messages;

class MessageSender {
 public:
  MessageSender(std::shared_ptr<SetupClient> setupClient);
  ~MessageSender(){};

  MessageParser* getMessageParser() { return msgParser_.get(); }

  void ParseSampleMessage() { msgParser_->parseSampleMessage(); };
  void stopBftClient() { bftClient_->stop(); }
  void createAndSendKVWriteRequest();
  void createAndSendKVReadRequest();
  bool verifyRequestAndReply();
  void sendClientRequest();

 private:
  logging::Logger getLogger() {
    static logging::Logger logger_(logging::getLogger("osexample::MessageSender"));
    return logger_;
  }
  uint64_t generateUniqueSequenceNumber();
  bft::client::ReadConfig createBftClientReadConfig();
  bft::client::WriteConfig createBftClientWriteConfig();
  void sendKVReadRequestMessage(KVRequest& request);
  void sendKVWriteRequestMessage(KVRequest& request);

 private:
  std::unique_ptr<MessageParser> msgParser_;
  std::weak_ptr<SetupClient> setupClient_;
  std::unique_ptr<bft::client::Client> bftClient_;
  // limited to the size lastMilli shifted.
  const u_int64_t last_count_limit = 0x3FFFFF;
  // lastMilliOfUniqueFetchID_ holds the last SN generated,
  uint64_t lastMilliOfUniqueFetchID_ = 0;
  // lastCount used to preserve uniqueness.
  uint32_t lastCountOfUniqueFetchID_ = 0;
};
}  // End of namespace concord::kvbc