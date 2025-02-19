// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_BROWSER_TRACING_DELEGATE_H_
#define CONTENT_PUBLIC_BROWSER_TRACING_DELEGATE_H_

#include <memory>
#include <string>

#include "base/callback.h"
#include "content/common/content_export.h"

namespace base {
class DictionaryValue;
}

namespace net {
class URLRequestContextGetter;
}

namespace content {
class BackgroundTracingConfig;
class TraceUploader;

typedef base::Callback<bool(const std::string& metadata_name)>
    MetadataFilterPredicate;

// This can be implemented by the embedder to provide functionality for the
// about://tracing WebUI.
class CONTENT_EXPORT TracingDelegate {
 public:
  virtual ~TracingDelegate() {}

  // Provide trace uploading functionality; see trace_uploader.h.
  virtual std::unique_ptr<TraceUploader> GetTraceUploader(
      net::URLRequestContextGetter* request_context) = 0;

  // This can be used to veto a particular background tracing scenario.
  virtual bool IsAllowedToBeginBackgroundScenario(
      const BackgroundTracingConfig& config,
      bool requires_anonymized_data);

  virtual bool IsAllowedToEndBackgroundScenario(
      const content::BackgroundTracingConfig& config,
      bool requires_anonymized_data);

  // Used to add any additional metadata to traces.
  virtual void GenerateMetadataDict(base::DictionaryValue* metadata_dict) {}

  virtual MetadataFilterPredicate GetMetadataFilterPredicate();
};

}  // namespace content

#endif  // CONTENT_PUBLIC_BROWSER_TRACING_DELEGATE_H_
