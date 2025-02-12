#include "idealTree.hpp"
#include "dataset.hpp"
#include "exceptions.hpp"
#include "place-dep.hpp"
#include <fmt/core.h>
#include <iostream>
#include <optional>
#include <vector>

namespace {
char g_localeCharOrder[256] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 5,  10, 22, 32, 23,
    21, 9,  11, 12, 18, 26, 2,  1,  7,  19, 33, 34, 35, 36, 37, 38, 39, 40, 41,
    42, 4,  3,  27, 28, 29, 6,  17, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62, 64,
    66, 68, 70, 72, 74, 76, 78, 80, 82, 84, 86, 88, 90, 92, 94, 13, 8,  14, 25,
    0,  24, 43, 45, 47, 49, 51, 53, 55, 57, 59, 61, 63, 65, 67, 69, 71, 73, 75,
    77, 79, 81, 83, 85, 87, 89, 91, 93, 15, 30, 16, 31, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1};
int localeCompare(const std::string &a, const std::string &b) {
  const size_t sizeA = a.size();
  const size_t sizeB = b.size();
  const size_t minLen = std::min(sizeA, sizeB);

  for (size_t i = 0; i < minLen; i++) {
    char charA = a[i], charB = b[i];
    if (charA != charB) {
      return g_localeCharOrder[charA] - g_localeCharOrder[charB];
    }
  }

  return sizeA - sizeB;
}
} // namespace

Node *DepsQueue::pop() {
  if (!sorted) {
    sort(deps.begin(), deps.end(), [](const Node *a, const Node *b) {
      int aDepth = a->depth;
      int bDepth = b->depth;
      if (aDepth == bDepth) {
        return localeCompare(a->path, b->path) < 0;
      } else {
        return aDepth < bDepth;
      }
    });
    sorted = true;
  }
  Node *ret = deps.front();
  deps.pop_front();
  return ret;
}

bool idealTree::isProblemEdge(Edge &edge) {
  if (edge.to && edge.to->loadFailure) {
    return false;
  }

  if (!edge.to) {
    if (!edge.peer || !edge.optional) {
      return true;
    }
    return false;
  }

  if (!edge.valid) {
    return true;
  }

  return false;
}

void idealTree::buildDeps() {
#ifdef PREVENT_LOOP
  int queueLoopCount = 0;
#endif
  while (!depsQueue.empty()) {
    Node *node = depsQueue.pop();

    if (depsSeen.find(node) != depsSeen.end() ||
        node->getRoot() != root.get()) {
      continue;
    }

#ifdef PREVENT_LOOP
    if (QUEUE_LOOP_LIMIT <= queueLoopCount++) {
      throw queueLoopException();
    }
#endif

    depsSeen.insert(node);

    std::vector<std::pair<Edge *, Node *>> tasks;
    auto peerSourceIt = peerSetSource.find(node);
    Node *peerSource =
        peerSourceIt != peerSetSource.end() ? peerSourceIt->second : node;

    for (auto &[_, edge] : node->edgesOut) {
      if (!isProblemEdge(edge)) {
        continue;
      }
      if (edge.peerConflict) {
        continue;
      }

#ifdef PREVENT_LOOP
      loadLoopCount = 0;
#endif

      Node *source = edge.peer ? peerSource : node;
      Node *vr = virtualRoot(source, true);
      assert(vr && "virtualRoot returns nullptr");
      auto vrEdgeIt = vr->edgesOut.find(edge.name);
      Edge *vrEdge =
          vrEdgeIt != vr->edgesOut.end() ? &vrEdgeIt->second : nullptr;
      Node *vrDep = vrEdge ? vrEdge->valid ? vrEdge->to : nullptr : nullptr;

      Node *parent = edge.peer ? vr : nullptr;
      Node *dep =
          vrDep ? edge.satisfiedBy(vrDep) ? vrDep : nodeFromEdge(&edge, parent)
                : nodeFromEdge(&edge, parent);

      if (dep->loadFailure) {
        continue;
      }

      tasks.emplace_back(&edge, dep);
    }

    sort(tasks.begin(), tasks.end(), [](const auto &a, const auto &b) {
      return localeCompare(a.first->name, b.first->name) < 0;
    });

    for (const auto &[edge, dep] : tasks) {
      placeDep pd(nodesStorage, edge, dep);
      depth(&pd);
    }
  }
}

void idealTree::depth(placeDep *pd) {
  visit(pd);
  for (auto &kid : pd->children) {
    depth(kid.get());
  }
}

void idealTree::visit(placeDep *pd) {
  Node *placed = pd->placed;
  Edge *edge = pd->edge;
  canPlaceDep *cpd = pd->canPlace.get();
  if (!placed || !cpd) {
    return;
  }

  if (cpd->canPlaceSelf == canPlaceType::OK) {
    for (Edge *edgeIn : placed->edgesIn) {
      if (edgeIn == edge) {
        continue;
      }
      if (!edgeIn->peerConflict && !edgeIn->valid &&
          depsSeen.find(&edgeIn->from) == depsSeen.end()) {
        depsQueue.push(&edgeIn->from);
      }
    }
  } else if (cpd->canPlaceSelf == canPlaceType::REPLACE) {
    for (Edge *edgeIn : placed->edgesIn) {
      if (edgeIn == edge) {
        continue;
      }
      if (!edgeIn->peerConflict && !edgeIn->valid) {
        depsSeen.erase(&edgeIn->from);
        depsQueue.push(&edgeIn->from);
      }
    }
#ifdef DETECT_QUEUE_LOOP
    bool *replacementFlag = &replacementRecord[fmt::format(
        "{} {} {}", placed->path, placed->name, placed->version)];
    if (*replacementFlag) {
      outputReplacementRecord =
          fmt::format("{} {} {}", placed->path, placed->name, placed->version);
      throw queueLoopReplacementDetectedException();
    } else {
      *replacementFlag = true;
    }
#endif
  }

  assert(cpd->canPlaceSelf != canPlaceType::CONFLICT &&
         "placed with canPlaceSelf=CONFLICT");

  depsQueue.push(placed);
  for (Node *dep : pd->needEvaluation) {
    depsSeen.erase(dep);
    depsQueue.push(dep);
  }
}

Node *idealTree::nodeFromEdge(const Edge *edge, Node *parent_,
                              const Edge *secondEdge) {
#ifdef PREVENT_LOOP
  if (LOAD_LOOP_LIMIT <= loadLoopCount++) {
    throw loadLoopException();
  }
#endif

  Node *parent = parent_ ? parent_ : virtualRoot(&edge->from);

  Node *first = nodeFromSpec(edge->name, edge->spec, parent);
  Node *second = secondEdge && !secondEdge->valid
                     ? nodeFromSpec(secondEdge->name, secondEdge->spec, parent)
                     : nullptr;

  Node *node = second && edge->valid ? second : first;

  assert(node && "nodeFromSpec returns nullptr");

  if (second && node != second) {
    node->setParent(parent);
  }

  peerSetSource[node] = parent->sourceReference;

  if (!node->loadFailure) {
    loadPeerSet(node);
  }

  return node;
}

void idealTree::loadPeerSet(Node *node) {
  std::vector<Edge *> peerEdges;
  for (auto &[_, edge] : node->edgesOut) {
    if (!edge.peer || edge.valid) {
      continue;
    }
    peerEdges.emplace_back(&edge);
  }

  sort(peerEdges.begin(), peerEdges.end(), [](const Edge *a, const Edge *b) {
    return localeCompare(a->name, b->name) < 0;
  });

  for (Edge *edge : peerEdges) {
    if (edge->valid && edge->to) {
      continue;
    }

    if (!node->getParent()) {
      throw npmErrorException();
    }
    auto parentEdgeIt = node->getParent()->edgesOut.find(edge->name);
    Edge *parentEdge = parentEdgeIt == node->getParent()->edgesOut.end()
                           ? nullptr
                           : &parentEdgeIt->second;

    Node *sourceReference = node->getParent()->sourceReference;
    bool conflictOK =
        sourceReference ? sourceReference->getParent() != nullptr : true;

    if (!edge->to) {
      if (!parentEdge) {
        nodeFromEdge(edge, node->getParent());
        continue;
      } else {
        nodeFromEdge(parentEdge, node->getParent(), edge);
        if (edge->valid) {
          continue;
        }

        if (conflictOK) {
          edge->peerConflict = true;
          continue;
        }

        throw failPeerConflictException();
      }
    } else {
      Node *current = edge->to;
      Node *dep = nodeFromEdge(edge);
      if (!dep->loadFailure && current->canReplaceWith(dep)) {
        nodeFromEdge(edge, node->getParent());
        continue;
      }

      if (conflictOK) {
        continue;
      }

      throw failPeerConflictException();
    }
  }
}

Node *idealTree::nodeFromSpec(const std::string &name, const std::string &spec,
                              Node *parent) {
  const DataSheet *data = nullptr;
  if (spec.starts_with("npm:")) {
    // spec type is alias
    int pos = spec.rfind('@');
    data = db->queryVersion(spec.substr(4, pos - 4), spec.substr(pos + 1));
  } else {
    data = db->queryVersion(name, spec);
  }

  if (data && !(data->compatible)) {
    data = nullptr;
  }

  nodesStorage.emplace_back(std::make_unique<Node>(name, data, parent));
  return nodesStorage.rbegin()->get();
}

Node *idealTree::virtualRoot(Node *node, bool reuse) {
  if (reuse) {
    auto vrIt = virtualRoots.find(node);
    if (vrIt != virtualRoots.end()) {
      return vrIt->second;
    }
  }
  nodesStorage.emplace_back(
      std::make_unique<Node>(node->name, nullptr, nullptr, node));
  Node *vr = nodesStorage.rbegin()->get();
  virtualRoots[node] = vr;

  return vr;
}

#ifdef DETECT_QUEUE_LOOP
idealTree::idealTree(DataSet *db, const std::string &name,
                     const std::string &version,
                     std::string &outputReplacementRecord)
    : db(db), outputReplacementRecord(outputReplacementRecord)
#else
idealTree::idealTree(DataSet *db, const std::string &name,
                     const std::string &version)
    : db(db)
#endif
{
  DataSheet init;
  init.compatible = true;
  init.dependencies.try_emplace(name, version);

  assert(nodesStorage.empty() && "nodesStorage is not empty");

  root = std::make_unique<Node>(".", &init, nullptr);
  depsQueue.push(root.get());
  buildDeps();

  bool emptyTree = true;
  for (auto &[name, edge] : root->edgesOut) {
    if (edge.valid) {
      emptyTree = false;
      break;
    }
  }
  if (emptyTree) {
    throw emptyTreeException();
  }
}

void idealTree::printDirs() {
  std::string output;
  printDirs(output);
  std::cout << output << std::flush;
}

void idealTree::printDirs(std::string &output) {
  std::vector<Node *> nodes;
  nodes.emplace_back(root.get());
  for (int i = 0; i < nodes.size(); i++) {
    for (auto &[_, node] : nodes[i]->children) {
      nodes.emplace_back(node);
    }
  }
  for (Node *node : nodes) {
    output += fmt::format("{} {}\n", node->path, node->version);
  }
}

namespace {
struct PrintingItem {
  Node *node;
  Edge *edge;
  std::vector<std::pair<std::unique_ptr<PrintingItem>, bool>> deps;
  PrintingItem(Node *node, Edge *edge) : node(node), edge(edge) {}
};

void printItem(PrintingItem *item, std::string &output) {
  if (item->deps.size()) {
    output += '{';
    for (auto &[kid, deduped] : item->deps) {
      output += fmt::format(
          "\"{}@{}{}\":{}", kid->node->name,
          kid->edge->alias.size()
              ? fmt::format("npm:{}@{}", kid->edge->alias, kid->node->version)
              : kid->node->version,
          kid->edge->peer ? " (peer)" : "", deduped ? "\"deduped\"," : "");
      if (!deduped) {
        printItem(kid.get(), output);
        output += ',';
      }
    }
    *output.rbegin() = '}';
  } else {
    output += "{}";
  }
}
} // namespace

void idealTree::printDeps() {
  std::string output;
  printDeps(output);
  std::cout << output << std::endl;
}

void idealTree::printDeps(std::string &output) {
  PrintingItem rootItem(root.get(), nullptr);
  std::vector<PrintingItem *> items;
  std::unordered_set<Node *> seenNodes;
  items.emplace_back(&rootItem);
  seenNodes.insert(root.get());
  for (int i = 0; i < items.size(); i++) {
    PrintingItem *item = items[i];
    std::vector<Edge *> edges;
    for (auto &[name, edge] : item->node->edgesOut) {
      if (!edge.valid || !edge.to) {
        continue;
      }
      edges.emplace_back(&edge);
    }
    sort(edges.begin(), edges.end(), [](const auto a, const auto b) {
      return localeCompare(a->name, b->name) < 0;
    });
    for (Edge *edge : edges) {
      if (seenNodes.find(edge->to) != seenNodes.end()) {
        item->deps.emplace_back(std::make_unique<PrintingItem>(edge->to, edge),
                                true);
      } else {
        item->deps.emplace_back(std::make_unique<PrintingItem>(edge->to, edge),
                                false);
        items.emplace_back(item->deps.rbegin()->first.get());
        seenNodes.insert(edge->to);
      }
    }
  }
  printItem(*items.begin(), output);
}

#ifdef DETECT_QUEUE_LOOP
void idealTree::printReplacementRecord(std::string &output) {
  if (replacementRecord.size()) {
    output += '[';
    for (const auto &[key, value] : replacementRecord) {
      output += fmt::format("\"{}\",", key);
    }
    *output.rbegin() = ']';
  } else {
    output += "[]";
  }
}
#endif
