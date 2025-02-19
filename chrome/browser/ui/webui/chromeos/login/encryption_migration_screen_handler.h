// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_CHROMEOS_LOGIN_ENCRYPTION_MIGRATION_SCREEN_HANDLER_H_
#define CHROME_BROWSER_UI_WEBUI_CHROMEOS_LOGIN_ENCRYPTION_MIGRATION_SCREEN_HANDLER_H_

#include <memory>

#include "base/callback_forward.h"
#include "base/macros.h"
#include "base/optional.h"
#include "chrome/browser/chromeos/login/screens/encryption_migration_mode.h"
#include "chrome/browser/chromeos/login/screens/encryption_migration_screen_view.h"
#include "chrome/browser/ui/webui/chromeos/login/base_screen_handler.h"
#include "chromeos/cryptohome/cryptohome_parameters.h"
#include "chromeos/dbus/power_manager_client.h"
#include "chromeos/login/auth/user_context.h"
#include "services/device/public/interfaces/wake_lock.mojom.h"
#include "third_party/cros_system_api/dbus/cryptohome/dbus-constants.h"

namespace base {
class TickClock;
class TimeTicks;
}  // namespace base

namespace chromeos {

class LoginFeedback;

// WebUI implementation of EncryptionMigrationScreenView
class EncryptionMigrationScreenHandler : public EncryptionMigrationScreenView,
                                         public BaseScreenHandler,
                                         public PowerManagerClient::Observer {
 public:
  EncryptionMigrationScreenHandler();
  ~EncryptionMigrationScreenHandler() override;

  // EncryptionMigrationScreenView implementation:
  void Show() override;
  void Hide() override;
  void SetDelegate(Delegate* delegate) override;
  void SetUserContext(const UserContext& user_context) override;
  void SetMode(EncryptionMigrationMode mode) override;
  void SetContinueLoginCallback(ContinueLoginCallback callback) override;
  void SetRestartLoginCallback(RestartLoginCallback callback) override;
  void SetupInitialView() override;

  // BaseScreenHandler implementation:
  void DeclareLocalizedValues(
      ::login::LocalizedValuesBuilder* builder) override;
  void Initialize() override;

 protected:
  // Callback that can be used to check free disk space.
  using FreeDiskSpaceFetcher = base::RepeatingCallback<int64_t()>;

  // Testing only: Sets the free disk space fetcher.
  void SetFreeDiskSpaceFetcherForTesting(
      FreeDiskSpaceFetcher free_disk_space_fetcher);
  // Testing only: Sets the tick clock used to measure elapsed time during
  // migration.
  void SetTickClockForTesting(std::unique_ptr<base::TickClock> tick_clock);

  virtual device::mojom::WakeLock* GetWakeLock();

 private:
  // Enumeration for migration UI state. These values must be kept in sync with
  // EncryptionMigrationUIState in JS code, and match the numbering for
  // MigrationUIScreen in histograms/enums.xml. Do not reorder or remove items,
  // only add new items before COUNT.
  enum UIState {
    INITIAL = 0,
    READY = 1,
    MIGRATING = 2,
    MIGRATION_FAILED = 3,
    NOT_ENOUGH_STORAGE = 4,
    MIGRATING_MINIMAL = 5,
    COUNT
  };

  // WebUIMessageHandler implementation:
  void RegisterMessages() override;

  // PowerManagerClient::Observer implementation:
  void PowerChanged(const power_manager::PowerSupplyProperties& proto) override;

  // Handlers for JS API callbacks.
  void HandleStartMigration();
  void HandleSkipMigration();
  void HandleRequestRestartOnLowStorage();
  void HandleRequestRestartOnFailure();
  void HandleOpenFeedbackDialog();

  // Updates UI state.
  void UpdateUIState(UIState state);

  void CheckAvailableStorage();
  void OnGetAvailableStorage(int64_t size);
  void WaitBatteryAndMigrate();
  void StartMigration();
  void OnMountExistingVault(bool success,
                            cryptohome::MountError return_code,
                            const std::string& mount_hash);
  // Removes cryptohome and shows the error screen after the removal finishes.
  void RemoveCryptohome();
  void OnRemoveCryptohome(bool success, cryptohome::MountError return_code);

  // Creates authorization key for MountEx method using |user_context_|.
  cryptohome::KeyDefinition GetAuthKey();

  // True if the session is in ARC kiosk mode.
  bool IsArcKiosk() const;

  // Handlers for cryptohome API callbacks.
  void OnMigrationProgress(cryptohome::DircryptoMigrationStatus status,
                           uint64_t current,
                           uint64_t total);
  void OnMigrationRequested(bool success);

  // Records UMA about visible screen after delay.
  void OnDelayedRecordVisibleScreen(UIState state);

  // True if |mode_| suggests that we are resuming an incomplete migration.
  bool IsResumingIncompleteMigration() const;

  // True if |mode_| suggests that migration should start immediately.
  bool IsStartImmediately() const;

  // True if |mode_| suggests that we are starting or resuming a minimal
  // migration.
  bool IsMinimalMigration() const;

  // Returns the UIState we should be in when migration is in progress.
  // This will be different between regular and minimal migration.
  UIState GetMigratingUIState() const;

  // Stop forcing migration if it was forced by policy.
  void MaybeStopForcingMigration();

  Delegate* delegate_ = nullptr;
  bool show_on_init_ = false;

  // The current UI state which should be refrected in the web UI.
  UIState current_ui_state_ = INITIAL;

  // The current user's UserContext, which is used to request the migration to
  // cryptohome.
  UserContext user_context_;

  // The callback which is used to log in to the session from the migration UI.
  ContinueLoginCallback continue_login_callback_;

  // The callback which is used to require the user to re-enter their password.
  RestartLoginCallback restart_login_callback_;

  // The migration mode (ask user / start migration automatically / resume
  // incomplete migratoin).
  EncryptionMigrationMode mode_ = EncryptionMigrationMode::ASK_USER;

  // The current battery level.
  base::Optional<double> current_battery_percent_;

  // True if the migration should start immediately once the battery level gets
  // sufficient.
  bool should_migrate_on_enough_battery_ = false;

  // The battery level at the timing that the migration starts.
  double initial_battery_percent_ = 0.0;

  // Point in time when minimal migration started, as reported by |tick_clock_|.
  base::TimeTicks minimal_migration_start_;

  device::mojom::WakeLockPtr wake_lock_;

  std::unique_ptr<LoginFeedback> login_feedback_;

  // Used to measure elapsed time during migration.
  std::unique_ptr<base::TickClock> tick_clock_;

  FreeDiskSpaceFetcher free_disk_space_fetcher_;

  base::WeakPtrFactory<EncryptionMigrationScreenHandler> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(EncryptionMigrationScreenHandler);
};

}  // namespace chromeos

#endif  // CHROME_BROWSER_UI_WEBUI_CHROMEOS_LOGIN_ENCRYPTION_MIGRATION_SCREEN_HANDLER_H_
