// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_FEEDBACK_SYSTEM_LOGS_SYSTEM_LOGS_SOURCE_H_
#define COMPONENTS_FEEDBACK_SYSTEM_LOGS_SYSTEM_LOGS_SOURCE_H_

#include <string>

#include "base/callback.h"
#include "components/feedback/feedback_common.h"

namespace system_logs {

using SystemLogsResponse = FeedbackCommon::SystemLogsMap;

// Callback that the data sources use to return data.
using SysLogsSourceCallback = base::Callback<void(SystemLogsResponse*)>;

// The SystemLogsSource provides an interface for the data sources that
// the SystemLogsFetcher class uses to fetch logs and other information.
class SystemLogsSource {
 public:
  // |source_name| provides a descriptive identifier for debugging.
  explicit SystemLogsSource(const std::string& source_name);
  virtual ~SystemLogsSource();

  // Fetches data and passes it by pointer to the callback
  virtual void Fetch(const SysLogsSourceCallback& callback) = 0;

  const std::string& source_name() const { return source_name_; }

 private:
  std::string source_name_;
};

}  // namespace system_logs

#endif  // COMPONENTS_FEEDBACK_SYSTEM_LOGS_SYSTEM_LOGS_SOURCE_H_
