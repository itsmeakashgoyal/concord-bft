#include <thread>
#include <sys/param.h>
#include <string>
#include <cstring>
#include <getopt.h>
#include <unistd.h>
#include <tuple>
#include <chrono>
#include <memory>

#include "Logger.hpp"
#include "setup_replica.hpp"
#include "communication/CommFactory.hpp"
#include "common_constants.hpp"
#include "memorydb/client.h"
#include "string.hpp"
#include "config_file_parser.hpp"
#include "direct_kv_storage_factory.h"
#include "merkle_tree_storage_factory.h"
#include "secrets_manager_plain.h"
#include "config/test_comm_config.hpp"

#include <boost/algorithm/string.hpp>
#include <experimental/filesystem>
#include "secrets_manager_enc.h"

#ifdef USE_S3_OBJECT_STORE
#include "s3/config_parser.hpp"
#endif

namespace fs = std::experimental::filesystem;
namespace concord::kvbc {

// Copy a value from the YAML node to `out`.
// Throws and exception if no value could be read but the value is required.
template <typename T>
void SetupReplica::readYamlField(const YAML::Node& yaml, const std::string& index, T& out, bool value_required) {
  try {
    out = yaml[index].as<T>();
  } catch (const std::exception& e) {
    if (value_required) {
      // We ignore the YAML exceptions because they aren't useful
      std::ostringstream msg;
      msg << "Failed to read \"" << index << "\"";
      throw std::runtime_error(msg.str().data());
    } else {
      LOG_INFO(GL, "No value found for \"" << index << "\"");
    }
  }
}

void SetupReplica::ParseReplicaConfFile(bftEngine::ReplicaConfig& replicaConfig, const YAML::Node& rconfig_yaml) {
  // Parsing the replica config file
  readYamlField(rconfig_yaml, "numOfClientProxies", replicaConfig.numOfClientProxies);
  readYamlField(rconfig_yaml, "numOfExternalClients", replicaConfig.numOfExternalClients);
  readYamlField(rconfig_yaml, "concurrencyLevel", replicaConfig.concurrencyLevel);
  readYamlField(rconfig_yaml, "debugStatisticsEnabled", replicaConfig.debugStatisticsEnabled);
  readYamlField(rconfig_yaml, "viewChangeTimerMillisec", replicaConfig.viewChangeTimerMillisec);
  readYamlField(rconfig_yaml, "statusReportTimerMillisec", replicaConfig.statusReportTimerMillisec);
  readYamlField(rconfig_yaml, "preExecutionFeatureEnabled", replicaConfig.preExecutionFeatureEnabled);
  readYamlField(rconfig_yaml, "clientBatchingEnabled", replicaConfig.clientBatchingEnabled);
  readYamlField(rconfig_yaml, "pruningEnabled_", replicaConfig.pruningEnabled_);
  readYamlField(rconfig_yaml, "numBlocksToKeep_", replicaConfig.numBlocksToKeep_);
  readYamlField(rconfig_yaml, "batchedPreProcessEnabled", replicaConfig.batchedPreProcessEnabled);
  readYamlField(rconfig_yaml, "timeServiceEnabled", replicaConfig.timeServiceEnabled);
  readYamlField(rconfig_yaml, "enablePostExecutionSeparation", replicaConfig.enablePostExecutionSeparation);
  readYamlField(rconfig_yaml, "preExecutionResultAuthEnabled", replicaConfig.preExecutionResultAuthEnabled);
  readYamlField(rconfig_yaml, "numOfClientServices", replicaConfig.numOfClientServices);
  readYamlField(rconfig_yaml, "viewChangeProtocolEnabled", replicaConfig.viewChangeProtocolEnabled);
  readYamlField(rconfig_yaml, "autoPrimaryRotationTimerMillisec", replicaConfig.autoPrimaryRotationTimerMillisec);
  readYamlField(rconfig_yaml, "autoPrimaryRotationEnabled", replicaConfig.autoPrimaryRotationEnabled);
  readYamlField(rconfig_yaml, "keyExchangeOnStart", replicaConfig.keyExchangeOnStart);
  readYamlField(rconfig_yaml, "concurrencyLevel", replicaConfig.concurrencyLevel);
  readYamlField(rconfig_yaml, "batchingPolicy", replicaConfig.batchingPolicy);
  readYamlField(rconfig_yaml, "maxBatchSizeInBytes", replicaConfig.maxBatchSizeInBytes);
  readYamlField(rconfig_yaml, "maxNumOfRequestsInBatch", replicaConfig.maxNumOfRequestsInBatch);
  readYamlField(rconfig_yaml, "batchFlushPeriod", replicaConfig.batchFlushPeriod);
  readYamlField(rconfig_yaml, "maxNumberOfDbCheckpoints", replicaConfig.maxNumberOfDbCheckpoints);
  readYamlField(rconfig_yaml, "dbCheckpointDirPath", replicaConfig.dbCheckpointDirPath);
  readYamlField(rconfig_yaml, "clientTransactionSigningEnabled", replicaConfig.clientTransactionSigningEnabled);
  readYamlField(rconfig_yaml, "numOfClientProxies", replicaConfig.numOfClientProxies);
  readYamlField(rconfig_yaml, "numOfExternalClients", replicaConfig.numOfExternalClients);
  readYamlField(rconfig_yaml, "certificatesRootPath", replicaConfig.certificatesRootPath);
  readYamlField(rconfig_yaml, "publishReplicasMasterKeyOnStartup", replicaConfig.publishReplicasMasterKeyOnStartup);
  readYamlField(rconfig_yaml, "blockAccumulation", replicaConfig.blockAccumulation);
  readYamlField(rconfig_yaml, "pathToOperatorPublicKey_", replicaConfig.pathToOperatorPublicKey_);
  readYamlField(rconfig_yaml, "dbCheckpointFeatureEnabled", replicaConfig.dbCheckpointFeatureEnabled);
  readYamlField(rconfig_yaml, "dbCheckPointWindowSize", replicaConfig.dbCheckPointWindowSize);
  replicaConfig.dbSnapshotIntervalSeconds =
      std::chrono::seconds(rconfig_yaml["dbSnapshotIntervalSeconds"].as<uint64_t>());
  replicaConfig.dbCheckpointMonitorIntervalSeconds =
      std::chrono::seconds(rconfig_yaml["dbCheckpointMonitorIntervalSeconds"].as<uint64_t>());
}

std::unique_ptr<SetupReplica> SetupReplica::ParseArgs(int argc, char** argv) {
  // prepare replica config object from custom replica config file which can be read from ParseReplicaConfFile() and
  // command line arguments
  std::stringstream args;
  for (int i{1}; i < argc; ++i) {
    args << argv[i] << " ";
  }
  LOG_INFO(GL, "Parsing" << KVLOG(argc) << " arguments, args:" << args.str());
  logging::Logger logger = logging::getLogger("test_replica");

  try {
    bftEngine::ReplicaConfig& replicaConfig = bftEngine::ReplicaConfig::instance();

    const auto persistMode = PersistencyMode::RocksDB;
    std::string keysFilePrefix;
    std::string commConfigFile = "";
    std::string s3ConfigFile;
    std::string certRootPath;
    std::string logPropsFile;
    std::string principalsMapping;
    std::string txnSigningKeysPath;
    std::string replicaSampleConfFilePath;
    std::optional<std::uint32_t> cronEntryNumberOfExecutes;
    int addAllKeysAsPublic = 0;

    static struct option longOptions[] = {{"replica-id", required_argument, 0, 'i'},
                                          {"replicaSampleConfFilePath", required_argument, 0, 'a'}};

    int o = 0;
    int optionIndex = 0;
    LOG_INFO(GL, "Command line options:");
    while ((o = getopt_long(argc, argv, "i:a:", longOptions, &optionIndex)) != -1) {
      switch (o) {
        case 'i': {
          replicaConfig.replicaId = concord::util::to<std::uint16_t>(std::string(optarg));
        } break;
        case 'a': {
          if (optarg[0] == '-') throw std::runtime_error("invalid argument for replica sample config file");
          replicaSampleConfFilePath = optarg;
          LOG_INFO(GL, " " << KVLOG(replicaSampleConfFilePath));
        } break;

        default:
          break;
      }
    }

    // Parsing replica sample config file to update replica config object
    auto yaml = YAML::LoadFile(replicaSampleConfFilePath);
    ParseReplicaConfFile(replicaConfig, yaml);

    replicaConfig.set("sourceReplicaReplacementTimeoutMilli", 6000);
    replicaConfig.set("concord.bft.st.runInSeparateThread", true);
    replicaConfig.set("concord.bft.keyExchage.clientKeysEnabled", false);
    replicaConfig.set("concord.bft.st.fetchRangeSize", 27);
    replicaConfig.set("concord.bft.st.gettingMissingBlocksSummaryWindowSize", 60);
    replicaConfig.set("concord.bft.st.RVT_K", 12);
    replicaConfig.set("concord.bft.keyExchage.clientKeysEnabled", true);

    readYamlField(yaml, "txnSigningKeysPath", txnSigningKeysPath);
    readYamlField(yaml, "principalsMapping", principalsMapping);
    readYamlField(yaml, "logPropsFile", logPropsFile);
    readYamlField(yaml, "keysFilePrefix", keysFilePrefix);
    readYamlField(yaml, "certRootPath", certRootPath);
    cronEntryNumberOfExecutes = yaml["cronEntryNumberOfExecutes"].as<std::uint32_t>();

    LOG_INFO(GL, "" << KVLOG(keysFilePrefix));
    LOG_INFO(GL, "" << KVLOG(principalsMapping));
    LOG_INFO(GL, "" << KVLOG(txnSigningKeysPath));
    LOG_INFO(GL, "" << KVLOG(certRootPath));

    if (keysFilePrefix.empty()) throw std::runtime_error("missing --key-file-prefix");

    // If principalsMapping and txnSigningKeysPath are set, enable clientTransactionSigningEnabled. If only one of them
    // is set, throw an error
    if (!principalsMapping.empty() && !txnSigningKeysPath.empty()) {
      replicaConfig.clientTransactionSigningEnabled = true;
      SetupReplica::setPublicKeysOfClients(principalsMapping, txnSigningKeysPath, replicaConfig.publicKeysOfClients);
    } else if (principalsMapping.empty() != txnSigningKeysPath.empty()) {
      throw std::runtime_error("Params principals-mapping and txn-signing-key-path must be set simultaneously.");
    }

    TestCommConfig testCommConfig(logger);
    testCommConfig.GetReplicaConfig(replicaConfig.replicaId, keysFilePrefix, &replicaConfig);
    uint16_t numOfReplicas =
        (uint16_t)(3 * replicaConfig.fVal + 2 * replicaConfig.cVal + 1 + replicaConfig.numRoReplicas);
    auto numOfClients =
        replicaConfig.numOfClientProxies ? replicaConfig.numOfClientProxies : replicaConfig.numOfExternalClients;
    std::shared_ptr<concord::secretsmanager::ISecretsManagerImpl> sm_ =
        std::make_shared<concord::secretsmanager::SecretsManagerPlain>();
#ifdef USE_COMM_PLAIN_TCP
    bft::communication::PlainTcpConfig conf =
        testCommConfig.GetTCPConfig(true, replicaConfig.replicaId, numOfClients, numOfReplicas, commConfigFile);
#elif USE_COMM_TLS_TCP
    bft::communication::TlsTcpConfig conf = testCommConfig.GetTlsTCPConfig(
        true, replicaConfig.replicaId, numOfClients, numOfReplicas, commConfigFile, certRootPath);
    if (conf.secretData_.has_value()) {
      sm_ = std::make_shared<concord::secretsmanager::SecretsManagerEnc>(conf.secretData_.value());
    } else {
      sm_ = std::make_shared<concord::secretsmanager::SecretsManagerPlain>();
    }
#else
    bft::communication::PlainUdpConfig conf =
        testCommConfig.GetUDPConfig(true, replicaConfig.replicaId, numOfClients, numOfReplicas, commConfigFile);
#endif
    replicaConfig.certificatesRootPath = certRootPath;
    std::unique_ptr<bft::communication::ICommunication> comm(bft::communication::CommFactory::create(conf));

    uint16_t metricsPort = conf.listenPort_ + 1000;

    LOG_INFO(logger, "\nReplica Configuration: \n" << replicaConfig);
    std::unique_ptr<SetupReplica> setup = nullptr;
    setup.reset(new SetupReplica(replicaConfig,
                                 std::move(comm),
                                 logger,
                                 metricsPort,
                                 persistMode == PersistencyMode::RocksDB,
                                 s3ConfigFile,
                                 logPropsFile,
                                 cronEntryNumberOfExecutes,
                                 addAllKeysAsPublic != 0));
    setup->sm_ = sm_;
    return setup;

  } catch (const std::exception& e) {
    LOG_FATAL(GL, "failed to parse command line arguments: " << e.what());
    throw;
  }
  return nullptr;
}

std::unique_ptr<IStorageFactory> SetupReplica::GetStorageFactory() {
  // TODO handle persistence mode
  std::stringstream dbPath;
  dbPath << CommonConstants::DB_FILE_PREFIX << GetReplicaConfig().replicaId;

#ifdef USE_S3_OBJECT_STORE
  if (GetReplicaConfig().isReadOnly) {
    if (s3ConfigFile_.empty()) throw std::runtime_error("--s3-config-file must be provided");
    const auto s3Config = concord::storage::s3::ConfigFileParser(s3ConfigFile_).parse();
    return std::make_unique<v1DirectKeyValue::S3StorageFactory>(dbPath.str(), s3Config);
  }
#endif
  return std::make_unique<v2MerkleTree::RocksDBStorageFactory>(dbPath.str());
}

std::vector<std::string> SetupReplica::getKeyDirectories(const std::string& path) {
  std::vector<std::string> result;
  if (!fs::exists(path) || !fs::is_directory(path)) {
    throw std::invalid_argument{"Transaction signing keys path doesn't exist at " + path};
  }
  for (auto& dir : fs::directory_iterator(path)) {
    if (fs::exists(dir) && fs::is_directory(dir)) {
      result.push_back(dir.path().string());
    }
  }
  return result;
}

void SetupReplica::setPublicKeysOfClients(
    const std::string& principalsMapping,
    const std::string& keysRootPath,
    std::set<std::pair<const std::string, std::set<uint16_t>>>& publicKeysOfClients) {
  // The string principalsMapping looks like:
  // "11 12;13 14;15 16;17 18;19 20", for 10 client ids divided into 5 participants.

  LOG_INFO(GL, "" << KVLOG(principalsMapping, keysRootPath));
  std::vector<std::string> keysDirectories = SetupReplica::getKeyDirectories(keysRootPath);
  std::vector<std::string> clientIdChunks;
  boost::split(clientIdChunks, principalsMapping, boost::is_any_of(";"));

  if (clientIdChunks.size() != keysDirectories.size()) {
    std::stringstream msg;
    msg << "Number of keys directory should match the number of sets of clientIds mappings in principals "
           "mapping string "
        << KVLOG(keysDirectories.size(), clientIdChunks.size(), principalsMapping);
    throw std::runtime_error(msg.str());
  }

  // Sort keysDirectories just to ensure ordering of the folders
  std::sort(keysDirectories.begin(), keysDirectories.end());

  // Build publicKeysOfClients of replicaConfig
  for (std::size_t i = 0; i < clientIdChunks.size(); ++i) {
    std::string keyFilePath =
        keysDirectories.at(i) + "/transaction_signing_pub.pem";  // Will be "../1/transaction_signing_pub.pem" etc.

    secretsmanager::SecretsManagerPlain smp;
    std::optional<std::string> keyPlaintext;
    keyPlaintext = smp.decryptFile(keyFilePath);
    if (keyPlaintext.has_value()) {
      // Each clientIdChunk would look like "11 12 13 14"
      std::vector<std::string> clientIds;
      boost::split(clientIds, clientIdChunks.at(i), boost::is_any_of(" "));
      std::set<uint16_t> clientIdsSet;
      for (const std::string& clientId : clientIds) {
        clientIdsSet.insert(std::stoi(clientId));
      }
      const std::string key = keyPlaintext.value();
      publicKeysOfClients.insert(std::pair<const std::string, std::set<uint16_t>>(key, clientIdsSet));
      bftEngine::ReplicaConfig::instance().clientGroups.emplace(i + 1, clientIdsSet);
    } else {
      throw std::runtime_error("Key public_key.pem not found in directory " + keysDirectories.at(i));
    }
  }
  bftEngine::ReplicaConfig::instance().clientsKeysPrefix = keysRootPath;
}
}  // namespace concord::kvbc
