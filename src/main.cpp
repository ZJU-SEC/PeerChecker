#include "config.hpp"
#include "dataset.hpp"
#include "exceptions.hpp"
#include "idealTree.hpp"
#include <fmt/core.h>
#include <iostream>

void parse(std::string &name, std::string &version, std::string &depTree, std::string &dirTree, std::string &log, DataSet& db) {
#ifdef DETECT_QUEUE_LOOP
  std::string outputReplacementRecord;
#endif

    try {
#ifdef DETECT_QUEUE_LOOP
      idealTree tree(&db, name, version, outputReplacementRecord);
#else
      idealTree tree(&db, name, version);
#endif
      tree.printDeps(depTree);
      tree.printDirs(dirTree);
      log = fmt::format("{} {}: Ok", name, version);
    } catch (const failPeerConflictException &) {
      log = fmt::format("{} {}: Conflict", name, version);
    } catch (const emptyTreeException &) {
      log = fmt::format("{} {}: Empty", name, version);
    } catch (const queueLoopException &) {
      log = fmt::format("{} {}: QueueLoop", name, version);
    }
#ifdef DETECT_QUEUE_LOOP
    catch (const queueLoopReplacementDetectedException &) {
      log = fmt::format("{} {}: QueueLoopReplacementDetected: {}", name, version, outputReplacementRecord);
    }
#endif
    catch (const loadLoopException &) {
      log = fmt::format("{} {}: LoadLoop", name, version);
    } catch (const placeLoopException &) {
      log = fmt::format("{} {}: PlaceLoop", name, version);
    } catch (const npmErrorException &) {
      log = fmt::format("{} {}: NpmError", name, version);
    } catch (...) {
      log = fmt::format("{} {}: UnknownError", name, version);
    }
}

int main() {
  DataSet db(PARSE_CONFIG_OS, PARSE_CONFIG_CPU, PARSE_CONFIG_LIBC);

  std::string name = "555-ntm-sdk", version = "1.1.3"; // in
  std::string depTree, dirTree, log; // out
  parse(name, version, depTree, dirTree, log, db);

  db.clearCache();

  fmt::print("{} {}\n", name, version);
  fmt::print("{}\n", depTree);
  fmt::print("{}\n", dirTree);
  fmt::print("{}\n", log);

  return 0;
}
