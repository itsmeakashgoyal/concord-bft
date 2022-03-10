// Concord
//
// Copyright (c) 2018-2022 VMware, Inc. All Rights Reserved.
//
// This product is licensed to you under the Apache 2.0 license (the "License").
// You may not use this product except in compliance with the Apache 2.0
// License.
//
// This product may include a number of subcomponents with separate copyright
// notices and license terms. Your use of these subcomponents is subject to the
// terms and conditions of the subcomponent's license, as noted in the LICENSE
// file.

#pragma once

#include <string>
#include "communication/CommDefs.hpp"
#include "ReplicaConfig.hpp"

// This class is used to configure the communication layer for replica or, client.
class CommunicationConfig {
 public:
  CommunicationConfig() = default;
  ~CommunicationConfig() = default;

  // Create a replica config for the replica with index `replicaId`.
  // inputReplicaKeyfile is used to read the keys for this replica, and
  // default values are loaded for non-cryptographic configuration parameters.
  void GetReplicaConfig(uint16_t replica_id, std::string& keyFilePrefix, bftEngine::ReplicaConfig* out_config);

  // Create a UDP communication configuration for the node (replica or client)
  // with index `id`.
  bft::communication::PlainUdpConfig GetUDPConfig(bool is_replica,
                                                  uint16_t node_id,
                                                  uint16_t& num_of_clients,
                                                  uint16_t& num_of_replicas);

  // Create a TCP communication configuration for the node (replica or client)
  // with index `id`
  bft::communication::PlainTcpConfig GetTCPConfig(bool is_replica,
                                                  uint16_t node_id,
                                                  uint16_t& num_of_clients,
                                                  uint16_t& num_of_replicas);

  // Create a TLSTCP communication configuration for the node (replica or client)
  // with index `id`
  bft::communication::TlsTcpConfig GetTlsTCPConfig(bool is_replica,
                                                   uint16_t id,
                                                   uint16_t& num_of_clients,
                                                   uint16_t& num_of_replicas,
                                                   const std::string& cert_root_path = "certs");

 private:
  std::unordered_map<bft::communication::NodeNum, bft::communication::NodeInfo> SetUpNodes(bool is_replica,
                                                                                           uint16_t node_id,
                                                                                           std::string& ip,
                                                                                           uint16_t& port,
                                                                                           uint16_t& num_of_clients,
                                                                                           uint16_t& num_of_replicas);

 private:
  // Network port of the first replica. Other replicas use ports
  // basePort + (2 * index).
  static const uint16_t base_port_ = 3710;
  static const uint32_t buf_length_ = 128 * 1024;  // 128 kB
  static const std::string default_ip_;
  static const std::string default_listen_ip_;
  static const char* ip_port_delimiter_;
};
