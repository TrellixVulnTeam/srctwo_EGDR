/*
 * Copyright (C) 2009 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
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

#ifndef Matrix3DTransformOperation_h
#define Matrix3DTransformOperation_h

#include "platform/transforms/TransformOperation.h"

namespace blink {

class PLATFORM_EXPORT Matrix3DTransformOperation final
    : public TransformOperation {
 public:
  static PassRefPtr<Matrix3DTransformOperation> Create(
      const TransformationMatrix& matrix) {
    return AdoptRef(new Matrix3DTransformOperation(matrix));
  }

  TransformationMatrix Matrix() const { return matrix_; }

  virtual bool CanBlendWith(const TransformOperation& other) const {
    return false;
  }

 private:
  OperationType GetType() const override { return kMatrix3D; }

  bool operator==(const TransformOperation& o) const override {
    if (!IsSameType(o))
      return false;
    const Matrix3DTransformOperation* m =
        static_cast<const Matrix3DTransformOperation*>(&o);
    return matrix_ == m->matrix_;
  }

  void Apply(TransformationMatrix& transform, const FloatSize&) const override {
    transform.Multiply(TransformationMatrix(matrix_));
  }

  PassRefPtr<TransformOperation> Blend(const TransformOperation* from,
                                       double progress,
                                       bool blend_to_identity = false) override;
  PassRefPtr<TransformOperation> Zoom(double factor) final;

  Matrix3DTransformOperation(const TransformationMatrix& mat) { matrix_ = mat; }

  TransformationMatrix matrix_;
};

}  // namespace blink

#endif  // Matrix3DTransformOperation_h
