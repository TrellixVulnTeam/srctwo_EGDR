// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NonInterpolableValue_h
#define NonInterpolableValue_h

#include "platform/wtf/RefCounted.h"

namespace blink {

// Represents components of a PropertySpecificKeyframe's value that either do
// not change or 50% flip when interpolating with an adjacent value.
class NonInterpolableValue : public RefCounted<NonInterpolableValue> {
 public:
  virtual ~NonInterpolableValue() {}

  typedef const void* Type;
  virtual Type GetType() const = 0;
};

// These macros provide safe downcasts of NonInterpolableValue subclasses with
// debug assertions.
// See CSSDefaultInterpolationType.cpp for example usage.
#define DECLARE_NON_INTERPOLABLE_VALUE_TYPE() \
  static Type static_type_;                   \
  Type GetType() const final { return static_type_; }

#define DEFINE_NON_INTERPOLABLE_VALUE_TYPE(T) \
  NonInterpolableValue::Type T::static_type_ = &T::static_type_;

#define DEFINE_NON_INTERPOLABLE_VALUE_TYPE_CASTS(T)               \
  inline bool Is##T(const NonInterpolableValue* value) {          \
    return !value || value->GetType() == T::static_type_;         \
  }                                                               \
  DEFINE_TYPE_CASTS(T, NonInterpolableValue, value, Is##T(value), \
                    Is##T(&value));

}  // namespace blink

#endif  // NonInterpolableValue_h
