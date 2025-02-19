// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file defines a service that sends metrics logs to a server.

#ifndef COMPONENTS_METRICS_METRICS_REPORTING_SERVICE_H_
#define COMPONENTS_METRICS_METRICS_REPORTING_SERVICE_H_

#include <stdint.h>

#include <string>

#include "base/macros.h"
#include "components/metrics/metrics_log_store.h"
#include "components/metrics/reporting_service.h"

class PrefService;
class PrefRegistrySimple;

namespace metrics {

class MetricsServiceClient;

// MetricsReportingService is concrete implementation of ReportingService for
// UMA logs. It uses a MetricsLogStore as its LogStore, reports to the UMA
// endpoint, and logs some histograms with the UMA prefix.
class MetricsReportingService : public ReportingService {
 public:
  // Creates a ReportingService with the given |client|, |local_state|.
  // Does not take ownership of the parameters; instead it stores a weak
  // pointer to each. Caller should ensure that the parameters are valid for
  // the lifetime of this class.
  MetricsReportingService(MetricsServiceClient* client,
                          PrefService* local_state);
  ~MetricsReportingService() override;

  MetricsLogStore* metrics_log_store() { return &metrics_log_store_; }
  const MetricsLogStore* metrics_log_store() const {
    return &metrics_log_store_;
  }

  // Registers local state prefs used by this class.
  static void RegisterPrefs(PrefRegistrySimple* registry);

 private:
  // ReportingService:
  LogStore* log_store() override;
  std::string GetUploadUrl() const override;
  base::StringPiece upload_mime_type() const override;
  MetricsLogUploader::MetricServiceType service_type() const override;
  void LogActualUploadInterval(base::TimeDelta interval) override;
  void LogCellularConstraint(bool upload_canceled) override;
  void LogResponseOrErrorCode(int response_code, int error_code) override;
  void LogSuccess(size_t log_size) override;
  void LogLargeRejection(size_t log_size) override;

  MetricsLogStore metrics_log_store_;

  DISALLOW_COPY_AND_ASSIGN(MetricsReportingService);
};

}  // namespace metrics

#endif  // COMPONENTS_METRICS_METRICS_REPORTING_SERVICE_H_
