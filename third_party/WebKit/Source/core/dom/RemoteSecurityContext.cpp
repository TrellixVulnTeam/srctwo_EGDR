// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/dom/RemoteSecurityContext.h"

#include "core/frame/csp/ContentSecurityPolicy.h"
#include "platform/weborigin/SecurityOrigin.h"
#include "platform/wtf/Assertions.h"

namespace blink {

RemoteSecurityContext::RemoteSecurityContext() : SecurityContext() {
  // RemoteSecurityContext's origin is expected to stay uninitialized until
  // we set it using replicated origin data from the browser process.
  DCHECK(!GetSecurityOrigin());

  // Start with a clean slate.
  SetContentSecurityPolicy(ContentSecurityPolicy::Create());

  // FIXME: Document::initSecurityContext has a few other things we may
  // eventually want here, such as enforcing a setting to
  // grantUniversalAccess().
}

RemoteSecurityContext* RemoteSecurityContext::Create() {
  return new RemoteSecurityContext();
}

DEFINE_TRACE(RemoteSecurityContext) {
  SecurityContext::Trace(visitor);
}

void RemoteSecurityContext::SetReplicatedOrigin(RefPtr<SecurityOrigin> origin) {
  DCHECK(origin);
  SetSecurityOrigin(std::move(origin));
  GetContentSecurityPolicy()->SetupSelf(*GetSecurityOrigin());
}

void RemoteSecurityContext::ResetReplicatedContentSecurityPolicy() {
  DCHECK(GetSecurityOrigin());
  SetContentSecurityPolicy(ContentSecurityPolicy::Create());
  GetContentSecurityPolicy()->SetupSelf(*GetSecurityOrigin());
}

}  // namespace blink
