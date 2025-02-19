// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_DOWNLOAD_PUBLIC_DOWNLOAD_SERVICE_H_
#define COMPONENTS_DOWNLOAD_PUBLIC_DOWNLOAD_SERVICE_H_

#include <memory>
#include <string>

#include "base/files/file_path.h"
#include "base/macros.h"
#include "base/memory/ptr_util.h"
#include "base/memory/ref_counted.h"
#include "base/sequenced_task_runner.h"
#include "components/download/public/clients.h"
#include "components/download/public/download_task_types.h"
#include "components/keyed_service/core/keyed_service.h"

namespace download {

class Client;
struct DownloadParams;
struct SchedulingParams;
class ServiceConfig;

using TaskFinishedCallback = base::Callback<void(bool)>;

// A service responsible for helping facilitate the scheduling and downloading
// of file content from the web.  See |DownloadParams| for more details on the
// types of scheduling that can be achieved and the required input parameters
// for starting a download.  Note that DownloadServices with a valid storage
// directory will persist the requests across restarts.  This means that any
// feature requesting a download will have to implement a download::Client
// interface so this class knows who to contact when a download completes after
// a process restart.
// See the embedder specific factories for creation options.
class DownloadService : public KeyedService {
 public:
  // The current status of the Service.
  enum class ServiceStatus {
    // The service is in the process of initializing and should not be used yet.
    // All registered Clients will be notified via
    // Client::OnServiceInitialized() once the service is ready.
    STARTING_UP = 0,

    // The service is ready and available for use.
    READY = 1,

    // The service is unavailable.  This is typically due to an unrecoverable
    // error on some internal component like the persistence layer.
    UNAVAILABLE = 2,
  };

  // Returns useful configuration information about the DownloadService.
  virtual const ServiceConfig& GetConfig() = 0;

  // Callback method to run by the service when a pre-scheduled task starts.
  // This method is invoked on main thread and while it is running, the system
  // holds a wakelock which is not released until either the |callback| is run
  // or OnStopScheduledTask is invoked by the system. Do not call this method
  // directly.
  virtual void OnStartScheduledTask(DownloadTaskType task_type,
                                    const TaskFinishedCallback& callback) = 0;

  // Callback method to run by the service if the system decides to stop the
  // task. Returns true if the task needs to be rescheduled. Any pending
  // TaskFinishedCallback should be reset after this call. Do not call this
  // method directly.
  virtual bool OnStopScheduledTask(DownloadTaskType task_type) = 0;

  // Whether or not the DownloadService is currently available, initialized
  // successfully, and ready to be used.
  virtual ServiceStatus GetStatus() = 0;

  // Sends the download to the service.  A callback to
  // |DownloadParams::callback| will be triggered once the download has been
  // persisted and saved in the service.
  virtual void StartDownload(const DownloadParams& download_params) = 0;

  // Allows any feature to pause or resume downloads at will.  Paused downloads
  // will not start or stop based on scheduling criteria.  They will be
  // effectively frozen.
  virtual void PauseDownload(const std::string& guid) = 0;
  virtual void ResumeDownload(const std::string& guid) = 0;

  // Cancels a download in this service.  The canceled download will be
  // interrupted if it is running.
  virtual void CancelDownload(const std::string& guid) = 0;

  // Changes the current scheduling criteria for a download.  This is useful if
  // a user action might constrain or loosen the device state during which this
  // download can run.
  virtual void ChangeDownloadCriteria(const std::string& guid,
                                      const SchedulingParams& params) = 0;

 protected:
  DownloadService() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(DownloadService);
};

}  // namespace download

#endif  // COMPONENTS_DOWNLOAD_PUBLIC_DOWNLOAD_SERVICE_H_
