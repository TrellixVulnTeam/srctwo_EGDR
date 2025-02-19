// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CC_TILES_TILE_PRIORITY_H_
#define CC_TILES_TILE_PRIORITY_H_

#include <stddef.h>

#include <algorithm>
#include <limits>
#include <memory>
#include <string>

#include "base/trace_event/trace_event_argument.h"
#include "cc/cc_export.h"

namespace base {
class Value;
}

namespace cc {

enum WhichTree {
  // Note: these must be 0 and 1 because we index with them in various places,
  // e.g. in Tile::priority_.
  ACTIVE_TREE = 0,
  PENDING_TREE = 1,
  LAST_TREE = 1
  // Be sure to update WhichTreeAsValue when adding new fields.
};
std::unique_ptr<base::Value> WhichTreeAsValue(WhichTree tree);

enum TileResolution {
  LOW_RESOLUTION = 0 ,
  HIGH_RESOLUTION = 1,
  NON_IDEAL_RESOLUTION = 2,
};
std::string TileResolutionToString(TileResolution resolution);

struct CC_EXPORT TilePriority {
  enum PriorityBin { NOW, SOON, EVENTUALLY };

  TilePriority()
      : resolution(NON_IDEAL_RESOLUTION),
        priority_bin(EVENTUALLY),
        distance_to_visible(std::numeric_limits<float>::infinity()) {}

  TilePriority(TileResolution resolution,
               PriorityBin bin,
               float distance_to_visible)
      : resolution(resolution),
        priority_bin(bin),
        distance_to_visible(distance_to_visible) {}

  void AsValueInto(base::trace_event::TracedValue* dict) const;

  bool IsHigherPriorityThan(const TilePriority& other) const {
    return priority_bin < other.priority_bin ||
           (priority_bin == other.priority_bin &&
            distance_to_visible < other.distance_to_visible);
  }

  TileResolution resolution;
  PriorityBin priority_bin;
  float distance_to_visible;
};

std::string TilePriorityBinToString(TilePriority::PriorityBin bin);

enum TileMemoryLimitPolicy {
  // Nothing. This mode is used when visible is set to false.
  ALLOW_NOTHING = 0,

  // You might be made visible, but you're not being interacted with.
  ALLOW_ABSOLUTE_MINIMUM = 1,  // Tall.

  // You're being interacted with, but we're low on memory.
  ALLOW_PREPAINT_ONLY = 2,  // Grande.

  // You're the only thing in town. Go crazy.
  ALLOW_ANYTHING = 3  // Venti.
};
std::string TileMemoryLimitPolicyToString(TileMemoryLimitPolicy policy);

enum TreePriority {
  SAME_PRIORITY_FOR_BOTH_TREES,
  SMOOTHNESS_TAKES_PRIORITY,
  NEW_CONTENT_TAKES_PRIORITY,
  LAST_TREE_PRIORITY = NEW_CONTENT_TAKES_PRIORITY
  // Be sure to update TreePriorityAsValue when adding new fields.
};
std::string TreePriorityToString(TreePriority prio);

class GlobalStateThatImpactsTilePriority {
 public:
  GlobalStateThatImpactsTilePriority()
      : memory_limit_policy(ALLOW_NOTHING),
        soft_memory_limit_in_bytes(0),
        hard_memory_limit_in_bytes(0),
        num_resources_limit(0),
        tree_priority(SAME_PRIORITY_FOR_BOTH_TREES) {}

  TileMemoryLimitPolicy memory_limit_policy;

  size_t soft_memory_limit_in_bytes;
  size_t hard_memory_limit_in_bytes;
  size_t num_resources_limit;

  TreePriority tree_priority;

  bool operator==(const GlobalStateThatImpactsTilePriority& other) const {
    return memory_limit_policy == other.memory_limit_policy &&
           soft_memory_limit_in_bytes == other.soft_memory_limit_in_bytes &&
           hard_memory_limit_in_bytes == other.hard_memory_limit_in_bytes &&
           num_resources_limit == other.num_resources_limit &&
           tree_priority == other.tree_priority;
  }
  bool operator!=(const GlobalStateThatImpactsTilePriority& other) const {
    return !(*this == other);
  }

  void AsValueInto(base::trace_event::TracedValue* dict) const;
};

}  // namespace cc

#endif  // CC_TILES_TILE_PRIORITY_H_
