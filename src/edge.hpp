#pragma once

#include "semver.hpp"
#include <memory>
#include <string>

class Node;

class Edge {
private:
  std::unique_ptr<range> spec_;

public:
  std::string name;
  std::string alias;
  std::string spec;

  Node &from;
  Node *to = nullptr;

  bool valid;

  bool peer;
  bool optional;
  bool peerConflict = false;

  // Edge must be construct in place in a Node's edgesOut
  // Edge cannot be copied or moved
  Edge(const std::string &name, const std::string &spec, Node &from, bool peer,
       bool optional)
      : from(from), name(name), spec(spec), peer(peer), optional(optional),
        valid(optional),
        alias(spec.starts_with("npm:") ? spec.substr(4, spec.rfind('@') - 4)
                                       : "") {
    spec_ = std::make_unique<range>(
        alias.empty() ? spec : spec.substr(4 + alias.size() + 1));
    reload();
  }

  void reload();

  bool satisfiedBy(const Node *node);
};
