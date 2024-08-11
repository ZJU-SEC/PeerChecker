#pragma once
#include <memory>
#include <unordered_set>
#include <vector>

enum class canPlaceType { UNKNOWN = 0, CONFLICT, OK, KEEP, REPLACE };

class Node;
class Edge;
class canPlaceDep {
private:
  Edge *topEdge;
  std::shared_ptr<std::unordered_set<Node *>> peerPath;

  canPlaceType checkCanPlace(Node *dep, Edge *edge, Edge *targetEdge,
                             Node *current, bool preferDedupe,
                             canPlaceDep *parent);
  canPlaceType checkCanPlaceCurrent(Node *dep, Edge *edge, Node *current,
                                    bool preferDedupe, canPlaceDep *parent);
  canPlaceType checkCanPlaceNoCurrent(Node *dep, Edge *edge);
  canPlaceType canPlacePeers(Node *dep, canPlaceType state);

public:
  Node *target;
  canPlaceType canPlace;
  canPlaceType canPlaceSelf;

  canPlaceDep(Node *dep, Edge *edge, canPlaceDep *parent, Node *target,
              bool preferDedupe = false,
              std::shared_ptr<std::unordered_set<Node *>> peerPath = nullptr);
};
