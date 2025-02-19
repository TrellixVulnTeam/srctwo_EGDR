// Copyright (c) 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef StyleContentAlignmentData_h
#define StyleContentAlignmentData_h

#include "core/style/ComputedStyleConstants.h"
#include "platform/wtf/Allocator.h"

namespace blink {

class StyleContentAlignmentData {
  DISALLOW_NEW();

 public:
  // Style data for Content-Distribution properties: align-content,
  // justify-content.
  // <content-distribution> || [ <overflow-position>? && <content-position> ]
  StyleContentAlignmentData(
      ContentPosition position,
      ContentDistributionType distribution,
      OverflowAlignment overflow = kOverflowAlignmentDefault)
      : position_(position), distribution_(distribution), overflow_(overflow) {}

  void SetPosition(ContentPosition position) { position_ = position; }
  void SetDistribution(ContentDistributionType distribution) {
    distribution_ = distribution;
  }
  void SetOverflow(OverflowAlignment overflow) { overflow_ = overflow; }

  ContentPosition GetPosition() const {
    return static_cast<ContentPosition>(position_);
  }
  ContentDistributionType Distribution() const {
    return static_cast<ContentDistributionType>(distribution_);
  }
  OverflowAlignment Overflow() const {
    return static_cast<OverflowAlignment>(overflow_);
  }

  bool operator==(const StyleContentAlignmentData& o) const {
    return position_ == o.position_ && distribution_ == o.distribution_ &&
           overflow_ == o.overflow_;
  }

  bool operator!=(const StyleContentAlignmentData& o) const {
    return !(*this == o);
  }

 private:
  unsigned position_ : 4;      // ContentPosition
  unsigned distribution_ : 3;  // ContentDistributionType
  unsigned overflow_ : 2;      // OverflowAlignment
};

}  // namespace blink

#endif  // StyleContentAlignmentData_h
