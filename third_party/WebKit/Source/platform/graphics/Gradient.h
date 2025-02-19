/*
 * Copyright (C) 2006, 2007, 2008, 2011 Apple Inc. All rights reserved.
 * Copyright (C) 2007 Alp Toker <alp@atoker.com>
 * Copyright (C) 2008 Torch Mobile, Inc.
 * Copyright (C) 2013 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef Gradient_h
#define Gradient_h

#include "platform/PlatformExport.h"
#include "platform/graphics/Color.h"
#include "platform/graphics/GraphicsTypes.h"
#include "platform/graphics/paint/PaintFlags.h"
#include "platform/graphics/paint/PaintShader.h"
#include "platform/wtf/Noncopyable.h"
#include "platform/wtf/PassRefPtr.h"
#include "platform/wtf/RefCounted.h"
#include "platform/wtf/Vector.h"
#include "third_party/skia/include/core/SkRefCnt.h"

class SkMatrix;
class SkShader;

namespace blink {

class FloatPoint;

class PLATFORM_EXPORT Gradient : public RefCounted<Gradient> {
  WTF_MAKE_NONCOPYABLE(Gradient);

 public:
  enum class Type { kLinear, kRadial, kConic };

  enum class ColorInterpolation {
    kPremultiplied,
    kUnpremultiplied,
  };

  static PassRefPtr<Gradient> CreateLinear(
      const FloatPoint& p0,
      const FloatPoint& p1,
      GradientSpreadMethod = kSpreadMethodPad,
      ColorInterpolation = ColorInterpolation::kUnpremultiplied);

  static PassRefPtr<Gradient> CreateRadial(
      const FloatPoint& p0,
      float r0,
      const FloatPoint& p1,
      float r1,
      float aspect_ratio = 1,
      GradientSpreadMethod = kSpreadMethodPad,
      ColorInterpolation = ColorInterpolation::kUnpremultiplied);

  static PassRefPtr<Gradient> CreateConic(
      const FloatPoint& position,
      float rotation,
      float start_angle,
      float end_angle,
      GradientSpreadMethod = kSpreadMethodPad,
      ColorInterpolation = ColorInterpolation::kUnpremultiplied);

  virtual ~Gradient();

  Type GetType() const { return type_; }

  struct ColorStop {
    DISALLOW_NEW_EXCEPT_PLACEMENT_NEW();
    float stop;
    Color color;

    ColorStop(float s, const Color& c) : stop(s), color(c) {}
  };
  void AddColorStop(const ColorStop&);
  void AddColorStop(float value, const Color& color) {
    AddColorStop(ColorStop(value, color));
  }
  void AddColorStops(const Vector<Gradient::ColorStop>&);

  void ApplyToFlags(PaintFlags&, const SkMatrix& local_matrix);

 protected:
  Gradient(Type, GradientSpreadMethod, ColorInterpolation);

  using ColorBuffer = Vector<SkColor, 8>;
  using OffsetBuffer = Vector<SkScalar, 8>;
  virtual sk_sp<PaintShader> CreateShader(const ColorBuffer&,
                                          const OffsetBuffer&,
                                          SkShader::TileMode,
                                          uint32_t flags,
                                          const SkMatrix&,
                                          SkColor) const = 0;

 private:
  sk_sp<PaintShader> CreateShaderInternal(const SkMatrix& local_matrix);

  sk_sp<SkColorFilter> color_filter_;

  void SortStopsIfNecessary();
  void FillSkiaStops(ColorBuffer&, OffsetBuffer&) const;

  const Type type_;
  const GradientSpreadMethod spread_method_;
  const ColorInterpolation color_interpolation_;

  Vector<ColorStop, 2> stops_;
  bool stops_sorted_;

  mutable sk_sp<PaintShader> cached_shader_;
};

}  // namespace blink

#endif
