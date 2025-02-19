/*
 * Copyright 2008, The Android Open Source Project
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
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

#ifndef Touch_h
#define Touch_h

#include "core/CoreExport.h"
#include "core/dom/Document.h"
#include "core/dom/events/EventTarget.h"
#include "core/input/TouchInit.h"
#include "platform/bindings/ScriptWrappable.h"
#include "platform/geometry/FloatPoint.h"
#include "platform/geometry/FloatSize.h"
#include "platform/geometry/LayoutPoint.h"
#include "platform/heap/Handle.h"

namespace blink {

class LocalFrame;

class CORE_EXPORT Touch final : public GarbageCollectedFinalized<Touch>,
                                public ScriptWrappable {
  DEFINE_WRAPPERTYPEINFO();

 public:
  static Touch* Create(LocalFrame* frame,
                       EventTarget* target,
                       int identifier,
                       const FloatPoint& screen_pos,
                       const FloatPoint& page_pos,
                       const FloatSize& radius,
                       float rotation_angle,
                       float force,
                       String region) {
    return new Touch(frame, target, identifier, screen_pos, page_pos, radius,
                     rotation_angle, force, region);
  }

  static Touch* Create(const Document& document, const TouchInit& initializer) {
    return new Touch(document.GetFrame(), initializer);
  }

  // DOM Touch implementation
  EventTarget* target() const { return target_.Get(); }
  int identifier() const { return identifier_; }
  double clientX() const { return client_pos_.X(); }
  double clientY() const { return client_pos_.Y(); }
  double screenX() const { return screen_pos_.X(); }
  double screenY() const { return screen_pos_.Y(); }
  double pageX() const { return page_pos_.X(); }
  double pageY() const { return page_pos_.Y(); }
  float radiusX() const { return radius_.Width(); }
  float radiusY() const { return radius_.Height(); }
  float rotationAngle() const { return rotation_angle_; }
  float force() const { return force_; }
  const String& region() const { return region_; }

  // Blink-internal methods
  const LayoutPoint& AbsoluteLocation() const { return absolute_location_; }
  const FloatPoint& ScreenLocation() const { return screen_pos_; }
  Touch* CloneWithNewTarget(EventTarget*) const;

  DECLARE_TRACE();

 private:
  Touch(LocalFrame*,
        EventTarget*,
        int identifier,
        const FloatPoint& screen_pos,
        const FloatPoint& page_pos,
        const FloatSize& radius,
        float rotation_angle,
        float force,
        String region);

  Touch(EventTarget*,
        int identifier,
        const FloatPoint& client_pos,
        const FloatPoint& screen_pos,
        const FloatPoint& page_pos,
        const FloatSize& radius,
        float rotation_angle,
        float force,
        String region,
        LayoutPoint absolute_location);

  Touch(LocalFrame*, const TouchInit&);

  Member<EventTarget> target_;
  int identifier_;
  // Position relative to the viewport in CSS px.
  FloatPoint client_pos_;
  // Position relative to the screen in DIPs.
  FloatPoint screen_pos_;
  // Position relative to the page in CSS px.
  FloatPoint page_pos_;
  // Radius in CSS px.
  FloatSize radius_;
  float rotation_angle_;
  float force_;
  String region_;
  // FIXME(rbyers): Shouldn't we be able to migrate callers to relying on
  // screenPos, pagePos or clientPos? absoluteLocation appears to be the same as
  // pagePos but without browser scale applied.
  LayoutPoint absolute_location_;
};

}  // namespace blink

#endif  // Touch_h
