/*
 * Copyright (C) 2006, 2007, 2008 Apple Inc. All rights reserved.
 * Copyright (C) 2007 Alp Toker <alp@atoker.com>
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

#include "modules/canvas2d/CanvasGradient.h"

#include "bindings/core/v8/ExceptionState.h"
#include "core/dom/ExceptionCode.h"
#include "modules/canvas2d/CanvasPattern.h"
#include "modules/canvas2d/CanvasStyle.h"

namespace blink {

CanvasGradient::CanvasGradient(const FloatPoint& p0, const FloatPoint& p1)
    : gradient_(Gradient::CreateLinear(p0, p1)), is_zero_size_(p0 == p1) {}

CanvasGradient::CanvasGradient(const FloatPoint& p0,
                               float r0,
                               const FloatPoint& p1,
                               float r1)
    : gradient_(Gradient::CreateRadial(p0, r0, p1, r1)),
      is_zero_size_(p0 == p1 && r0 == r1) {}

void CanvasGradient::addColorStop(float value,
                                  const String& color_string,
                                  ExceptionState& exception_state) {
  if (!(value >= 0 && value <= 1.0f)) {
    exception_state.ThrowDOMException(
        kIndexSizeError, "The provided value (" + String::Number(value) +
                             ") is outside the range (0.0, 1.0).");
    return;
  }

  Color color = 0;
  if (!ParseColorOrCurrentColor(color, color_string, 0 /*canvas*/)) {
    exception_state.ThrowDOMException(kSyntaxError,
                                      "The value provided ('" + color_string +
                                          "') could not be parsed as a color.");
    return;
  }

  gradient_->AddColorStop(value, color);
}

}  // namespace blink
