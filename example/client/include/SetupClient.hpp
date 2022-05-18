#pragma once

#include <cstdio>
#include <cstring>
#include <sys/param.h>
#include <unistd.h>

#include "communication/CommFactory.hpp"
#include "tests/config/test_comm_config.hpp"
#include "bftclient/bft_client.h"
#include "secrets_manager_impl.h"
#include "secrets_manager_plain.h"
#include "secrets_manager_enc.h"

using namespace bftEngine;
using namespace bft::communication;

namespace concord::kvbc {

struct ClientParams {
  uint16_t clientId = 4;
  uint16_t numOfReplicas = 4;
  uint16_t numOfClients = 1;
  uint16_t numOfFaulty = 1;
  uint16_t numOfSlow = 0;
  bool measurePerformance = false;
  uint16_t get_numOfReplicas() { return (uint16_t)(3 * numOfFaulty + 2 * numOfSlow + 1); }
};

class SetupClient {
 public:
  SetupClient();
  void ParseClientArgs(int argc, char** argv);
  bft::communication::ICommunication* createCommunication();
  void setupClientParams(int argc, char** argv);
  bft::client::ClientConfig setupClientConfig();
  logging::Logger getLogger() const { return logger_; }
  std::string getLogPropertiesFile() const { return logPropsFile_; }
  ClientParams getClientParams() const { return clientParams_; }
  std::string getSampleMsgFile() { return sampleMsgFile_; }

 private:
  std::string logPropsFile_;
  ClientParams clientParams_;
  uint16_t numOfReplicas_;
  logging::Logger logger_;
  std::string sampleMsgFile_;
};
}  // namespace concord::kvbc
