#include "SetupClient.hpp"

using namespace concord::kvbc;

SetupClient::SetupClient() { logPropsFile_ = std::string("logging.properties"); }

void SetupClient::ParseClientArgs(int argc, char** argv) {
  std::stringstream args;
  for (int i{1}; i < argc; ++i) {
    args << argv[i] << " ";
  }
  LOG_INFO(GL, "Parsing" << KVLOG(argc) << " arguments, args:" << args.str());
  logger_ = logging::getLogger("test_client");

  try {
    setupClientParams(argc, argv);

  } catch (const std::exception& e) {
    LOG_FATAL(GL, "failed to parse command line arguments: " << e.what());
    throw;
  }
}

void SetupClient::setupClientParams(int argc, char** argv) {
  clientParams_.clientId = UINT16_MAX;
  clientParams_.numOfFaulty = UINT16_MAX;
  clientParams_.numOfSlow = UINT16_MAX;
  clientParams_.numOfOperations = UINT16_MAX;
  char argTempBuffer[PATH_MAX + 10];
  int o = 0;
  while ((o = getopt(argc, argv, "i:f:c:p:n:r:m:")) != EOF) {
    switch (o) {
      case 'i': {
        strncpy(argTempBuffer, optarg, sizeof(argTempBuffer) - 1);
        argTempBuffer[sizeof(argTempBuffer) - 1] = 0;
        std::string idStr = argTempBuffer;
        int tempId = std::stoi(idStr);
        if (tempId >= 0 && tempId < UINT16_MAX) clientParams_.clientId = (uint16_t)tempId;
      } break;

      case 'f': {
        strncpy(argTempBuffer, optarg, sizeof(argTempBuffer) - 1);
        argTempBuffer[sizeof(argTempBuffer) - 1] = 0;
        std::string fStr = argTempBuffer;
        int tempfVal = std::stoi(fStr);
        if (tempfVal >= 1 && tempfVal < UINT16_MAX) clientParams_.numOfFaulty = (uint16_t)tempfVal;
      } break;

      case 'c': {
        strncpy(argTempBuffer, optarg, sizeof(argTempBuffer) - 1);
        argTempBuffer[sizeof(argTempBuffer) - 1] = 0;
        std::string cStr = argTempBuffer;
        int tempcVal = std::stoi(cStr);
        if (tempcVal >= 0 && tempcVal < UINT16_MAX) clientParams_.numOfSlow = (uint16_t)tempcVal;
      } break;

      case 'p': {
        strncpy(argTempBuffer, optarg, sizeof(argTempBuffer) - 1);
        argTempBuffer[sizeof(argTempBuffer) - 1] = 0;
        std::string numOfOpsStr = argTempBuffer;
        uint32_t tempPVal = std::stoul(numOfOpsStr);
        if (tempPVal >= 1 && tempPVal < UINT32_MAX) clientParams_.numOfOperations = tempPVal;
      } break;

      case 'n': {
        strncpy(argTempBuffer, optarg, sizeof(argTempBuffer) - 1);
        argTempBuffer[sizeof(argTempBuffer) - 1] = 0;
        clientParams_.configFileName = argTempBuffer;
      } break;

      case 'r': {
        strncpy(argTempBuffer, optarg, sizeof(argTempBuffer) - 1);
        argTempBuffer[sizeof(argTempBuffer) - 1] = 0;
        std::string idStr = argTempBuffer;
        int numReplicas = std::stoi(idStr);
        if (numReplicas >= 0 && numReplicas < UINT16_MAX) numOfReplicas_ = (uint16_t)numReplicas;
      } break;

      case 'm': {
        strncpy(argTempBuffer, optarg, sizeof(argTempBuffer) - 1);
        argTempBuffer[sizeof(argTempBuffer) - 1] = 0;
        sampleMsgFile_ = argTempBuffer;
      } break;

      default:
        break;
    }
  }

  if (clientParams_.clientId == UINT16_MAX || clientParams_.numOfFaulty == UINT16_MAX ||
      clientParams_.numOfSlow == UINT16_MAX || clientParams_.numOfOperations == UINT32_MAX) {
    LOG_ERROR(logger_, "Wrong usage! Required parameters: " << argv[0] << " -f F -c C -p NUM_OPS -i ID");
    exit(-1);
  }
}

bft::client::ClientConfig SetupClient::setupClientConfig() {
  bft::client::ClientConfig bftClientConf;
  bftClientConf.f_val = clientParams_.numOfFaulty;
  bftClientConf.c_val = clientParams_.numOfSlow;
  bftClientConf.id = bft::client::ClientId{clientParams_.clientId};
  for (uint16_t i = 0; i < numOfReplicas_; i++) {
    bftClientConf.all_replicas.emplace(bft::client::ReplicaId{i});
  }
  bftClientConf.replicas_master_key_folder_path = std::nullopt;
  return bftClientConf;
}

bft::communication::ICommunication* SetupClient::createCommunication() {
  CommunicationConfig commConfig;
  uint16_t numOfReplicas = clientParams_.get_numOfReplicas();
  std::shared_ptr<concord::secretsmanager::ISecretsManagerImpl> sm_;

#ifdef USE_COMM_PLAIN_TCP
  PlainTcpConfig conf =
      commConfig.GetTCPConfig(false, clientParams_.clientId, clientParams_.numOfClients, numOfReplicas);
#elif USE_COMM_TLS_TCP
  TlsTcpConfig conf =
      commConfig.GetTlsTCPConfig(false, clientParams_.clientId, clientParams_.numOfClients, numOfReplicas);
  if (conf.secretData_.has_value()) {
    sm_ = std::make_shared<concord::secretsmanager::SecretsManagerEnc>(conf.secretData_.value());
  } else {
    sm_ = std::make_shared<concord::secretsmanager::SecretsManagerPlain>();
  }
#else
  PlainUdpConfig conf =
      commConfig.GetUDPConfig(false, clientParams_.clientId, clientParams_.numOfClients, numOfReplicas);
#endif

  return CommFactory::create(conf);
}
