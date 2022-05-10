#pragma once

#include "MsgReader.hpp"
#include "SetupClient.hpp"
#include "messages/ClientRequestMsg.hpp"

namespace concord::kvbc {
class MsgSender {
 public:
  MsgSender(std::shared_ptr<SetupClient> setupClient);
  ~MsgSender(){};

  MsgReader* getMsgReader() { return msgReader_.get(); }
  void createAndSendClientRequestMsg();
  bft::client::WriteConfig createWriteConfig();

 private:
  logging::Logger getLogger() {
    static logging::Logger logger_(logging::getLogger("concord::kvbc::MsgSender"));
    return logger_;
  }

 private:
  std::unique_ptr<MsgReader> msgReader_;
  std::weak_ptr<SetupClient> setupClient_;
  std::unique_ptr<bft::client::Client> bftClient_;
};
}  // End of namespace concord::kvbc