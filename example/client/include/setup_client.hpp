#pragma once

#include <cstdio>
#include <cstring>
#include <sys/param.h>
#include <unistd.h>
#include <yaml-cpp/yaml.h>

#include "KVBCInterfaces.h"
#include "config/test_comm_config.hpp"
#include "config/test_parameters.hpp"
#include "communication/CommFactory.hpp"
#include "config.h"

using namespace bftEngine;
using namespace bft::communication;
using namespace bft::client;

namespace concord::kvbc {

class SetupClient {
 public:
  SetupClient();
  void ParseClientArgs(int argc, char** argv);

  ICommunication* setupCommunicationParams();
  void setupClientParams(int argc, char** argv);

  ClientConfig setupClientConfig();
  bft::communication::ICommunication* getCommunication() const { return communication_.get(); }
  logging::Logger getLogger() const { return logger_; }
  std::string getLogPropertiesFile() const { return logPropsFile_; }
  ClientParams getClientParams() const { return clientParams_; }

 private:
  std::unique_ptr<bft::communication::ICommunication> communication_;
  logging::Logger logger_;
  std::string logPropsFile_;
  ClientParams clientParams_;
  uint16_t numOfReplicas_;
};
}  // namespace concord::kvbc