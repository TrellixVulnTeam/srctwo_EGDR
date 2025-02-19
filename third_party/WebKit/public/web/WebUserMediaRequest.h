/*
 * Copyright (C) 2011 Google Inc. All rights reserved.
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

#ifndef WebUserMediaRequest_h
#define WebUserMediaRequest_h

#include "public/platform/WebCommon.h"
#include "public/platform/WebPrivatePtr.h"
#include "public/platform/WebSecurityOrigin.h"
#include "public/platform/WebString.h"

namespace blink {

class UserMediaRequest;
class WebDocument;
class WebMediaConstraints;
class WebMediaStream;

class BLINK_EXPORT WebUserMediaRequest {
 public:
  WebUserMediaRequest() {}
  WebUserMediaRequest(const WebUserMediaRequest& request) { Assign(request); }
  ~WebUserMediaRequest() { Reset(); }

  WebUserMediaRequest& operator=(const WebUserMediaRequest& other) {
    Assign(other);
    return *this;
  }

  void Reset();
  bool IsNull() const { return private_.IsNull(); }
  bool Equals(const WebUserMediaRequest&) const;
  void Assign(const WebUserMediaRequest&);

  bool Audio() const;
  bool Video() const;
  WebMediaConstraints AudioConstraints() const;
  WebMediaConstraints VideoConstraints() const;

  WebSecurityOrigin GetSecurityOrigin() const;
  WebDocument OwnerDocument() const;

  void RequestSucceeded(const WebMediaStream&);

  void RequestDenied(const WebString& description = WebString());
  void RequestFailedConstraint(const WebString& constraint_name,
                               const WebString& description = WebString());
  void RequestFailedUASpecific(const WebString& name,
                               const WebString& constraint_name = WebString(),
                               const WebString& description = WebString());

  // DEPRECATED
  void RequestFailed(const WebString& description = WebString()) {
    RequestDenied(description);
  }

  // For testing in content/
  static WebUserMediaRequest CreateForTesting(const WebMediaConstraints& audio,
                                              const WebMediaConstraints& video);

#if INSIDE_BLINK
  WebUserMediaRequest(UserMediaRequest*);
  operator UserMediaRequest*() const;
#endif

 private:
  WebPrivatePtr<UserMediaRequest> private_;
};

inline bool operator==(const WebUserMediaRequest& a,
                       const WebUserMediaRequest& b) {
  return a.Equals(b);
}

}  // namespace blink

#endif  // WebUserMediaRequest_h
