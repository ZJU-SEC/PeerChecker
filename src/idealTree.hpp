#pragma once

#include "config.hpp"
#include "node.hpp"
#include <deque>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class DataSet;
class placeDep;

enum checkResult { CONFLICT, OK, KEEP, REPLACE };

class DepsQueue {
private:
  std::deque<Node *> deps;
  bool sorted = true;

public:
  bool empty() { return deps.empty(); }
  void push(Node *node) {
    sorted = false;
    deps.emplace_back(node);
  }
  Node *pop();
};

class idealTree {
private:
#ifdef PREVENT_LOOP
  int loadLoopCount = 0;
#endif

#ifdef DETECT_QUEUE_LOOP
  std::unordered_map<std::string, bool> replacementRecord;
  std::string &outputReplacementRecord;
  void printReplacementRecord(std::string &output);
#endif

  std::unique_ptr<Node> root;

  std::vector<std::unique_ptr<Node>> nodesStorage; // donnot support parallel
  DataSet *db;

  DepsQueue depsQueue;

  std::unordered_set<Node *> depsSeen;

  std::unordered_map<Node *, Node *> peerSetSource;

  std::unordered_map<Node *, Node *> virtualRoots;

  Node *virtualRoot(Node *node, bool reuse = false);

  Node *nodeFromEdge(const Edge *edge, Node *parent_ = nullptr,
                     const Edge *secondEdge = nullptr);
  Node *nodeFromSpec(const std::string &name, const std::string &spec,
                     Node *parent);
  void loadPeerSet(Node *node);

  bool isProblemEdge(Edge &edge);
  void buildDeps();

  void depth(placeDep *pd);
  void visit(placeDep *pd);

public:
#ifdef DETECT_QUEUE_LOOP
  idealTree(DataSet *db, const std::string &name, const std::string &version,
            std::string &outputReplacementRecord);
#else
  idealTree(DataSet *db, const std::string &name, const std::string &version);
#endif

  // directory structure
  void printDirs();
  void printDirs(std::string &output);

  // dependency structure
  void printDeps();
  void printDeps(std::string &output);
};
