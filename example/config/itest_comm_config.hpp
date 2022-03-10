#pragma once

#include <string>

#include "Logger.hpp"
#include "communication/CommDefs.hpp"
#include "ReplicaConfig.hpp"

class ITestCommConfig {
 public:
  explicit ITestCommConfig(logging::Logger& logger) : logger_(logger) {}
  virtual ~ITestCommConfig() = default;

  // Create a replica config for the replica with index `replicaId`.
  // inputReplicaKeyfile is used to read the keys for this replica, and
  // default values are loaded for non-cryptographic configuration parameters.
  virtual void GetReplicaConfig(uint16_t replica_id,
                                std::string keyFilePrefix,
                                bftEngine::ReplicaConfig* out_config) = 0;

  // Create a UDP communication configuration for the node (replica or client)
  // with index `id`.
  virtual bft::communication::PlainUdpConfig GetUDPConfig(bool is_replica,
                                                          uint16_t id,
                                                          uint16_t& num_of_clients,
                                                          uint16_t& num_of_replicas,
                                                          const std::string& config_file_name) = 0;

  // Create a UDP communication configuration for the node (replica or client)
  // with index `id`.
  virtual bft::communication::PlainTcpConfig GetTCPConfig(bool is_replica,
                                                          uint16_t id,
                                                          uint16_t& num_of_clients,
                                                          uint16_t& num_of_replicas,
                                                          const std::string& config_file_name) = 0;

  virtual bft::communication::TlsTcpConfig GetTlsTCPConfig(bool is_replica,
                                                           uint16_t id,
                                                           uint16_t& num_of_clients,
                                                           uint16_t& num_of_replicas,
                                                           const std::string& config_file_name,
                                                           const std::string& cert_root_path) = 0;

 protected:
  logging::Logger& logger_;
};
