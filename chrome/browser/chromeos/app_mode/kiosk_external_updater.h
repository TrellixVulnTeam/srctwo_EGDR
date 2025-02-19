// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_APP_MODE_KIOSK_EXTERNAL_UPDATER_H_
#define CHROME_BROWSER_CHROMEOS_APP_MODE_KIOSK_EXTERNAL_UPDATER_H_

#include <memory>
#include <string>

#include "base/files/file_path.h"
#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "base/observer_list.h"
#include "base/sequenced_task_runner.h"
#include "chrome/browser/chromeos/app_mode/kiosk_external_update_validator.h"
#include "chromeos/disks/disk_mount_manager.h"

namespace chromeos {

class KioskExternalUpdateNotification;

// Observes the disk mount/unmount events, scans the usb stick for external
// kiosk app updates, validates the external crx, and updates the cache.
class KioskExternalUpdater : public disks::DiskMountManager::Observer,
                             public KioskExternalUpdateValidatorDelegate {
 public:
  enum ExternalUpdateErrorCode {
    ERROR_NONE,
    ERROR_NO_MANIFEST,
    ERROR_INVALID_MANIFEST,
  };

  KioskExternalUpdater(
      const scoped_refptr<base::SequencedTaskRunner>& backend_task_runner,
      const base::FilePath& crx_cache_dir,
      const base::FilePath& crx_unpack_dir);

  ~KioskExternalUpdater() override;

 private:
  enum ExternalUpdateStatus {
    PENDING,
    SUCCESS,
    FAILED,
  };
  struct ExternalUpdate {
    ExternalUpdate();
    ExternalUpdate(const ExternalUpdate& other);
    ~ExternalUpdate();

    std::string app_name;
    extensions::CRXFileInfo external_crx;
    ExternalUpdateStatus update_status;
    base::string16 error;
  };

  // disks::DiskMountManager::Observer overrides.
  void OnDiskEvent(disks::DiskMountManager::DiskEvent event,
                   const disks::DiskMountManager::Disk* disk) override;
  void OnDeviceEvent(disks::DiskMountManager::DeviceEvent event,
                     const std::string& device_path) override;
  void OnMountEvent(
      disks::DiskMountManager::MountEvent event,
      MountError error_code,
      const disks::DiskMountManager::MountPointInfo& mount_info) override;
  void OnFormatEvent(disks::DiskMountManager::FormatEvent event,
                     FormatError error_code,
                     const std::string& device_path) override;
  void OnRenameEvent(disks::DiskMountManager::RenameEvent event,
                     RenameError error_code,
                     const std::string& device_path) override;

  // KioskExternalUpdateValidatorDelegate overrides:
  void OnExtenalUpdateUnpackSuccess(const std::string& app_id,
                                    const std::string& version,
                                    const std::string& min_browser_version,
                                    const base::FilePath& temp_dir) override;
  void OnExternalUpdateUnpackFailure(const std::string& app_id) override;

  // Processes the parsed external update manifest, check |parsing_error| for
  // any manifest parsing error.
  void ProcessParsedManifest(ExternalUpdateErrorCode* parsing_error,
                             const base::FilePath& external_update_dir,
                             base::DictionaryValue* parsed_manifest);

  // Returns true if |external_update_| is interrupted before the updating
  // completes.
  bool CheckExternalUpdateInterrupted();

  // Validates the external updates.
  void ValidateExternalUpdates();

  // Returns true if there are any external updates pending.
  bool IsExternalUpdatePending();

  // Returns true if all external updates specified in the manifest are
  // completed successfully.
  bool IsAllExternalUpdatesSucceeded();

  // Returns true if the app with |app_id| should be updated to
  // |external_extension|.
  bool ShouldDoExternalUpdate(const std::string& app_id,
                              const std::string& version,
                              const std::string& min_browser_version);

  // Installs the validated extension into cache.
  // |*crx_copied| indicates whether the |crx_file| is copied successfully.
  void PutValidatedExtension(bool* crx_copied,
                             const std::string& app_id,
                             const base::FilePath& crx_file,
                             const std::string& version);

  // Called upon completion of installing the validated external extension into
  // the local cache. |success| is true if the operation succeeded.
  void OnPutValidatedExtension(const std::string& app_id, bool success);

  void NotifyKioskUpdateProgress(const base::string16& message);

  void MaybeValidateNextExternalUpdate();

  // Notifies the kiosk update status with UI and KioskAppUpdateService, if
  // there is no kiosk external updates pending.
  void MayBeNotifyKioskAppUpdate();

  void NotifyKioskAppUpdateAvailable();

  // Dismisses the UI notification for kiosk updates.
  void DismissKioskUpdateNotification();

  // Return a detailed message for kiosk updating status.
  base::string16 GetUpdateReportMessage();

  // Task runner for executing file I/O tasks.
  const scoped_refptr<base::SequencedTaskRunner> backend_task_runner_;

  // The directory where kiosk crx files are cached.
  const base::FilePath crx_cache_dir_;

  // The directory used by SandBoxedUnpacker for unpack extensions.
  const base::FilePath crx_unpack_dir_;

  // The path where external crx files resides(usb stick mount path).
  base::FilePath external_update_path_;

  // map of app_id: ExternalUpdate
  typedef std::map<std::string, ExternalUpdate> ExternalUpdateMap;
  ExternalUpdateMap external_updates_;
  std::unique_ptr<KioskExternalUpdateNotification> notification_;

  base::WeakPtrFactory<KioskExternalUpdater> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(KioskExternalUpdater);
};

}  // namespace chromeos

#endif  // CHROME_BROWSER_CHROMEOS_APP_MODE_KIOSK_EXTERNAL_UPDATER_H_
