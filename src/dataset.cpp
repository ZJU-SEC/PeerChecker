#include "dataset.hpp"
#include "semver.hpp"
#include <curl/curl.h>
#include <fmt/core.h>

using namespace std;
using namespace nlohmann;
using namespace sw::redis;

static size_t WriteCallback(void *contents, size_t size, size_t nmemb,
                            void *userp) {
  ((std::string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

const DataSheet *DataSet::queryVersion(const string &name,
                                       const string &rangeSpec) {
  auto it = cache.find(name + " " + rangeSpec);
  if (it != cache.end()) {
    return &(it->second);
  }

#if defined(USE_OFFICIAL) || defined(USE_LOCAL)
  CURL *easyhandle = curl_easy_init();
  if (!easyhandle) {
    fmt::print(stderr, "{} {}: cannot init curl\n", name, rangeSpec);
    return nullptr;
  }
  string readBuffer;
  curl_easy_setopt(easyhandle, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(easyhandle, CURLOPT_WRITEDATA, &readBuffer);
#ifdef USE_OFFICIAL
  curl_easy_setopt(easyhandle, CURLOPT_URL,
                 ("https://registry.npmjs.org/" + name).c_str());
#elif defined(USE_LOCAL)
  curl_easy_setopt(easyhandle, CURLOPT_URL,
                   ("http://127.0.0.1:8080/" + name).c_str());
#endif

  CURLcode res = curl_easy_perform(easyhandle);

  curl_easy_cleanup(easyhandle);

  if (res) {
    fmt::print(stderr, "{} {}: libcurl error codes: {}\n", name, rangeSpec,
              static_cast<int>(res));
    return nullptr;
  }

  auto j = json::parse(readBuffer);

  auto &versions = j["versions"];
  if (!versions.size()) {
    fmt::print(stderr, "{} {}: has no version\n", name, rangeSpec);
    return nullptr;
  }

  auto &tags = j["dist-tags"];
  string tagVersion; // tagVersion is used to save a target version or the
                     // latest version

  const string *targetKey = nullptr;
  const json *targetValue = nullptr;

  // test if the rangeSpec is a tag
  auto tagIt = tags.find(rangeSpec);
  if (tagIt != tags.end()) {
    if (tagIt->is_string()) {
      tagVersion = *tagIt;
      targetKey = &tagVersion;
      targetValue = &(versions[tagVersion]);
    } else {
      return nullptr;
    }
  } else {
    // the rangeSpec is not a tag, so we resolve it
    range targetRange(rangeSpec);
    if (!targetRange.valid) {
      return nullptr;
    }

    // the version tagged as "latest" is the maximum version to be checked
    auto latestIt = tags.find("latest");
    bool isLatestVersionSatisfied = false;
    if (latestIt != tags.end()) {
      if (latestIt->is_string()) {
        tagVersion = *latestIt;
        isLatestVersionSatisfied =
            targetRange.format() == "" || targetRange.test(tagVersion);
      } else {
        return nullptr;
      }
    }

    if (isLatestVersionSatisfied) {
      targetKey = &tagVersion;
      targetValue = &(versions[tagVersion]);
    } else {
      // TODO: consider engineOK
      bool isCurrentDeprecated = false;
      for (const auto &[k, v] : versions.items()) {
        if (!targetRange.test(k)) {
          continue;
        }

        auto deprIt = v.find("deprecated");
        bool isNewDeprecated = deprIt == v.end()      ? false
                               : deprIt->is_boolean() ? deprIt->get<bool>()
                                                      : true;
        bool update = false;
        if (targetKey) {
          semver currKey(*targetKey);
          semver newKey(k);
          if (isCurrentDeprecated) {
            if (!isNewDeprecated) {
              update = true;
            } else if (newKey.valid && currKey.compare(newKey) < 0) {
              update = true;
            }
          } else {
            if (!isNewDeprecated && newKey.valid &&
                currKey.compare(newKey) < 0) {
              update = true;
            }
          }
        } else {
          update = true;
        }

        if (update) {
          isCurrentDeprecated = isNewDeprecated;
          targetKey = &k;
          targetValue = &v;
        }
      }
    }
  }

  if (!(targetKey && targetValue)) {
    return nullptr;
  }
#elif defined(USE_REDIS)
  vector<string> versions;
  r0.lrange(name, 0, -1, back_inserter(versions));
  if (!versions.size()) {
    // fmt::print(stderr, "{} {}: has no version\n", name, rangeSpec);
    return nullptr;
  }

  json tags;
  string tagVersion; // tagVersion is used to save a target version or the
                     // latest version

  auto dist_tags = r1.get(name);
  if (dist_tags) {
    tags = json::parse(*dist_tags);
  }

  const string *targetKey = nullptr;

  // test if the rangeSpec is a tag
  auto tagIt = tags.find(rangeSpec);
  if (tagIt != tags.end()) {
    if (tagIt->is_string()) {
      tagVersion = *tagIt;
      targetKey = &tagVersion;
    } else {
      return nullptr;
    }
  } else {
    // TODO: consider depricated and engineOK

    // the rangeSpec is not a tag, so we resolve it
    range targetRange(rangeSpec);
    if (!targetRange.valid) {
      return nullptr;
    }

    // the version tagged as "latest" is the maximum version to be checked
    auto latestIt = tags.find("latest");
    bool isLatestVersionSatisfied = false;
    if (latestIt != tags.end()) {
      if (latestIt->is_string()) {
        tagVersion = *latestIt;
        isLatestVersionSatisfied =
            targetRange.format() == "" || targetRange.test(tagVersion);
      } else {
        return nullptr;
      }
    }

    if (isLatestVersionSatisfied) {
      targetKey = &tagVersion;
    } else {
      for (const auto &k : versions) {
        if (!targetRange.test(k)) {
          continue;
        }

        if (targetKey) {
          semver currKey(*targetKey);
          semver newKey(k);
          if (newKey.valid && currKey.compare(newKey) > 0) {
            continue;
          }
        }

        targetKey = &k;
      }
    }
  }

  if (!targetKey) {
    return nullptr;
  }

  const json *targetValue = nullptr;

  json versionMetadata;
  auto target = r2.get(name + " " + *targetKey);
  if (target) {
    versionMetadata = json::parse(*target);
    targetValue = &versionMetadata;
  } else {
    return nullptr;
  }
#endif

  return setDataSheet(name + " " + rangeSpec, targetKey, targetValue);
}

DataSheet *DataSet::setDataSheet(const std::string cacheKey,
                                 const std::string *targetKey,
                                 const nlohmann::json *targetValue) {
  DataSheet *ret = &(cache[cacheKey]);

  if (validateOS(*targetValue) && validateArch(*targetValue) &&
      validateLibc(*targetValue)) {
    ret->compatible = true;
    ret->version = (*targetKey);
    auto depsIt = targetValue->find("dependencies");
    if (depsIt != targetValue->end()) {
      ret->dependencies = *depsIt;
    }
    auto optionalDepsIt = targetValue->find("optionalDependencies");
    if (optionalDepsIt != targetValue->end()) {
      ret->optionalDependencies = *optionalDepsIt;
      for (const auto &[k, v] : ret->optionalDependencies) {
        ret->dependencies.erase(k);
      }
    }
    auto peerDepsIt = targetValue->find("peerDependencies");
    if (peerDepsIt != targetValue->end()) {
      ret->peerDependencies = *peerDepsIt;
    }
    auto peerDepsMetaIt = targetValue->find("peerDependenciesMeta");
    if (peerDepsMetaIt != targetValue->end()) {
      for (auto it = peerDepsMetaIt->begin(); it != peerDepsMetaIt->end();
           it++) {
        if (it.value()["optional"]) {
          ret->peerOptionalDependencies[it.key()] =
              ret->peerDependencies[it.key()];
          ret->peerDependencies.erase(it.key());
        }
      }
    }
  } else {
    ret->compatible = false;
  }

  return ret;
}

bool DataSet::validateOS(const json &package) {
  if (os.empty()) {
    return true;
  }
  auto packageOS = package.find("os");
  if (packageOS == package.end()) {
    return true;
  }
  else if (packageOS->is_array() &&
           std::find(packageOS->begin(), packageOS->end(), os) !=
               packageOS->end()) {
    return true;
  } else {
    return false;
  }
}

bool DataSet::validateArch(const json &package) {
  if (cpu.empty()) {
    return true;
  }
  auto packageCPU = package.find("cpu");
  if (packageCPU == package.end()) {
    return true;
  }
  else if (packageCPU->is_array() &&
           std::find(packageCPU->begin(), packageCPU->end(), cpu) !=
               packageCPU->end()) {
    return true;
  } else {
    return false;
  }
}

bool DataSet::validateLibc(const json &package) {
  if (libc.empty()) {
    return true;
  }
  auto packageLibc = package.find("libc");
  if (packageLibc == package.end()) {
    return true;
  }
  else if (packageLibc->is_array() &&
           std::find(packageLibc->begin(), packageLibc->end(), libc) !=
               packageLibc->end()) {
    return true;
  } else {
    return false;
  }
}
