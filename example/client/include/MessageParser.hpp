#pragma once

#include <chrono>
#include <map>
#include <set>
#include <vector>
#include <cstdio>
#include <cstring>
#include <sys/param.h>
#include <unistd.h>
#include <yaml-cpp/yaml.h>
#include <TimeUtils.hpp>

#include "Logger.hpp"
#include "kv_replica_msgs.cmf.hpp"
#include "messages/ClientRequestMsg.hpp"
#include "KVBCInterfaces.h"
#include "bftclient/base_types.h"
#include "common_constants.hpp"
#include "block_metadata.hpp"
#include "boost/detail/endian.hpp"
#include "assertUtils.hpp"
#include "storage/db_types.h"

namespace concord::kvbc {
using namespace ::kvbc::messages;

struct KVBlock {
  uint64_t id = 0;
  std::vector<std::pair<std::vector<uint8_t>, std::vector<uint8_t>>> items;
};
typedef std::map<std::pair<std::vector<uint8_t>, uint64_t>, std::vector<uint8_t>> KeyBlockIdToValueMap;

enum MessageType : uint16_t { WRITE = 0, READ = 1, BOTH = 2 };

struct SampleMessage {
  uint16_t messageType = MessageType::WRITE;
  uint16_t senderNodeId = 5;  // default client id for a single client
  uint64_t reqTimeoutMilli = 1000;
  uint16_t numberOfOperation = 1;
  uint16_t numOfWrites = 6;
  uint16_t numOfKeysInReadSet = 7;
};

class MessageParser {
 public:
  MessageParser(const std::string& fileName) : sampleMsgFileName_(fileName) {}
  ~MessageParser(){};
  SampleMessage* getSampleMessage() { return sampleMsg_.get(); }

  // Parsing sample msg file
  void parseSampleMessage();
  void createKVWriteRequestMessage(KVRequest& request);
  void createKVReadRequestMessage(KVRequest& request);
  std::list<KVRequest> getAllRequests() { return requests_; }
  std::list<KVReply> getAllReplies() { return replies_; }

 private:
  logging::Logger getLogger() {
    static logging::Logger logger_(logging::getLogger("osexample::MessageParser"));
    return logger_;
  }

  void addExpectedWriteReply(bool foundConflict);
  void addExpectedReadReply(size_t numberOfKeysToRead, KVReadRequest& read_req);
  bool lookForConflicts(uint64_t readVersion, const std::vector<std::vector<uint8_t>>& readKeysArray);
  void addNewBlock(const std::vector<std::pair<std::vector<uint8_t>, std::vector<uint8_t>>>& writesKVArray);

 private:
  std::string sampleMsgFileName_;
  std::unique_ptr<SampleMessage> sampleMsg_;
  std::list<KVRequest> requests_;
  std::list<KVReply> replies_;
  std::map<uint64_t, KVBlock> internalBlockchain_;
  KeyBlockIdToValueMap allKeysToValueMap_;
  BlockId prevLastBlockId_ = 0;
  BlockId lastBlockId_ = 0;
};
}  // end namespace concord::kvbc