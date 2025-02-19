// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_DOWNLOAD_INTERNAL_TEST_MOCK_DOWNLOAD_SERVICE_H_
#define COMPONENTS_DOWNLOAD_INTERNAL_TEST_MOCK_DOWNLOAD_SERVICE_H_

#include "base/macros.h"
#include "components/download/public/download_params.h"
#include "components/download/public/download_service.h"
#include "components/download/public/service_config.h"
#include "testing/gmock/include/gmock/gmock.h"

namespace download {
namespace test {

class MockDownloadService : public DownloadService {
 public:
  MockDownloadService();
  ~MockDownloadService() override;

  // DownloadService implementation.
  MOCK_METHOD0(GetConfig, const ServiceConfig&());
  MOCK_METHOD2(OnStartScheduledTask,
               void(DownloadTaskType task_type,
                    const TaskFinishedCallback& callback));
  MOCK_METHOD1(OnStopScheduledTask, bool(DownloadTaskType task_type));
  MOCK_METHOD0(GetStatus, ServiceStatus());
  MOCK_METHOD1(StartDownload, void(const DownloadParams& download_params));
  MOCK_METHOD1(PauseDownload, void(const std::string& guid));
  MOCK_METHOD1(ResumeDownload, void(const std::string& guid));
  MOCK_METHOD1(CancelDownload, void(const std::string& guid));
  MOCK_METHOD2(ChangeDownloadCriteria,
               void(const std::string& guid, const SchedulingParams& params));

 private:
  DISALLOW_COPY_AND_ASSIGN(MockDownloadService);
};

}  // namespace test
}  // namespace download

#endif  // COMPONENTS_DOWNLOAD_INTERNAL_TEST_MOCK_DOWNLOAD_SERVICE_H_
