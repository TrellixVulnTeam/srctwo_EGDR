/*
 * Copyright (C) 2013 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef InterpolatedTransformOperation_h
#define InterpolatedTransformOperation_h

#include "platform/transforms/TransformOperation.h"
#include "platform/transforms/TransformOperations.h"

namespace blink {

// This class is an implementation detail for deferred interpolations.
class PLATFORM_EXPORT InterpolatedTransformOperation final
    : public TransformOperation {
 public:
  static PassRefPtr<InterpolatedTransformOperation> Create(
      const TransformOperations& from,
      const TransformOperations& to,
      double progress) {
    return AdoptRef(new InterpolatedTransformOperation(from, to, progress));
  }

  virtual bool CanBlendWith(const TransformOperation& other) const {
    return IsSameType(other);
  }

 private:
  OperationType GetType() const override { return kInterpolated; }

  bool operator==(const TransformOperation&) const override;
  void Apply(TransformationMatrix&,
             const FloatSize& border_box_size) const override;

  PassRefPtr<TransformOperation> Blend(const TransformOperation* from,
                                       double progress,
                                       bool blend_to_identity = false) override;
  PassRefPtr<TransformOperation> Zoom(double factor) final {
    return Create(from.Zoom(factor), to.Zoom(factor), progress);
  }

  bool DependsOnBoxSize() const override {
    return from.DependsOnBoxSize() || to.DependsOnBoxSize();
  }

  InterpolatedTransformOperation(const TransformOperations& from,
                                 const TransformOperations& to,
                                 double progress)
      : from(from), to(to), progress(progress) {}

  const TransformOperations from;
  const TransformOperations to;
  double progress;
};

}  // namespace blink

#endif  // InterpolatedTransformOperation_h
