#include "node.hpp"
#include "dataset.hpp"
#include "edge.hpp"
#include <iostream>

Node::Node(const std::string &name, const DataSheet *package, Node *parent,
           Node *sourceReference)
    : name(name), version(package           ? package->version
                          : sourceReference ? sourceReference->version
                                            : ""),
      sourceReference(sourceReference), root(this) {
  if (package) {
    loadDeps(package);
  } else if (sourceReference) {
    for (auto &[name, edge] : sourceReference->edgesOut) {
      edgesOut.try_emplace(name, edge.name, edge.spec, *this, edge.peer,
                           edge.optional);
    }
    loadFailure = sourceReference->loadFailure;
  } else {
    loadFailure = true;
  }
  if (!loadFailure) {
    setParent(parent);
  }
}

Node::Node(Node *node)
    : name(node->name), version(node->version), sourceReference(nullptr),
      loadFailure(node->loadFailure), root(this) {
  for (auto &[name, edge] : node->edgesOut) {
    edgesOut.try_emplace(name, edge.name, edge.spec, *this, edge.peer,
                         edge.optional);
  }
}

void Node::loadDeps(const DataSheet *package) {
  for (const auto &[name, spec] : package->dependencies) {
    edgesOut.try_emplace(name, name, spec, *this, false, false);
  }
  for (const auto &[name, spec] : package->optionalDependencies) {
    edgesOut.try_emplace(name, name, spec, *this, false, true);
  }
  for (const auto &[name, spec] : package->peerDependencies) {
    edgesOut.try_emplace(name, name, spec, *this, true, false);
  }
  for (const auto &[name, spec] : package->peerOptionalDependencies) {
    edgesOut.try_emplace(name, name, spec, *this, true, true);
  }
}

void Node::setParent(Node *parent) {
  if (this->parent == parent) {
    return;
  }

  if (this == parent) {
    return;
  }

  if (this->parent) {
    this->parent->children.erase(name);
  }

  this->parent = parent;

  if (parent) {
    auto oldChildIt = parent->children.find(name);
    if (oldChildIt != parent->children.end()) {
      oldChildIt->second->setParent(nullptr);
    }
    parent->children[name] = this;
    refresh(true);
    parent->reloadEdge(name, this);
  } else {
    refresh(true);
  }
}

void Node::replace(Node *node) {
  std::vector<Node *> nodeChildren;
  for (auto &[_, kid] : node->children) {
    nodeChildren.emplace_back(kid);
  }
  for (Node *kid : nodeChildren) {
    kid->setParent(this);
  }

  setParent(node->parent);
}

bool Node::isDescendantOf(Node *node) {
  assert(node && "Node::isDescendantOf(nullptr) is illegal");
  for (Node *p = this; p; p = p->getParent()) {
    if (p == node) {
      return true;
    }
  }
  return false;
}

void Node::refresh(bool refreshLocation) {
  if (refreshLocation) {
    std::string newPath = parent ? parent->path + "/node_modules/" + name : ".";
    refreshLocation = newPath != path;
    if (refreshLocation) {
      path = std::move(newPath);
      depth = parent ? parent->depth + 1 : 0;
    }
  }

  std::vector<Edge *> toBeReloaded;
  toBeReloaded.reserve(edgesIn.size());
  for (Edge *edge : edgesIn) {
    toBeReloaded.emplace_back(edge);
  }
  for (Edge *edge : toBeReloaded) {
    edge->reload();
  }

  for (auto &[_, edge] : edgesOut) {
    edge.reload();
  }

  root = parent ? parent->root : this;

  for (auto &[_, kid] : children) {
    kid->refresh(refreshLocation);
  }
}

void Node::reloadEdge(const std::string &name, const Node *skip) {
  auto edgeIt = edgesOut.find(name);
  if (edgeIt != edgesOut.end()) {
    edgeIt->second.reload();
  }
  for (auto &[_, kid] : children) {
    if (kid == skip) {
      continue;
    }
    kid->reloadEdge(name);
  }
}

void Node::reloadEdge(const std::string &name) {
  auto edgeIt = edgesOut.find(name);
  if (edgeIt != edgesOut.end()) {
    edgeIt->second.reload();
  }
  for (auto &[_, kid] : children) {
    kid->reloadEdge(name);
  }
}

Node *Node::resolve(const std::string &name) {
  auto childIt = children.find(name);
  if (childIt != children.end()) {
    return childIt->second;
  }
  if (parent) {
    return parent->resolve(name);
  }
  return nullptr;
}

bool Node::canDedupe() {
  if (!parent || !parent->parent) {
    return false;
  }

  if (edgesIn.empty()) {
    return true;
  }

  Node *other = parent->parent->resolve(name);

  if (!other) {
    return false;
  }

  if (other->version == version) {
    return true;
  }

  if (!canReplaceWith(other)) {
    return false;
  }

  semver thisVer(version), otherVer(other->version);
  if (thisVer.valid && otherVer.valid && otherVer.compare(thisVer) >= 0) {
    return true;
  }

  return false;
}

bool Node::canReplaceWith(const Node *node) {
  assert(node != this && "testing replace node with itself");

  if (name != node->name) {
    return false;
  }

  std::unordered_set<Node *> depSet =
      gatherDepSet([this](const Edge *e) { return e->to != this && e->valid; });

  for (Edge *edge : edgesIn) {
    if (depSet.find(&edge->from) == depSet.end() && !edge->satisfiedBy(node)) {
      return false;
    }
  }

  return true;
}

bool Node::canReplaceWith(
    const Node *node, std::unordered_map<std::string, Node *> &ignorePeers) {
  assert(node != this && "testing replace node with itself");

  if (name != node->name) {
    return false;
  }

  std::unordered_set<Node *> depSet =
      gatherDepSet([this](const Edge *e) { return e->to != this && e->valid; });

  for (Edge *edge : edgesIn) {
    if (parent && edge->from.parent == parent && edge->peer &&
        ignorePeers.find(edge->from.name) != ignorePeers.end()) {
      continue;
    }

    if (depSet.find(&edge->from) == depSet.end() && !edge->satisfiedBy(node)) {
      return false;
    }
  }

  return true;
}

Node *Node::deepestNestingTarget(const std::string &name) {
  for (Node *target = this; target; target = target->getParent()) {
    if (!target->getParent()) {
      return target;
    }
    auto targetEdgeIt = target->edgesOut.find(name);
    if (targetEdgeIt == target->edgesOut.end()) {
      return target;
    }
    if (!targetEdgeIt->second.peer) {
      return target;
    }
  }

  assert(false && "should never reach here");
  return this;
}

std::unordered_set<Node *>
Node::gatherDepSet(std::function<bool(const Edge *)> &&edgeFilter) {
  std::unordered_set<Node *> depSet;
  depSet.insert(this);
  gatherDepSet(depSet, std::move(edgeFilter));
  return depSet;
}

void Node::gatherDepSet(std::unordered_set<Node *> &depSet,
                        std::function<bool(const Edge *)> &&edgeFilter) {
  std::vector<const Node *> tmpQueue;
  for (Node *node : depSet) {
    tmpQueue.emplace_back(node);
  }
  for (int i = 0; i < tmpQueue.size(); i++) {
    for (const auto &[_, edge] : tmpQueue[i]->edgesOut) {
      if (edge.to && edgeFilter(&edge)) {
        if (depSet.find(edge.to) == depSet.end()) {
          tmpQueue.emplace_back(edge.to);
          depSet.insert(edge.to);
        }
      }
    }
  }

  bool changed = true;
  while (changed && depSet.size()) {
    changed = false;
    for (auto depIt = depSet.begin(); depIt != depSet.end();) {
      bool deleted = false;
      for (const Edge *edge : (*depIt)->edgesIn) {
        if (depSet.find(&edge->from) == depSet.end() && edgeFilter(edge)) {
          deleted = true;
          break;
        }
      }
      if (deleted) {
        changed = true;
        depIt = depSet.erase(depIt);
      } else {
        depIt++;
      }
    }
  }
}

std::unordered_map<Edge *, std::unordered_set<Node *>> Node::peerEntrySets() {
  std::unordered_set<Node *> unionSet;
  std::vector<const Node *> tmpQueue;
  unionSet.insert(this);
  tmpQueue.emplace_back(this);
  for (int i = 0; i < tmpQueue.size(); i++) {
    for (const auto &[_, edge] : tmpQueue[i]->edgesOut) {
      if (edge.valid && edge.peer && edge.to) {
        if (unionSet.find(edge.to) == unionSet.end()) {
          tmpQueue.emplace_back(edge.to);
          unionSet.insert(edge.to);
        }
      }
    }
    for (Edge *edge : tmpQueue[i]->edgesIn) {
      if (edge->valid && edge->peer) {
        if (unionSet.find(&edge->from) == unionSet.end()) {
          tmpQueue.emplace_back(&edge->from);
          unionSet.insert(&edge->from);
        }
      }
    }
  }

  std::unordered_map<Edge *, std::unordered_set<Node *>> entrySets;
  for (Node *peer : unionSet) {
    for (Edge *edge : peer->edgesIn) {
      if (!edge->valid) {
        continue;
      }
      if (!edge->peer || edge->from.getParent() == nullptr) {
        std::unordered_set<Node *> sub;
        std::vector<const Node *> tmpQueue;
        sub.insert(peer);
        tmpQueue.emplace_back(peer);
        for (int i = 0; i < tmpQueue.size(); i++) {
          for (const auto &[_, edge] : tmpQueue[i]->edgesOut) {
            if (edge.valid && edge.peer && edge.to) {
              if (sub.find(edge.to) == sub.end()) {
                tmpQueue.emplace_back(edge.to);
                sub.insert(edge.to);
              }
            }
          }
        }

        if (sub.find(this) != sub.end()) {
          entrySets.try_emplace(edge, std::move(sub));
        }
      }
    }
  }

  return entrySets;
}
