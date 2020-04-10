//
//
//  Copyright (C) 2020 Schrödinger, LLC
//
//   @@ All Rights Reserved @@
//  This file is part of the RDKit.
//  The contents are covered by the terms of the BSD license
//  which is included in the file license.txt, found at the root
//  of the RDKit source tree.
//

#include "Sort.h"
#include "rules/SequenceRule.h"

namespace RDKit {
namespace CIPLabeler {

Sort::Sort(const SequenceRule *comparator) : rules{comparator} {}

Sort::Sort(std::vector<const SequenceRule *> comparators)
    : rules{std::move(comparators)} {}

const std::vector<const SequenceRule *> &Sort::getRules() const {
  return rules;
}

Priority Sort::prioritise(const Node *node, std::vector<Edge *> &edges) const {
  return prioritise(node, edges, true);
}

Priority Sort::prioritise(const Node *node, std::vector<Edge *> &edges,
                          bool deep) const {
  bool unique = true;
  int numPseudoAsym = 0;

  for (auto i = 0u; i < edges.size(); ++i)
    for (auto j = i; j > 0; --j) {
      int cmp = compareLigands(node, edges[j - 1], edges[j], deep);

      if (cmp < -1 || cmp > +1) {
        ++numPseudoAsym;
      }

      if (cmp < 0) {
        swap(edges, j, j - 1);
      } else {
        if (cmp == 0) {
          unique = false;
        }
        break;
      }
    }

  return Priority(unique, numPseudoAsym == 1);
}

int Sort::compareLigands(const Node *node, const Edge *a, const Edge *b,
                         bool deep) const {
  // ensure 'up' edges are moved to the front
  if (!a->isBeg(node) && b->isBeg(node)) {
    return +1;
  } else if (a->isBeg(node) && !b->isBeg(node)) {
    return -1;
  }

  for (const auto &rule : rules) {
    int cmp = rule->getComparision(a, b, deep);

    if (cmp != 0) {
      return cmp;
    }
  }
  return 0;
}

void Sort::swap(std::vector<Edge *> &list, int i, int j) const {
  std::swap(list[i], list[j]);
}

std::vector<std::vector<Edge *>>
Sort::getGroups(const std::vector<Edge *> &sorted) const {
  // would be nice to have this integrated whilst sorting - may provide a
  // small speed increase but as most of our lists are small we take use
  // ugly sort then group approach
  auto groups = std::vector<std::vector<Edge *>>{};

  Edge *prev = nullptr;
  for (auto *edge : sorted) {
    if (prev == nullptr ||
        compareLigands(prev->getBeg(), prev, edge, true) != 0) {
      groups.emplace_back();
    }
    prev = edge;
    groups.back().push_back(edge);
  }

  return groups;
}

} // namespace CIPLabeler
} // namespace RDKit