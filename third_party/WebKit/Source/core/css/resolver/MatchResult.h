/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011 Apple Inc.
 * All rights reserved.
 * Copyright (C) 2013 Google Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef MatchResult_h
#define MatchResult_h

#include "core/css/RuleSet.h"
#include "core/css/SelectorChecker.h"
#include "platform/heap/Handle.h"
#include "platform/wtf/RefPtr.h"
#include "platform/wtf/Vector.h"

namespace blink {

class StylePropertySet;

struct CORE_EXPORT MatchedProperties {
  DISALLOW_NEW_EXCEPT_PLACEMENT_NEW();

 public:
  MatchedProperties();
  ~MatchedProperties();

  DECLARE_TRACE();

  Member<StylePropertySet> properties;

  union {
    struct {
      unsigned link_match_type : 2;
      unsigned whitelist_type : 2;
    } types_;
    // Used to make sure all memory is zero-initialized since we compute the
    // hash over the bytes of this object.
    void* possibly_padded_member;
  };
};

}  // namespace blink

WTF_ALLOW_MOVE_AND_INIT_WITH_MEM_FUNCTIONS(blink::MatchedProperties);

namespace blink {

using MatchedPropertiesVector = HeapVector<MatchedProperties, 64>;

// MatchedPropertiesRange is used to represent a subset of the matched
// properties from a given origin, for instance UA rules, author rules, or a
// shadow tree scope.  This is needed because rules from different origins are
// applied in the opposite order for !important rules, yet in the same order as
// for normal rules within the same origin.

class MatchedPropertiesRange {
 public:
  MatchedPropertiesRange(MatchedPropertiesVector::const_iterator begin,
                         MatchedPropertiesVector::const_iterator end)
      : begin_(begin), end_(end) {}

  MatchedPropertiesVector::const_iterator begin() const { return begin_; }
  MatchedPropertiesVector::const_iterator end() const { return end_; }

  bool IsEmpty() const { return begin() == end(); }

 private:
  MatchedPropertiesVector::const_iterator begin_;
  MatchedPropertiesVector::const_iterator end_;
};

class CORE_EXPORT MatchResult {
  WTF_MAKE_NONCOPYABLE(MatchResult);
  STACK_ALLOCATED();

 public:
  MatchResult() {}

  void AddMatchedProperties(const StylePropertySet* properties,
                            unsigned link_match_type = CSSSelector::kMatchAll,
                            PropertyWhitelistType = kPropertyWhitelistNone);
  bool HasMatchedProperties() const { return matched_properties_.size(); }

  void FinishAddingUARules();
  void FinishAddingAuthorRulesForTreeScope();

  void SetIsCacheable(bool cacheable) { is_cacheable_ = cacheable; }
  bool IsCacheable() const { return is_cacheable_; }

  MatchedPropertiesRange AllRules() const {
    return MatchedPropertiesRange(matched_properties_.begin(),
                                  matched_properties_.end());
  }
  MatchedPropertiesRange UaRules() const {
    return MatchedPropertiesRange(matched_properties_.begin(),
                                  matched_properties_.begin() + ua_range_end_);
  }
  MatchedPropertiesRange AuthorRules() const {
    return MatchedPropertiesRange(matched_properties_.begin() + ua_range_end_,
                                  matched_properties_.end());
  }

  const MatchedPropertiesVector& GetMatchedProperties() const {
    return matched_properties_;
  }

 private:
  friend class ImportantAuthorRanges;
  friend class ImportantAuthorRangeIterator;

  MatchedPropertiesVector matched_properties_;
  Vector<unsigned, 16> author_range_ends_;
  unsigned ua_range_end_ = 0;
  bool is_cacheable_ = true;
};

class ImportantAuthorRangeIterator {
  STACK_ALLOCATED();

 public:
  ImportantAuthorRangeIterator(const MatchResult& result, int end_index)
      : result_(result), end_index_(end_index) {}

  MatchedPropertiesRange operator*() const {
    unsigned range_end = result_.author_range_ends_[end_index_];
    unsigned range_begin = end_index_
                               ? result_.author_range_ends_[end_index_ - 1]
                               : result_.ua_range_end_;
    return MatchedPropertiesRange(
        result_.GetMatchedProperties().begin() + range_begin,
        result_.GetMatchedProperties().begin() + range_end);
  }

  ImportantAuthorRangeIterator& operator++() {
    --end_index_;
    return *this;
  }

  bool operator==(const ImportantAuthorRangeIterator& other) const {
    return end_index_ == other.end_index_ && &result_ == &other.result_;
  }
  bool operator!=(const ImportantAuthorRangeIterator& other) const {
    return !(*this == other);
  }

 private:
  const MatchResult& result_;
  unsigned end_index_;
};

class ImportantAuthorRanges {
  STACK_ALLOCATED();

 public:
  explicit ImportantAuthorRanges(const MatchResult& result) : result_(result) {}

  ImportantAuthorRangeIterator begin() const {
    return ImportantAuthorRangeIterator(result_,
                                        result_.author_range_ends_.size() - 1);
  }
  ImportantAuthorRangeIterator end() const {
    return ImportantAuthorRangeIterator(result_, -1);
  }

 private:
  const MatchResult& result_;
};

inline bool operator==(const MatchedProperties& a, const MatchedProperties& b) {
  return a.properties == b.properties &&
         a.types_.link_match_type == b.types_.link_match_type;
}

inline bool operator!=(const MatchedProperties& a, const MatchedProperties& b) {
  return !(a == b);
}

}  // namespace blink

#endif  // MatchResult_h
