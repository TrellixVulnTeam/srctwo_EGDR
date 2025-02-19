/*
 * Copyright (C) 2006 Apple Computer, Inc.  All rights reserved.
 * Copyright (C) 2012 Rik Cabanier (cabanier@adobe.com)
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

#include "platform/graphics/GraphicsTypes.h"

#include "platform/wtf/Assertions.h"
#include "platform/wtf/text/WTFString.h"

namespace blink {

static const char* const kCompositeOperatorNames[] = {"clear",
                                                      "copy",
                                                      "source-over",
                                                      "source-in",
                                                      "source-out",
                                                      "source-atop",
                                                      "destination-over",
                                                      "destination-in",
                                                      "destination-out",
                                                      "destination-atop",
                                                      "xor",
                                                      "lighter"};

static const char* const kBlendOperatorNames[] = {
    "normal",     "multiply",   "screen",      "overlay",
    "darken",     "lighten",    "color-dodge", "color-burn",
    "hard-light", "soft-light", "difference",  "exclusion",
    "hue",        "saturation", "color",       "luminosity"};
const int kNumCompositeOperatorNames =
    WTF_ARRAY_LENGTH(kCompositeOperatorNames);
const int kNumBlendOperatorNames = WTF_ARRAY_LENGTH(kBlendOperatorNames);

bool ParseCompositeAndBlendOperator(const String& s,
                                    CompositeOperator& op,
                                    WebBlendMode& blend_op) {
  for (int i = 0; i < kNumCompositeOperatorNames; i++) {
    if (s == kCompositeOperatorNames[i]) {
      op = static_cast<CompositeOperator>(i);
      blend_op = WebBlendMode::kNormal;
      return true;
    }
  }

  for (int i = 0; i < kNumBlendOperatorNames; i++) {
    if (s == kBlendOperatorNames[i]) {
      blend_op = static_cast<WebBlendMode>(i);
      op = kCompositeSourceOver;
      return true;
    }
  }

  return false;
}

String CompositeOperatorName(CompositeOperator op, WebBlendMode blend_op) {
  DCHECK_GE(op, 0);
  DCHECK_LT(op, kNumCompositeOperatorNames);
  DCHECK_GE(static_cast<unsigned>(blend_op), 0u);
  if (blend_op != WebBlendMode::kNormal)
    return kBlendOperatorNames[static_cast<unsigned>(blend_op)];
  return kCompositeOperatorNames[op];
}

bool ParseLineCap(const String& s, LineCap& cap) {
  if (s == "butt") {
    cap = kButtCap;
    return true;
  }
  if (s == "round") {
    cap = kRoundCap;
    return true;
  }
  if (s == "square") {
    cap = kSquareCap;
    return true;
  }
  return false;
}

String LineCapName(LineCap cap) {
  DCHECK_GE(cap, 0);
  DCHECK_LT(cap, 3);
  const char* const kNames[3] = {"butt", "round", "square"};
  return kNames[cap];
}

bool ParseLineJoin(const String& s, LineJoin& join) {
  if (s == "miter") {
    join = kMiterJoin;
    return true;
  }
  if (s == "round") {
    join = kRoundJoin;
    return true;
  }
  if (s == "bevel") {
    join = kBevelJoin;
    return true;
  }
  return false;
}

String LineJoinName(LineJoin join) {
  DCHECK_GE(join, 0);
  DCHECK_LT(join, 3);
  const char* const kNames[3] = {"miter", "round", "bevel"};
  return kNames[join];
}

String TextAlignName(TextAlign align) {
  DCHECK_GE(align, 0);
  DCHECK_LT(align, 5);
  const char* const kNames[5] = {"start", "end", "left", "center", "right"};
  return kNames[align];
}

bool ParseTextAlign(const String& s, TextAlign& align) {
  if (s == "start") {
    align = kStartTextAlign;
    return true;
  }
  if (s == "end") {
    align = kEndTextAlign;
    return true;
  }
  if (s == "left") {
    align = kLeftTextAlign;
    return true;
  }
  if (s == "center") {
    align = kCenterTextAlign;
    return true;
  }
  if (s == "right") {
    align = kRightTextAlign;
    return true;
  }
  return false;
}

String TextBaselineName(TextBaseline baseline) {
  DCHECK_GE(baseline, 0);
  DCHECK_LT(baseline, 6);
  const char* const kNames[6] = {"alphabetic", "top",         "middle",
                                 "bottom",     "ideographic", "hanging"};
  return kNames[baseline];
}

bool ParseTextBaseline(const String& s, TextBaseline& baseline) {
  if (s == "alphabetic") {
    baseline = kAlphabeticTextBaseline;
    return true;
  }
  if (s == "top") {
    baseline = kTopTextBaseline;
    return true;
  }
  if (s == "middle") {
    baseline = kMiddleTextBaseline;
    return true;
  }
  if (s == "bottom") {
    baseline = kBottomTextBaseline;
    return true;
  }
  if (s == "ideographic") {
    baseline = kIdeographicTextBaseline;
    return true;
  }
  if (s == "hanging") {
    baseline = kHangingTextBaseline;
    return true;
  }
  return false;
}

}  // namespace blink
