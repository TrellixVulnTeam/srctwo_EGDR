// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_WEB_VIEW_INTERNAL_TRANSLATE_WEB_VIEW_TRANSLATE_SERVICE_H_
#define IOS_WEB_VIEW_INTERNAL_TRANSLATE_WEB_VIEW_TRANSLATE_SERVICE_H_

#include "base/macros.h"
#include "base/memory/ptr_util.h"
#include "components/web_resource/resource_request_allowed_notifier.h"

namespace base {
template <typename T>
struct DefaultSingletonTraits;
}

namespace ios_web_view {

// Singleton managing the resources required for Translate.
class WebViewTranslateService {
 public:
  static WebViewTranslateService* GetInstance();

  // Must be called before the Translate feature can be used.
  void Initialize();

  // Must be called to shut down the Translate feature.
  void Shutdown();

 private:
  // Manages enabling translate requests only when resource requests are
  // allowed.
  // TODO(crbug.com/728776): Merge TranslateRequestsAllowedListener and
  // WebViewTranslateService. They currently must be separate classes because
  // the destructor of web_resource::ResourceRequestAllowedNotifier::Observer is
  // not virtual.
  class TranslateRequestsAllowedListener
      : public web_resource::ResourceRequestAllowedNotifier::Observer {
   public:
    TranslateRequestsAllowedListener();
    ~TranslateRequestsAllowedListener();

    // ResourceRequestAllowedNotifier::Observer methods.
    void OnResourceRequestsAllowed() override;

   private:
    // Notifier class to know if it's allowed to make network resource requests.
    web_resource::ResourceRequestAllowedNotifier
        resource_request_allowed_notifier_;

    DISALLOW_COPY_AND_ASSIGN(TranslateRequestsAllowedListener);
  };

  WebViewTranslateService();
  ~WebViewTranslateService();

  friend struct base::DefaultSingletonTraits<WebViewTranslateService>;

  // Listener which manages when translate requests can occur.
  TranslateRequestsAllowedListener translate_requests_allowed_listener_;

  DISALLOW_COPY_AND_ASSIGN(WebViewTranslateService);
};

}  // namespace ios_web_view

#endif  // IOS_WEB_VIEW_INTERNAL_TRANSLATE_WEB_VIEW_TRANSLATE_SERVICE_H_
