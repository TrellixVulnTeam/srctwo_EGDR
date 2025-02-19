// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PropertyHandle_h
#define PropertyHandle_h

#include "core/CSSPropertyNames.h"
#include "core/CoreExport.h"
#include "core/dom/QualifiedName.h"
#include "platform/wtf/Allocator.h"

namespace blink {

// Represents the property of a PropertySpecificKeyframe.
class CORE_EXPORT PropertyHandle {
  DISALLOW_NEW_EXCEPT_PLACEMENT_NEW();

 public:
  explicit PropertyHandle(CSSPropertyID property,
                          bool is_presentation_attribute = false)
      : handle_type_(is_presentation_attribute ? kHandlePresentationAttribute
                                               : kHandleCSSProperty),
        css_property_(property) {
    DCHECK_NE(property, CSSPropertyInvalid);
    DCHECK_NE(property, CSSPropertyVariable);
  }

  explicit PropertyHandle(const AtomicString& property_name)
      : handle_type_(kHandleCSSCustomProperty),
        svg_attribute_(nullptr),
        property_name_(property_name) {}

  explicit PropertyHandle(const QualifiedName& attribute_name)
      : handle_type_(kHandleSVGAttribute), svg_attribute_(&attribute_name) {}

  bool operator==(const PropertyHandle&) const;
  bool operator!=(const PropertyHandle& other) const {
    return !(*this == other);
  }

  unsigned GetHash() const;

  bool IsCSSProperty() const {
    return handle_type_ == kHandleCSSProperty || IsCSSCustomProperty();
  }
  CSSPropertyID CssProperty() const {
    DCHECK(IsCSSProperty());
    return handle_type_ == kHandleCSSProperty ? css_property_
                                              : CSSPropertyVariable;
  }

  bool IsCSSCustomProperty() const {
    return handle_type_ == kHandleCSSCustomProperty;
  }
  const AtomicString& CustomPropertyName() const {
    DCHECK(IsCSSCustomProperty());
    return property_name_;
  }

  bool IsPresentationAttribute() const {
    return handle_type_ == kHandlePresentationAttribute;
  }
  CSSPropertyID PresentationAttribute() const {
    DCHECK(IsPresentationAttribute());
    return css_property_;
  }

  bool IsSVGAttribute() const { return handle_type_ == kHandleSVGAttribute; }
  const QualifiedName& SvgAttribute() const {
    DCHECK(IsSVGAttribute());
    return *svg_attribute_;
  }

 private:
  enum HandleType {
    kHandleEmptyValueForHashTraits,
    kHandleDeletedValueForHashTraits,
    kHandleCSSProperty,
    kHandleCSSCustomProperty,
    kHandlePresentationAttribute,
    kHandleSVGAttribute,
  };

  explicit PropertyHandle(HandleType handle_type)
      : handle_type_(handle_type), svg_attribute_(nullptr) {}

  static PropertyHandle EmptyValueForHashTraits() {
    return PropertyHandle(kHandleEmptyValueForHashTraits);
  }

  static PropertyHandle DeletedValueForHashTraits() {
    return PropertyHandle(kHandleDeletedValueForHashTraits);
  }

  bool IsDeletedValueForHashTraits() {
    return handle_type_ == kHandleDeletedValueForHashTraits;
  }

  HandleType handle_type_;
  union {
    CSSPropertyID css_property_;
    const QualifiedName* svg_attribute_;
  };
  AtomicString property_name_;

  friend struct ::WTF::HashTraits<blink::PropertyHandle>;
};

}  // namespace blink

namespace WTF {

template <>
struct DefaultHash<blink::PropertyHandle> {
  struct Hash {
    STATIC_ONLY(Hash);
    static unsigned GetHash(const blink::PropertyHandle& handle) {
      return handle.GetHash();
    }

    static bool Equal(const blink::PropertyHandle& a,
                      const blink::PropertyHandle& b) {
      return a == b;
    }

    static const bool safe_to_compare_to_empty_or_deleted = true;
  };
};

template <>
struct HashTraits<blink::PropertyHandle>
    : SimpleClassHashTraits<blink::PropertyHandle> {
  static const bool kNeedsDestruction = true;
  static void ConstructDeletedValue(blink::PropertyHandle& slot, bool) {
    new (NotNull, &slot) blink::PropertyHandle(
        blink::PropertyHandle::DeletedValueForHashTraits());
  }
  static bool IsDeletedValue(blink::PropertyHandle value) {
    return value.IsDeletedValueForHashTraits();
  }

  static blink::PropertyHandle EmptyValue() {
    return blink::PropertyHandle::EmptyValueForHashTraits();
  }
};

}  // namespace WTF

#endif  // PropertyHandle_h
