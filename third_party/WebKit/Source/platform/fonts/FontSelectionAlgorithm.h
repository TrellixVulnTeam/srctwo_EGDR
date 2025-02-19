/*
 * Copyright (C) 2017 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef FontSelectionAlgorithm_h
#define FontSelectionAlgorithm_h

#include "platform/PlatformExport.h"
#include "platform/fonts/FontSelectionTypes.h"
#include "platform/wtf/Optional.h"
#include "platform/wtf/Vector.h"

namespace blink {

class PLATFORM_EXPORT FontSelectionAlgorithm {
 public:
  FontSelectionAlgorithm() = delete;

  FontSelectionAlgorithm(FontSelectionRequest request,
                         const FontSelectionCapabilities& capabilities_bounds)
      : request_(request), capabilities_bounds_(capabilities_bounds) {}

  bool IsBetterMatchForRequest(
      const FontSelectionCapabilities& firstCapabilities,
      const FontSelectionCapabilities& secondCapabilities);

  struct DistanceResult {
    FontSelectionValue distance;
    FontSelectionValue value;
  };

  DistanceResult StretchDistance(FontSelectionCapabilities) const;
  DistanceResult StyleDistance(FontSelectionCapabilities) const;
  DistanceResult WeightDistance(FontSelectionCapabilities) const;

 private:
  FontSelectionRequest request_;
  FontSelectionCapabilities capabilities_bounds_;
};

}  // namespace blink

#endif  // FontSelectionAlgorithm_h
