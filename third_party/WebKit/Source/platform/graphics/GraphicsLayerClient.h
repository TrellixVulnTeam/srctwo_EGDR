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

#ifndef GraphicsLayerClient_h
#define GraphicsLayerClient_h

#include "platform/PlatformExport.h"
#include "platform/geometry/LayoutSize.h"
#include "platform/wtf/text/WTFString.h"

namespace blink {

class GraphicsContext;
class GraphicsLayer;
class IntRect;

enum GraphicsLayerPaintingPhaseFlags {
  kGraphicsLayerPaintBackground = (1 << 0),
  kGraphicsLayerPaintForeground = (1 << 1),
  kGraphicsLayerPaintMask = (1 << 2),
  kGraphicsLayerPaintOverflowContents = (1 << 3),
  kGraphicsLayerPaintCompositedScroll = (1 << 4),
  kGraphicsLayerPaintChildClippingMask = (1 << 5),
  kGraphicsLayerPaintAncestorClippingMask = (1 << 6),
  kGraphicsLayerPaintDecoration = (1 << 7),
  kGraphicsLayerPaintAllWithOverflowClip =
      (kGraphicsLayerPaintBackground | kGraphicsLayerPaintForeground |
       kGraphicsLayerPaintMask |
       kGraphicsLayerPaintDecoration)
};
typedef unsigned GraphicsLayerPaintingPhase;

enum {
  kLayerTreeNormal = 0,
  // Dump extra debugging info like layer addresses.
  kLayerTreeIncludesDebugInfo = 1 << 0,
  kLayerTreeIncludesPaintInvalidations = 1 << 1,
  kLayerTreeIncludesPaintingPhases = 1 << 2,
  kLayerTreeIncludesRootLayer = 1 << 3,
  kLayerTreeIncludesClipAndScrollParents = 1 << 4,
  kLayerTreeIncludesCompositingReasons = 1 << 5,
  // Outputs all layers as a layer tree. The default is output children
  // (excluding the root) as a layer list, in paint (preorder) order.
  kOutputAsLayerTree = 1 << 6,
};
typedef unsigned LayerTreeFlags;

class PLATFORM_EXPORT GraphicsLayerClient {
 public:
  virtual ~GraphicsLayerClient() {}

  virtual void InvalidateTargetElementForTesting() {}

  virtual IntRect ComputeInterestRect(
      const GraphicsLayer*,
      const IntRect& previous_interest_rect) const = 0;
  virtual LayoutSize SubpixelAccumulation() const { return LayoutSize(); }
  // Returns whether the client needs to be repainted with respect to the given
  // graphics layer.
  virtual bool NeedsRepaint(const GraphicsLayer&) const = 0;
  virtual void PaintContents(const GraphicsLayer*,
                             GraphicsContext&,
                             GraphicsLayerPaintingPhase,
                             const IntRect& interest_rect) const = 0;

  virtual bool IsTrackingRasterInvalidations() const { return false; }

  virtual String DebugName(const GraphicsLayer*) const = 0;

#if DCHECK_IS_ON()
  // CompositedLayerMapping overrides this to verify that it is not
  // currently painting contents. An ASSERT fails, if it is.
  // This is executed in GraphicsLayer construction and destruction
  // to verify that we don't create or destroy GraphicsLayers
  // while painting.
  virtual void VerifyNotPainting() {}
#endif
};

}  // namespace blink

#endif  // GraphicsLayerClient_h
