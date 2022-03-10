#pragma once

#include "itest_comm_config.hpp"

class TestCommConfig : public ITestCommConfig {
 public:
  explicit TestCommConfig(logging::Logger& logger) : ITestCommConfig(logger) {}

  void GetReplicaConfig(uint16_t replica_id, std::string keyFilePrefix, bftEngine::ReplicaConfig* out_config) override;

  bft::communication::PlainUdpConfig GetUDPConfig(bool is_replica,
                                                  uint16_t node_id,
                                                  uint16_t& num_of_clients,
                                                  uint16_t& num_of_replicas,
                                                  const std::string& config_file_name) override;

  bft::communication::PlainTcpConfig GetTCPConfig(bool is_replica,
                                                  uint16_t node_id,
                                                  uint16_t& num_of_clients,
                                                  uint16_t& num_of_replicas,
                                                  const std::string& config_file_name) override;

  bft::communication::TlsTcpConfig GetTlsTCPConfig(bool is_replica,
                                                   uint16_t id,
                                                   uint16_t& num_of_clients,
                                                   uint16_t& num_of_replicas,
                                                   const std::string& config_file_name,
                                                   const std::string& cert_root_path = "certs") override;

 private:
  std::unordered_map<bft::communication::NodeNum, bft::communication::NodeInfo> SetUpConfiguredNodes(
      bool is_replica,
      const std::string& config_file_name,
      uint16_t node_id,
      std::string& ip,
      uint16_t& port,
      uint16_t& num_of_clients,
      uint16_t& num_of_replicas);

  std::unordered_map<bft::communication::NodeNum, bft::communication::NodeInfo> SetUpDefaultNodes(
      uint16_t node_id, std::string& ip, uint16_t& port, uint16_t num_of_clients, uint16_t num_of_replicas);

  std::unordered_map<bft::communication::NodeNum, bft::communication::NodeInfo> SetUpNodes(
      bool is_replica,
      uint16_t node_id,
      std::string& ip,
      uint16_t& port,
      uint16_t& num_of_clients,
      uint16_t& num_of_replicas,
      const std::string& config_file_name);

 private:
  // Network port of the first replica. Other replicas use ports
  // basePort + (2 * index).
  static const uint16_t base_port_ = 3710;
  static const uint32_t buf_length_ = 128 * 1024;  // 128 kB
  static const std::string default_ip_;
  static const std::string default_listen_ip_;
  static const char* ip_port_delimiter_;
};
