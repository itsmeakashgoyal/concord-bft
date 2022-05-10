#pragma once

#include <cstdio>
#include <cstring>
#include <sys/param.h>
#include <unistd.h>
#include <yaml-cpp/yaml.h>
#include "Logger.hpp"

namespace concord::kvbc {

struct SampleMsg {
  uint16_t senderNodeId;
  uint64_t clientMsgFlag;
  uint64_t reqSeqNum;
  uint32_t requestLength;
  std::string request;
  uint64_t reqTimeoutMilli;
};

class MsgReader {
 public:
  MsgReader(const std::string& fileName) : sampleMsgFileName_(fileName) { parseSampleMsg(); }
  ~MsgReader(){};

  void parseSampleMsg();
  SampleMsg* getSampleMsg() { return sampleMsg_.get(); }

 private:
  logging::Logger getLogger() {
    static logging::Logger logger_(logging::getLogger("concord::kvbc::MsgReader"));
    return logger_;
  }

 private:
  std::string sampleMsgFileName_;
  std::unique_ptr<SampleMsg> sampleMsg_;
};
}  // end namespace concord::kvbc