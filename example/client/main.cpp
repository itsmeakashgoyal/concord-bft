#include "setup_client.hpp"

using namespace concord::kvbc;

int main(int argc, char **argv) {
  try {
    std::unique_ptr<SetupClient> setup = std::make_unique<SetupClient>();
    setup->ParseClientArgs(argc, argv);
    logging::initLogger(setup->getLogPropertiesFile());
    auto logger = setup->getLogger();

    // TODO:: create client and start it
    bft::client::Client *bftClient = new bft::client::Client(setup->getCommunication(), setup->setupClientConfig());
    LOG_INFO(GL, "All Good ! ");

  } catch (const std::exception &e) {
    LOG_ERROR(GL, "Exception: " << e.what());
  }
}