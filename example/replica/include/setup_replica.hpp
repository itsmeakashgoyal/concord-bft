#pragma once

#include <chrono>
#include <memory>
#include <optional>
#include <utility>
#include <yaml-cpp/yaml.h>
#include "ReplicaConfig.hpp"
#include "communication/ICommunication.hpp"
#include "config/test_parameters.hpp"
#include "Logger.hpp"
#include "MetricsServer.hpp"
#include "storage_factory_interface.h"
#include "PerformanceManager.hpp"
#include "secrets_manager_impl.h"

#ifdef USE_S3_OBJECT_STORE
#include "s3/client.hpp"
#endif

namespace concord::kvbc {

class SetupReplica {
 public:
  static std::unique_ptr<SetupReplica> ParseArgs(int argc, char** argv);
  static void ParseReplicaConfFile(bftEngine::ReplicaConfig& replicaConfig, const YAML::Node& rconfig_yaml);

  template <typename T>
  static void readYamlField(const YAML::Node& yaml, const std::string& index, T& out, bool value_required = false);

  std::unique_ptr<IStorageFactory> GetStorageFactory();
  std::shared_ptr<concord::secretsmanager::ISecretsManagerImpl> GetSecretManager() const { return sm_; }
  const bftEngine::ReplicaConfig& GetReplicaConfig() const { return replicaConfig_; }
  bft::communication::ICommunication* GetCommunication() const { return communication_.get(); }
  concordMetrics::Server& GetMetricsServer() { return metricsServer_; }
  logging::Logger GetLogger() { return logger_; }
  const bool UsePersistentStorage() const { return usePersistentStorage_; }
  std::string getLogPropertiesFile() { return logPropsFile_; }
  std::shared_ptr<concord::performance::PerformanceManager> GetPerformanceManager() { return pm_; }
  std::optional<std::uint32_t> GetCronEntryNumberOfExecutes() const { return cronEntryNumberOfExecutes_; }
  bool AddAllKeysAsPublic() const { return addAllKeysAsPublic_; }

  static inline constexpr auto kCronTableComponentId = 42;
  static inline constexpr auto kTickGeneratorPeriod = std::chrono::seconds{1};

 private:
  SetupReplica(const bftEngine::ReplicaConfig& config,
               std::unique_ptr<bft::communication::ICommunication> comm,
               logging::Logger logger,
               uint16_t metricsPort,
               bool usePersistentStorage,
               const std::string& s3ConfigFile,
               const std::string& logPropsFile,
               const std::optional<std::uint32_t>& cronEntryNumberOfExecutes,
               bool addAllKeysAsPublic)
      : replicaConfig_(config),
        communication_(std::move(comm)),
        logger_(logger),
        metricsServer_(metricsPort),
        usePersistentStorage_(usePersistentStorage),
        s3ConfigFile_(s3ConfigFile),
        logPropsFile_(logPropsFile),
        pm_{std::make_shared<concord::performance::PerformanceManager>()},
        cronEntryNumberOfExecutes_{cronEntryNumberOfExecutes},
        addAllKeysAsPublic_{addAllKeysAsPublic} {}

  SetupReplica() = delete;

  static std::vector<std::string> getKeyDirectories(const std::string& keysRootPath);

  static void setPublicKeysOfClients(const std::string& principalsMapping,
                                     const std::string& keysRootPath,
                                     std::set<std::pair<const std::string, std::set<uint16_t>>>& publicKeysOfClients);

  const bftEngine::ReplicaConfig& replicaConfig_;
  std::unique_ptr<bft::communication::ICommunication> communication_;
  logging::Logger logger_;
  concordMetrics::Server metricsServer_;
  bool usePersistentStorage_;
  std::string s3ConfigFile_;
  std::string logPropsFile_;
  std::shared_ptr<concord::performance::PerformanceManager> pm_ = nullptr;
  std::optional<std::uint32_t> cronEntryNumberOfExecutes_;
  std::shared_ptr<concord::secretsmanager::ISecretsManagerImpl> sm_;
  bool addAllKeysAsPublic_{false};
};

}  // namespace concord::kvbc
