#pragma once

#include "edge.hpp"
#include <functional>
#include <string>
#include <unordered_set>

class DataSheet;

class Node {
private:
  Node *parent = nullptr;
  Node *root;

  void loadDeps(const DataSheet *package);

public:
  const std::string name;
  const std::string version;

  std::unordered_map<std::string, Node *> children;
  std::string path = ".";
  int depth = 0;

  // XXX: javascript's map is in order of pushing sequence
  std::unordered_map<std::string, Edge> edgesOut;
  std::unordered_set<Edge *> edgesIn;

  Node *sourceReference;

  bool loadFailure = false;

  Node(const std::string &name, const DataSheet *package, Node *parent,
       Node *sourceReference = nullptr);

  Node(Node *node);

  Node(const Node &) = delete;
  Node &operator=(const Node &) = delete;

  void setParent(Node *parent);
  Node *getParent() const { return parent; }
  Node *getRoot() const { return root; }

  void replace(Node *node);

  bool isDescendantOf(Node *node);

  void refresh(bool refreshLocation);

  void reloadEdge(const std::string &name, const Node *skip);

  void reloadEdge(const std::string &name);

  Node *resolve(const std::string &name);

  bool canReplaceWith(const Node *node);

  bool canReplaceWith(const Node *node,
                      std::unordered_map<std::string, Node *> &ignorePeers);

  bool canDedupe();

  Node *deepestNestingTarget(const std::string &name);

  std::unordered_set<Node *>
  gatherDepSet(std::function<bool(const Edge *)> &&edgeFilter);

  static void gatherDepSet(std::unordered_set<Node *> &depSet,
                           std::function<bool(const Edge *)> &&edgeFilter);

  std::unordered_map<Edge *, std::unordered_set<Node *>> peerEntrySets();
};
