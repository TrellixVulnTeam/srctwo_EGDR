// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_BLINK_WEBMEDIACAPABILITIESCLIENT_IMPL_H_
#define MEDIA_BLINK_WEBMEDIACAPABILITIESCLIENT_IMPL_H_

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "media/blink/media_blink_export.h"
#include "third_party/WebKit/public/platform/modules/media_capabilities/WebMediaCapabilitiesClient.h"

namespace media {

class MEDIA_BLINK_EXPORT WebMediaCapabilitiesClientImpl
    : public blink::WebMediaCapabilitiesClient {
 public:
  WebMediaCapabilitiesClientImpl();
  ~WebMediaCapabilitiesClientImpl() override;

  // Implementation of blink::WebMediaCapabilitiesClient.
  void DecodingInfo(
      const blink::WebMediaConfiguration&,
      std::unique_ptr<blink::WebMediaCapabilitiesQueryCallbacks>) override;

 private:
  DISALLOW_COPY_AND_ASSIGN(WebMediaCapabilitiesClientImpl);
};

}  // namespace media

#endif  // MEDIA_BLINK_WEBMEDIACAPABILITIESCLIENT_IMPL_H_
