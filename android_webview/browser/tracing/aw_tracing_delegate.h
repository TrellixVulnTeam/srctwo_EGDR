// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ANDROID_WEBVIEW_BROWSER_TRACING_AW_TRACING_DELEGATE_H_
#define ANDROID_WEBVIEW_BROWSER_TRACING_AW_TRACING_DELEGATE_H_

#include "content/public/browser/tracing_delegate.h"

namespace android_webview {

class AwTracingDelegate : public content::TracingDelegate {
 public:
  AwTracingDelegate();
  ~AwTracingDelegate() override;

  // content::TracingDelegate implementation:
  std::unique_ptr<content::TraceUploader> GetTraceUploader(
      net::URLRequestContextGetter* request_context) override;
  void GenerateMetadataDict(base::DictionaryValue* metadata_dict) override;
};

}  // namespace android_webview

#endif  // ANDROID_WEBVIEW_BROWSER_TRACING_AW_TRACING_DELEGATE_H_
