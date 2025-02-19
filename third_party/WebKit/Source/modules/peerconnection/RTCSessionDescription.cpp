/*
 * Copyright (C) 2012 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer
 *    in the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of Google Inc. nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
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

#include "modules/peerconnection/RTCSessionDescription.h"

#include "bindings/core/v8/ScriptValue.h"
#include "bindings/core/v8/V8ObjectBuilder.h"
#include "core/dom/ExecutionContext.h"
#include "core/frame/UseCounter.h"
#include "modules/peerconnection/RTCSessionDescriptionInit.h"

namespace blink {

RTCSessionDescription* RTCSessionDescription::Create(
    ExecutionContext* context,
    const RTCSessionDescriptionInit& description_init_dict) {
  String type;
  if (description_init_dict.hasType())
    type = description_init_dict.type();
  else
    UseCounter::Count(context, WebFeature::kRTCSessionDescriptionInitNoType);

  String sdp;
  if (description_init_dict.hasSdp())
    sdp = description_init_dict.sdp();
  else
    UseCounter::Count(context, WebFeature::kRTCSessionDescriptionInitNoSdp);

  return new RTCSessionDescription(WebRTCSessionDescription(type, sdp));
}

RTCSessionDescription* RTCSessionDescription::Create(
    WebRTCSessionDescription web_session_description) {
  return new RTCSessionDescription(web_session_description);
}

RTCSessionDescription::RTCSessionDescription(
    WebRTCSessionDescription web_session_description)
    : web_session_description_(web_session_description) {}

String RTCSessionDescription::type() {
  return web_session_description_.GetType();
}

void RTCSessionDescription::setType(const String& type) {
  web_session_description_.SetType(type);
}

String RTCSessionDescription::sdp() {
  return web_session_description_.Sdp();
}

void RTCSessionDescription::setSdp(const String& sdp) {
  web_session_description_.SetSDP(sdp);
}

ScriptValue RTCSessionDescription::toJSONForBinding(ScriptState* script_state) {
  V8ObjectBuilder result(script_state);
  result.AddStringOrNull("type", type());
  result.AddStringOrNull("sdp", sdp());
  return result.GetScriptValue();
}

WebRTCSessionDescription RTCSessionDescription::WebSessionDescription() {
  return web_session_description_;
}

}  // namespace blink
