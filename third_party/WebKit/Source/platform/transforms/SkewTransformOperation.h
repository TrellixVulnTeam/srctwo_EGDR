/*
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 *           (C) 2000 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2003, 2005, 2006, 2007, 2008 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Graham Dennis (graham.dennis@gmail.com)
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

#ifndef SkewTransformOperation_h
#define SkewTransformOperation_h

#include "platform/transforms/TransformOperation.h"
#include "platform/wtf/PassRefPtr.h"

namespace blink {

class PLATFORM_EXPORT SkewTransformOperation final : public TransformOperation {
 public:
  static PassRefPtr<SkewTransformOperation> Create(double angle_x,
                                                   double angle_y,
                                                   OperationType type) {
    return AdoptRef(new SkewTransformOperation(angle_x, angle_y, type));
  }

  double AngleX() const { return angle_x_; }
  double AngleY() const { return angle_y_; }

  virtual bool CanBlendWith(const TransformOperation& other) const;

 private:
  OperationType GetType() const override { return type_; }

  bool operator==(const TransformOperation& o) const override {
    if (!IsSameType(o))
      return false;
    const SkewTransformOperation* s =
        static_cast<const SkewTransformOperation*>(&o);
    return angle_x_ == s->angle_x_ && angle_y_ == s->angle_y_;
  }

  void Apply(TransformationMatrix& transform, const FloatSize&) const override {
    transform.Skew(angle_x_, angle_y_);
  }

  PassRefPtr<TransformOperation> Blend(const TransformOperation* from,
                                       double progress,
                                       bool blend_to_identity = false) override;
  PassRefPtr<TransformOperation> Zoom(double factor) final { return this; }

  SkewTransformOperation(double angle_x, double angle_y, OperationType type)
      : angle_x_(angle_x), angle_y_(angle_y), type_(type) {}

  double angle_x_;
  double angle_y_;
  OperationType type_;
};

}  // namespace blink

#endif  // SkewTransformOperation_h
