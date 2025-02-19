/*
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 *           (C) 2000 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2003, 2005, 2006, 2007, 2008 Apple Inc. All rights reserved.
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

#ifndef StyleGeneratedImage_h
#define StyleGeneratedImage_h

#include "core/CoreExport.h"
#include "core/style/StyleImage.h"

namespace blink {

class CSSValue;
class CSSImageGeneratorValue;
class Document;
class ImageResourceObserver;

class CORE_EXPORT StyleGeneratedImage final : public StyleImage {
 public:
  static StyleGeneratedImage* Create(const CSSImageGeneratorValue& value) {
    return new StyleGeneratedImage(value);
  }

  WrappedImagePtr Data() const override { return image_generator_value_.Get(); }

  CSSValue* CssValue() const override;
  CSSValue* ComputedCSSValue() const override;

  LayoutSize ImageSize(const Document&,
                       float multiplier,
                       const LayoutSize& default_object_size) const override;
  bool ImageHasRelativeSize() const override { return !fixed_size_; }
  bool UsesImageContainerSize() const override { return !fixed_size_; }
  void AddClient(ImageResourceObserver*) override;
  void RemoveClient(ImageResourceObserver*) override;
  RefPtr<Image> GetImage(const ImageResourceObserver&,
                         const Document&,
                         const ComputedStyle&,
                         const IntSize&) const override;
  bool KnownToBeOpaque(const Document&, const ComputedStyle&) const override;

  DECLARE_VIRTUAL_TRACE();

 private:
  StyleGeneratedImage(const CSSImageGeneratorValue&);

  // TODO(sashab): Replace this with <const CSSImageGeneratorValue> once
  // Member<> supports const types.
  Member<CSSImageGeneratorValue> image_generator_value_;
  const bool fixed_size_;
};

DEFINE_STYLE_IMAGE_TYPE_CASTS(StyleGeneratedImage, IsGeneratedImage());

}  // namespace blink
#endif
