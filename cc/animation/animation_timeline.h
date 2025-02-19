// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CC_ANIMATION_ANIMATION_TIMELINE_H_
#define CC_ANIMATION_ANIMATION_TIMELINE_H_

#include <memory>
#include <unordered_map>

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "cc/animation/animation_export.h"

namespace cc {

class AnimationHost;
class AnimationPlayer;

// An AnimationTimeline owns a group of AnimationPlayers.
// This is a cc counterpart for blink::AnimationTimeline (in 1:1 relationship).
// Each AnimationTimeline and its AnimationPlayers have their copies on
// the impl thread. We synchronize main thread and impl thread instances
// using integer IDs.
class CC_ANIMATION_EXPORT AnimationTimeline
    : public base::RefCounted<AnimationTimeline> {
 public:
  static scoped_refptr<AnimationTimeline> Create(int id);
  scoped_refptr<AnimationTimeline> CreateImplInstance() const;

  int id() const { return id_; }

  // Parent AnimationHost.
  AnimationHost* animation_host() { return animation_host_; }
  const AnimationHost* animation_host() const { return animation_host_; }
  void SetAnimationHost(AnimationHost* animation_host);

  void set_is_impl_only(bool is_impl_only) { is_impl_only_ = is_impl_only; }
  bool is_impl_only() const { return is_impl_only_; }

  void AttachPlayer(scoped_refptr<AnimationPlayer> player);
  void DetachPlayer(scoped_refptr<AnimationPlayer> player);

  void ClearPlayers();

  void PushPropertiesTo(AnimationTimeline* timeline_impl);

  AnimationPlayer* GetPlayerById(int player_id) const;

  void SetNeedsPushProperties();
  bool needs_push_properties() const { return needs_push_properties_; }

 private:
  friend class base::RefCounted<AnimationTimeline>;

  explicit AnimationTimeline(int id);
  virtual ~AnimationTimeline();

  void PushAttachedPlayersToImplThread(AnimationTimeline* timeline) const;
  void RemoveDetachedPlayersFromImplThread(AnimationTimeline* timeline) const;
  void PushPropertiesToImplThread(AnimationTimeline* timeline);

  void ErasePlayer(scoped_refptr<AnimationPlayer> player);

  // A list of all players which this timeline owns.
  using IdToPlayerMap = std::unordered_map<int, scoped_refptr<AnimationPlayer>>;
  IdToPlayerMap id_to_player_map_;

  int id_;
  AnimationHost* animation_host_;
  bool needs_push_properties_;

  // Impl-only AnimationTimeline has no main thread instance and lives on
  // it's own.
  bool is_impl_only_;

  DISALLOW_COPY_AND_ASSIGN(AnimationTimeline);
};

}  // namespace cc

#endif  // CC_ANIMATION_ANIMATION_TIMELINE_H_
