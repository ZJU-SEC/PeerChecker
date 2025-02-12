#pragma once

#include "can-place-dep.hpp"
#include "config.hpp"
#include <functional>
#include <memory>
#include <vector>

class Node;
class Edge;
class placeDep {
private:
#ifdef PREVENT_LOOP
  int placeLoopCount;
#endif

  placeDep *top;

  bool isMine();

  void replaceOldDep(Node *oldDep, Node *placed);

  void pruneForReplacement(Node *node, std::unordered_set<Node *> &oldDeps);

  void pruneDedupable(Node *node);

  Node *getStartNode();

public:
  Edge *edge;
  Node *placed = nullptr;
  std::shared_ptr<canPlaceDep> canPlaceSelf;
  std::shared_ptr<canPlaceDep> canPlace;

  placeDep *parent;
  std::vector<std::unique_ptr<placeDep>> children;

  std::vector<Node *> needEvaluation;

  placeDep(std::vector<std::unique_ptr<Node>> &nodesStorage, Edge *edge,
           Node *dep, placeDep *parent = nullptr);
};
