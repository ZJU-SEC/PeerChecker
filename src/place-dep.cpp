#include "place-dep.hpp"
#include "exceptions.hpp"
#include "node.hpp"
#include <assert.h>
#include <iostream>

Node *placeDep::getStartNode() {
  Node *from = parent ? parent->getStartNode() : &edge->from;
  return from->deepestNestingTarget(edge->name);
}

void placeDep::replaceOldDep(Node *oldDep, Node *placed) {
  Node *target = oldDep->getParent();

  std::unordered_set<Node *> oldDeps;
  for (auto &[name, edge] : oldDep->edgesOut) {
    if (placed->edgesOut.find(name) == placed->edgesOut.end() && edge.to) {
      Node *edgeTo = edge.to;
      auto depSet = edgeTo->gatherDepSet(
          [edgeTo](const Edge *e) { return e->to != edgeTo; });
      oldDeps.merge(depSet);
    }
  }

  std::unordered_set<Node *> prunePeerSets;
  for (Edge *edge : oldDep->edgesIn) {
    if (!edge->peer || edge->peerConflict || edge->from.getParent() != target ||
        edge->satisfiedBy(placed)) {
      continue;
    }

    for (auto &[entryEdge, entrySet] : edge->from.peerEntrySets()) {
      Node *entryNode = entryEdge->to;
      if (entryNode != target && entryEdge->from.getParent()) {
        auto depSet = entryNode->gatherDepSet([entryNode](const Edge *e) {
          return e->to != entryNode && !e->peerConflict;
        });
        prunePeerSets.merge(depSet);
      } else {
        edge->peerConflict = true;
      }
    }
  }

  placed->replace(oldDep);
  pruneForReplacement(placed, oldDeps);

  std::unordered_set<Node *> needEvaluationSet;
  for (Node *dep : prunePeerSets) {
    for (Edge *edge : dep->edgesIn) {
      needEvaluationSet.insert(&edge->from);
    }
    dep->setParent(nullptr);
  }
  for (Node *node : needEvaluationSet) {
    if (node->getParent()) {
      needEvaluation.emplace_back(node);
    }
  }
}

void placeDep::pruneForReplacement(Node *node,
                                   std::unordered_set<Node *> &oldDeps) {
  std::unordered_set<Node *> invalidDeps;
  for (auto &[_, edge] : node->edgesOut) {
    if (edge.to && !edge.valid) {
      invalidDeps.insert(edge.to);
    }
  }
  for (Node *dep : oldDeps) {
    for (Node *dep : dep->gatherDepSet(
             [dep](const Edge *e) { return e->to != dep && e->valid; })) {
      invalidDeps.insert(dep);
    }
  }

  Node::gatherDepSet(invalidDeps, [node](const Edge *e) {
    return &e->from != node && e->to != node && e->valid;
  });

  for (Node *dep : invalidDeps) {
    dep->setParent(nullptr);
  }
}

void placeDep::pruneDedupable(Node *node) {
  if (node->canDedupe()) {
    auto deps = node->gatherDepSet(
        [node](const Edge *e) { return e->to != node && e->valid; });
    for (Node *node : deps) {
      node->setParent(nullptr);
    }
  }
}

placeDep::placeDep(std::vector<std::unique_ptr<Node>> &nodesStorage, Edge *edge,
                   Node *dep, placeDep *parent)
    : edge(edge), parent(parent), top(parent ? parent->top : this) {
#ifdef PREVENT_LOOP
  placeLoopCount = parent ? parent->placeLoopCount + 1 : 0;
  if (PLACE_LOOP_LIMIT <= placeLoopCount) {
    throw placeLoopException();
  }
#endif
  assert(!dep->loadFailure && "place loadFailure dep");

  if (edge->to && edge->valid) { // if edge represents optionalDependency,
                                 // the valid may be true when to is nullptr
    return;
  }

  Node *start = getStartNode();
  for (Node *target = start; target; target = target->getParent()) {
    if (target->getParent()) {
      auto targetEdge = target->edgesOut.find(edge->name);
      if (targetEdge != target->edgesOut.end() && targetEdge->second.peer) {
        continue;
      }
    }

    auto cpd = std::make_shared<canPlaceDep>(
        dep, edge,
        parent ? parent->canPlace ? parent->canPlace.get() : nullptr : nullptr,
        target);

    if (cpd->canPlaceSelf != canPlaceType::CONFLICT) {
      canPlaceSelf = cpd;
    }

    if (cpd->canPlace != canPlaceType::CONFLICT) {
      canPlace = cpd;
    } else {
      break;
    }
  }

  if (!canPlace) {
    if (isMine()) {
      throw failPeerConflictException();
    }

    if (!canPlaceSelf) {
      edge->peerConflict = true;
      return;
    }

    canPlace = canPlaceSelf;
  }

  canPlaceType placementType = canPlace->canPlace == canPlaceType::CONFLICT
                                   ? canPlace->canPlaceSelf
                                   : canPlace->canPlace;
  Node *target = canPlace->target;

  if (placementType == canPlaceType::KEEP) {
    if (edge->peer && !edge->valid) {
      edge->peerConflict = true;
    }

    return;
  }

  // handle unresolvable dependency nesting loops
  // a1 -> b1 -> a2 -> b2 -> a1 -> ...
  for (Node *p = target; p; p = p->getParent()) {
    if (p->name == dep->name && p->version == dep->version && p->getParent()) {
      return;
    }
  }

  nodesStorage.emplace_back(std::make_unique<Node>(dep));
  placed = nodesStorage.rbegin()->get();

  auto oldDepIt = target->children.find(edge->name);
  if (oldDepIt != target->children.end()) {
    Node *oldDep = oldDepIt->second;
    replaceOldDep(oldDep, placed);
  } else {
    placed->setParent(target);
  }

  if (edge->peer && !edge->satisfiedBy(placed)) {
    edge->peerConflict = true;
  }

  if (edge->valid && edge->to && edge->to != placed) {
    pruneDedupable(edge->to);
  }

  std::vector<Node *> descendants;
  descendants.emplace_back(target);
  for (int i = 0; i < descendants.size(); i++) {
    for (auto &[name, node] : descendants[i]->children) {
      descendants.emplace_back(node);
    }
  }
  for (Node *node : descendants) {
    if (node->name != edge->name || !node->getParent()) {
      continue;
    }
    pruneDedupable(node);
    if (node->getRoot() == target->getRoot()) {
      std::vector<Node *> kids;
      for (auto &[_, kid] : node->children) {
        kids.emplace_back(kid);
      }
      for (Node *kid : kids) {
        pruneDedupable(kid);
      }
    }
  }

  Node *vr = dep->getParent();
  for (auto &peerEdgeItem : placed->edgesOut) {
    Edge *peerEdge = &peerEdgeItem.second;
    if (peerEdge->valid || !peerEdge->peer || peerEdge->peerConflict) {
      continue;
    }

    if (!vr) {
      throw npmErrorException();
    }
    const auto peerIt = vr->children.find(peerEdge->name);
    if (peerIt == vr->children.end()) {
      continue;
    }
    if (!peerEdge->satisfiedBy(peerIt->second)) {
      continue;
    }

    children.emplace_back(std::make_unique<placeDep>(nodesStorage, peerEdge,
                                                     peerIt->second, this));
  }
}

bool placeDep::isMine() {
  Edge *edge = top->edge;
  Node *node = &edge->from;
  if (node->getParent() == nullptr) {
    return true;
  }
  if (!edge->peer) {
    return false;
  }

  bool hasPeerEdges = false;
  for (Edge *edge : node->edgesIn) {
    if (edge->peer) {
      hasPeerEdges = true;
      continue;
    }
    if (edge->from.getParent() == nullptr) {
      return true;
    }
  }
  if (hasPeerEdges) {
    for (auto &[edge, _] : node->peerEntrySets()) {
      if (edge->from.getParent() == nullptr) {
        return true;
      }
    }
  }

  return false;
}
