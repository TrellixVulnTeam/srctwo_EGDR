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

#ifndef WebSocketHandle_h
#define WebSocketHandle_h

#include <stdint.h>
#include "platform/wtf/Forward.h"
#include "platform/wtf/Vector.h"
#include "public/platform/modules/websockets/websocket.mojom-blink.h"

namespace blink {

class KURL;
class SecurityOrigin;
class WebSocketHandleClient;

// WebSocketHandle is an interface class designed to be a handle of WebSocket
// connection.  WebSocketHandle will be used together with
// WebSocketHandleClient.
//
// Once a WebSocketHandle is deleted there will be no notification to the
// corresponding WebSocketHandleClient.  Once a WebSocketHandleClient receives
// didClose, any method of the corresponding WebSocketHandle can't be called.

class WebSocketHandle {
 public:
  enum MessageType {
    kMessageTypeContinuation,
    kMessageTypeText,
    kMessageTypeBinary,
  };

  virtual ~WebSocketHandle() {}

  virtual void Initialize(mojom::blink::WebSocketPtr) = 0;
  virtual void Connect(const KURL&,
                       const Vector<String>& protocols,
                       SecurityOrigin*,
                       const KURL& site_for_cookies,
                       const String& user_agent_override,
                       WebSocketHandleClient*) = 0;
  virtual void Send(bool fin, MessageType, const char* data, size_t) = 0;
  virtual void FlowControl(int64_t quota) = 0;
  virtual void Close(unsigned short code, const String& reason) = 0;
};

}  // namespace blink

#endif  // WebSocketHandle_h
