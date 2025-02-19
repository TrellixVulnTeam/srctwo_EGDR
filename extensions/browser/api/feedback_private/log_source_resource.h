// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXTENSIONS_BROWSER_API_FEEDBACK_PRIVATE_LOG_SOURCE_RESOURCE_H_
#define EXTENSIONS_BROWSER_API_FEEDBACK_PRIVATE_LOG_SOURCE_RESOURCE_H_

#include <memory>

#include "base/callback_helpers.h"
#include "base/macros.h"
#include "components/feedback/system_logs/system_logs_fetcher.h"
#include "extensions/browser/api/api_resource.h"
#include "extensions/browser/api/api_resource_manager.h"
#include "extensions/browser/api/feedback_private/log_source_access_manager.h"

namespace extensions {

// Holds a SystemLogsSource object that is used by an extension using the
// feedbackPrivate API.
class LogSourceResource : public ApiResource {
 public:
  static const content::BrowserThread::ID kThreadId =
      content::BrowserThread::UI;

  LogSourceResource(const std::string& extension_id,
                    std::unique_ptr<system_logs::SystemLogsSource> source,
                    base::Closure unregister_callback_);

  ~LogSourceResource() override;

  system_logs::SystemLogsSource* GetLogSource() const { return source_.get(); }

 private:
  friend class ApiResourceManager<LogSourceResource>;
  static const char* service_name() { return "LogSourceResource"; }

  std::unique_ptr<system_logs::SystemLogsSource> source_;

  // This unregisters the LogSourceResource from a LogSourceAccessManager when
  // this resource is cleaned up. Just pass in a base::Closure to the
  // constructor.
  base::ScopedClosureRunner unregister_runner_;

  DISALLOW_COPY_AND_ASSIGN(LogSourceResource);
};

}  // namespace extensions

#endif  // EXTENSIONS_BROWSER_API_FEEDBACK_PRIVATE_LOG_SOURCE_RESOURCE_H_
