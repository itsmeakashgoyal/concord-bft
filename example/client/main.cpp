#include "SetupClient.hpp"
#include "MessageSender.hpp"

using namespace concord::kvbc;

int main(int argc, char **argv) {
  try {
    std::shared_ptr<SetupClient> setupClient = std::make_shared<SetupClient>();
    setupClient->ParseClientArgs(argc, argv);
    logging::initLogger(setupClient->getLogPropertiesFile());
    auto logger = setupClient->getLogger();

    // Create MessageSender object and initialize bft client
    std::unique_ptr<MessageSender> msgSender = std::make_unique<MessageSender>(setupClient);
    msgSender->ParseSampleMessage();

    // create and send client request
    msgSender->sendClientRequest();

    // verify all write and read requests
    if (!msgSender->verifyRequestAndReply()) {
      LOG_ERROR(logger, "Request and reply size mismatch");
    }

    msgSender->stopBftClient();
    LOG_INFO(logger, "All Good Bye ! ");

  } catch (const std::exception &e) {
    LOG_ERROR(GL, "Exception: " << e.what());
  }
}
