#include "SetupClient.hpp"
#include "MsgSender.hpp"

using namespace concord::kvbc;

int main(int argc, char **argv) {
  try {
    LOG_INFO(GL, "All Good 1 ");
    std::shared_ptr<SetupClient> setupClient = std::make_shared<SetupClient>();
    setupClient->ParseClientArgs(argc, argv);
    LOG_INFO(GL, "All Good 2 ");
    logging::initLogger(setupClient->getLogPropertiesFile());
    auto logger = setupClient->getLogger();
    LOG_INFO(logger, "All Good 3 ");

    // Create MsgSender object and initialize bft client
    std::unique_ptr<MsgSender> msgSender = std::make_unique<MsgSender>(setupClient);

    LOG_INFO(logger, "All Good ! ");

  } catch (const std::exception &e) {
    LOG_ERROR(GL, "Exception: " << e.what());
  }
}
