/*
 * Copyright (C) 2012 Google Inc. All rights reserved.
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

#include "public/platform/WebRTCStatsRequest.h"

#include "platform/peerconnection/RTCStatsRequest.h"
#include "platform/peerconnection/RTCStatsResponseBase.h"
#include "public/platform/WebMediaStream.h"
#include "public/platform/WebMediaStreamTrack.h"
#include "public/platform/WebRTCStatsResponse.h"

namespace blink {

WebRTCStatsRequest::WebRTCStatsRequest(RTCStatsRequest* request)
    : private_(request) {}

void WebRTCStatsRequest::Assign(const WebRTCStatsRequest& other) {
  private_ = other.private_;
}

void WebRTCStatsRequest::Reset() {
  private_.Reset();
}

WebRTCStatsResponse WebRTCStatsRequest::CreateResponse() const {
  return WebRTCStatsResponse(private_->CreateResponse());
}

bool WebRTCStatsRequest::HasSelector() const {
  return private_->HasSelector();
}

const WebMediaStreamTrack WebRTCStatsRequest::Component() const {
  return WebMediaStreamTrack(private_->Component());
}

void WebRTCStatsRequest::RequestSucceeded(
    const WebRTCStatsResponse& response) const {
  private_->RequestSucceeded(response);
}

}  // namespace blink
