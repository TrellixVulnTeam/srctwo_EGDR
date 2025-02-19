// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_SYSTEM_LOGS_DEBUG_DAEMON_LOG_SOURCE_H_
#define CHROME_BROWSER_CHROMEOS_SYSTEM_LOGS_DEBUG_DAEMON_LOG_SOURCE_H_

#include <map>
#include <string>
#include <vector>

#include "base/files/file_path.h"
#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "components/feedback/system_logs/system_logs_source.h"

namespace system_logs {

// Gathers log data from Debug Daemon.
class DebugDaemonLogSource : public SystemLogsSource {
 public:
  explicit DebugDaemonLogSource(bool scrub);
  ~DebugDaemonLogSource() override;

  // SystemLogsSource override:
  // Fetches logs from the daemon over dbus. After the fetch is complete, the
  // results will be forwarded to the request supplied to the constructor and
  // this instance will free itself.
  void Fetch(const SysLogsSourceCallback& callback) override;

 private:
  typedef std::map<std::string, std::string> KeyValueMap;

  // Callbacks for the 5 different dbus calls to debugd.
  void OnGetRoutes(bool succeeded, const std::vector<std::string>& routes);
  void OnGetNetworkStatus(bool succeeded, const std::string& status);
  void OnGetModemStatus(bool succeeded, const std::string& status);
  void OnGetWiMaxStatus(bool succeeded, const std::string& status);
  void OnGetLogs(bool succeeded,
                 const KeyValueMap& logs);
  void OnGetUserLogFiles(bool succeeded,
                         const KeyValueMap& logs);

  // Read the contents of the specified user logs files and adds it to
  // the response parameter.
  static void ReadUserLogFiles(
      const KeyValueMap& user_log_files,
      const std::vector<base::FilePath>& profile_dirs,
      SystemLogsResponse* response);

  // Merge the responses from ReadUserLogFiles into the main response dict and
  // call RequestComplete to indicate that the user log files read is complete.
  void MergeResponse(SystemLogsResponse* response);

  // Sends the data to the callback_ when all the requests are completed
  void RequestCompleted();

  std::unique_ptr<SystemLogsResponse> response_;
  SysLogsSourceCallback callback_;
  int num_pending_requests_;
  bool scrub_;
  base::WeakPtrFactory<DebugDaemonLogSource> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(DebugDaemonLogSource);
};


}  // namespace system_logs

#endif  // CHROME_BROWSER_CHROMEOS_SYSTEM_LOGS_DEBUG_DAEMON_LOG_SOURCE_H_
