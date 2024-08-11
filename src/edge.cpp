#include "edge.hpp"
#include "node.hpp"

void Edge::reload() {
  Node *newTo = from.resolve(name);
  if (newTo != to) {
    if (to) {
      to->edgesIn.erase(this);
    }
    to = newTo;
    if (to) {
      to->edgesIn.insert(this);
    }
    valid = to ? satisfiedBy(to) : optional;
  }
}

bool Edge::satisfiedBy(const Node *node) {
  if (name != node->name) {
    return false;
  }

  if (spec_->valid) {
    return spec_->test(node->version);
  } else {
    // spec is tag, url or anything else, ignore
    return !node->loadFailure;
  }
}
