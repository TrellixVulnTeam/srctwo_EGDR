// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FontVariantNumeric_h
#define FontVariantNumeric_h

#include "platform/wtf/Allocator.h"

namespace blink {

class FontVariantNumeric {
  STACK_ALLOCATED();

 public:
  enum NumericFigure { kNormalFigure = 0, kLiningNums, kOldstyleNums };

  enum NumericSpacing { kNormalSpacing = 0, kProportionalNums, kTabularNums };

  enum NumericFraction {
    kNormalFraction = 0,
    kDiagonalFractions,
    kStackedFractions
  };

  enum Ordinal { kOrdinalOff = 0, kOrdinalOn };

  enum SlashedZero { kSlashedZeroOff = 0, kSlashedZeroOn };

  FontVariantNumeric() : fields_as_unsigned_(0) {}

  static FontVariantNumeric InitializeFromUnsigned(unsigned init_value) {
    return FontVariantNumeric(init_value);
  }

  void SetNumericFigure(NumericFigure figure) {
    fields_.numeric_figure_ = figure;
  };
  void SetNumericSpacing(NumericSpacing spacing) {
    fields_.numeric_spacing_ = spacing;
  };
  void SetNumericFraction(NumericFraction fraction) {
    fields_.numeric_fraction_ = fraction;
  };
  void SetOrdinal(Ordinal ordinal) { fields_.ordinal_ = ordinal; };
  void SetSlashedZero(SlashedZero slashed_zero) {
    fields_.slashed_zero_ = slashed_zero;
  };

  NumericFigure NumericFigureValue() const {
    return static_cast<NumericFigure>(fields_.numeric_figure_);
  }
  NumericSpacing NumericSpacingValue() const {
    return static_cast<NumericSpacing>(fields_.numeric_spacing_);
  }
  NumericFraction NumericFractionValue() const {
    return static_cast<NumericFraction>(fields_.numeric_fraction_);
  }
  Ordinal OrdinalValue() const {
    return static_cast<Ordinal>(fields_.ordinal_);
  };
  SlashedZero SlashedZeroValue() const {
    return static_cast<SlashedZero>(fields_.slashed_zero_);
  }

  bool IsAllNormal() { return !fields_as_unsigned_; }

  bool operator==(const FontVariantNumeric& other) const {
    return fields_as_unsigned_ == other.fields_as_unsigned_;
  }

 private:
  FontVariantNumeric(unsigned init_value) : fields_as_unsigned_(init_value) {}

  struct BitFields {
    unsigned numeric_figure_ : 2;
    unsigned numeric_spacing_ : 2;
    unsigned numeric_fraction_ : 2;
    unsigned ordinal_ : 1;
    unsigned slashed_zero_ : 1;
  };

  union {
    BitFields fields_;
    unsigned fields_as_unsigned_;
  };
  static_assert(sizeof(BitFields) == sizeof(unsigned),
                "Mapped union types must match in size.");

  // Used in setVariant to store the value in m_fields.m_variantNumeric;
  friend class FontDescription;
};
}

#endif  // FontVariantNumeric_h
