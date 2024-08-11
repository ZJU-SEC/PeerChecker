#include "can-place-dep.hpp"
#include "assert.h"
#include "exceptions.hpp"
#include "node.hpp"

canPlaceDep::canPlaceDep(Node *dep, Edge *edge, canPlaceDep *parent,
                         Node *target, bool preferDedupe,
                         std::shared_ptr<std::unordered_set<Node *>> peerPath)
    : target(target), peerPath(peerPath),
      topEdge(parent ? parent->topEdge : edge),
      canPlaceSelf(canPlaceType::UNKNOWN) {
  assert(target && "the target of canPlaceDep is nullptr");

  auto currentIt = target->children.find(edge->name);
  Node *current =
      currentIt != target->children.end() ? currentIt->second : nullptr;

  assert(dep != current && "dep and current in canPlaceDep are the same");

  auto targetEdgeIt = target->edgesOut.find(edge->name);
  Edge *targetEdge =
      targetEdgeIt != target->edgesOut.end() ? &targetEdgeIt->second : nullptr;

  preferDedupe = preferDedupe || edge->peer;

  canPlace =
      checkCanPlace(dep, edge, targetEdge, current, preferDedupe, parent);
  if (canPlaceSelf == canPlaceType::UNKNOWN) {
    canPlaceSelf = canPlace;
  }
}

canPlaceType canPlaceDep::checkCanPlace(Node *dep, Edge *edge, Edge *targetEdge,
                                        Node *current, bool preferDedupe,
                                        canPlaceDep *parent) {
  if (dep->loadFailure) {
    return current ? canPlaceType::REPLACE : canPlaceType::OK;
  }

  if (targetEdge && targetEdge->peer && target->getParent()) {
    return canPlaceType::CONFLICT;
  }

  if (!current && targetEdge && !targetEdge->satisfiedBy(dep) &&
      targetEdge != edge) {
    return canPlaceType::CONFLICT;
  }

  return current
             ? checkCanPlaceCurrent(dep, edge, current, preferDedupe, parent)
             : checkCanPlaceNoCurrent(dep, edge);
}

canPlaceType canPlaceDep::checkCanPlaceCurrent(Node *dep, Edge *edge,
                                               Node *current, bool preferDedupe,
                                               canPlaceDep *parent) {
  if (dep->name == current->name && dep->version == current->version) {
    return canPlaceType::KEEP;
  }

  canPlaceType cppReplace = canPlaceType::UNKNOWN;

  semver curVer(current->version), newVer(dep->version);
  bool tryReplace = curVer.valid && newVer.valid && newVer.compare(curVer) >= 0;
  if (tryReplace && current->canReplaceWith(dep)) {
    cppReplace = canPlacePeers(dep, canPlaceType::REPLACE);
    if (cppReplace != canPlaceType::CONFLICT) {
      return cppReplace;
    }
  }

  if (preferDedupe && edge->satisfiedBy(current)) {
    return canPlaceType::KEEP;
  }

  if (preferDedupe && !tryReplace && current->canReplaceWith(dep)) {
    cppReplace = canPlacePeers(dep, canPlaceType::REPLACE);
    if (cppReplace != canPlaceType::CONFLICT) {
      return cppReplace;
    }
  }

  Node &topEdgeFrom = topEdge ? topEdge->from : edge->from;
  Node *myDeepest = topEdgeFrom.deepestNestingTarget(edge->name);
  if (target != myDeepest) {
    return canPlaceType::CONFLICT;
  }

  if (!edge->peer && target == &edge->from) {
    return cppReplace == canPlaceType::UNKNOWN
               ? canPlacePeers(dep, canPlaceType::REPLACE)
               : cppReplace;
  }

  if (!parent && !edge->peer) {
    return canPlaceType::CONFLICT;
  }

  bool canReplace = true;
  for (auto &[entryEdge, currentPeers] : current->peerEntrySets()) {
    Edge *peerEntryEdge = topEdge;
    if (entryEdge == edge || entryEdge == peerEntryEdge) {
      continue;
    }

    Node *entryNode = entryEdge->to;
    if (!dep->getParent()) {
      throw npmErrorException();
    }
    auto entryRepIt = dep->getParent()->children.find(entryNode->name);
    Node *entryRep = entryRepIt != dep->getParent()->children.end()
                         ? entryRepIt->second
                         : nullptr;
    if (entryRep) {
      if (entryNode->canReplaceWith(entryRep, dep->getParent()->children)) {
        continue;
      }
    }
    bool canClobber = !entryRep;
    if (canClobber) {
      std::unordered_set<Node *> peerReplacementWalk;
      peerReplacementWalk.insert(entryNode);
      std::vector<Node *> tmpQueue;
      tmpQueue.emplace_back(entryNode);
      for (int i = 0; i < tmpQueue.size(); i++) {
        for (auto &[_, edge] : tmpQueue[i]->edgesOut) {
          if (!edge.peer || !edge.valid) {
            continue;
          }
          auto repIt = dep->getParent()->children.find(edge.name);
          if (repIt == dep->getParent()->children.end()) {
            if (edge.to) {
              if (peerReplacementWalk.find(edge.to) ==
                  peerReplacementWalk.end()) {
                peerReplacementWalk.insert(edge.to);
                tmpQueue.emplace_back(edge.to);
              }
            }
            continue;
          }
          if (!edge.satisfiedBy(repIt->second)) {
            canClobber = false;
            goto OUTER;
          }
        }
      }
    OUTER:;
    }
    if (canClobber) {
      continue;
    }

    bool canNestCurrent = true;
    for (Node *currentPeer : currentPeers) {
      Node *curDeep = entryEdge->from.deepestNestingTarget(currentPeer->name);
      if (curDeep == target || target->isDescendantOf(curDeep)) {
        canNestCurrent = false;
        canReplace = false;
        break;
      }
    }
  }

  if (canReplace) {
    return cppReplace == canPlaceType::UNKNOWN
               ? canPlacePeers(dep, canPlaceType::REPLACE)
               : cppReplace;
  }

  return canPlaceType::CONFLICT;
}

canPlaceType canPlaceDep::checkCanPlaceNoCurrent(Node *dep, Edge *edge) {
  Edge *peerEntryEdge = topEdge;
  Node *current =
      target != &peerEntryEdge->from ? target->resolve(edge->name) : nullptr;
  if (current) {
    for (Edge *edge : current->edgesIn) {
      if (edge->from.isDescendantOf(target) && edge->valid) {
        if (!edge->satisfiedBy(dep)) {
          return canPlaceType::CONFLICT;
        }
      }
    }
  }

  return canPlacePeers(dep, canPlaceType::OK);
}

canPlaceType canPlaceDep::canPlacePeers(Node *dep, canPlaceType state) {
  canPlaceSelf = state;

  if (!peerPath) {
    peerPath = std::make_shared<std::unordered_set<Node *>>();
  }
  peerPath->insert(dep);
  bool sawConflict = false;
  for (auto &[_, peerEdge] : dep->edgesOut) {
    if (!peerEdge.peer || !peerEdge.to ||
        peerPath->find(peerEdge.to) != peerPath->end()) {
      continue;
    }
    Node *peer = peerEdge.to;
    canPlaceDep cpp(peer, &peerEdge, this,
                    target->deepestNestingTarget(peer->name), true, peerPath);

    if (cpp.canPlace == canPlaceType::CONFLICT) {
      sawConflict = true;
      break;
    }
  }
  peerPath->erase(dep);

  return sawConflict ? canPlaceType::CONFLICT : state;
}