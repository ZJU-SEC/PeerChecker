#pragma once

#include "config.hpp"
#include "json.hpp"
#include <sw/redis++/redis++.h>

struct DataSheet {
  bool compatible;
  std::string version;
  std::unordered_map<std::string, std::string> dependencies;
  std::unordered_map<std::string, std::string> optionalDependencies;
  std::unordered_map<std::string, std::string> peerDependencies;
  std::unordered_map<std::string, std::string> peerOptionalDependencies;
};

class DataSet {
private:
#ifdef USE_REDIS
  sw::redis::Redis r0 = sw::redis::Redis("tcp://127.0.0.1:6379");
  sw::redis::Redis r1 = sw::redis::Redis("tcp://127.0.0.1:6379/1");
  sw::redis::Redis r2 = sw::redis::Redis("tcp://127.0.0.1:6379/2");
#endif

  std::unordered_map<std::string, DataSheet> cache;

  DataSheet *setDataSheet(const std::string cacheKey,
                          const std::string *targetKey,
                          const nlohmann::json *targetValue);

  bool validateOS(const nlohmann::json &package);
  bool validateArch(const nlohmann::json &package);
  bool validateLibc(const nlohmann::json &package);

public:
  std::string os;
  std::string cpu;
  std::string libc;

  DataSet(const std::string &os, const std::string &cpu,
          const std::string &libc)
      : os(os), cpu(cpu), libc(libc) {}

  void clearCache() { cache.clear(); }

  const DataSheet *queryVersion(const std::string &name,
                                const std::string &rangeSpec);
};
