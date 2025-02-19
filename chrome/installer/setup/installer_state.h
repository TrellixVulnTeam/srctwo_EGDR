// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_INSTALLER_SETUP_INSTALLER_STATE_H_
#define CHROME_INSTALLER_SETUP_INSTALLER_STATE_H_

#include <memory>

#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/macros.h"
#include "base/strings/string16.h"
#include "base/version.h"
#include "build/build_config.h"
#include "chrome/installer/setup/progress_calculator.h"
#include "chrome/installer/util/browser_distribution.h"
#include "chrome/installer/util/product.h"
#include "chrome/installer/util/util_constants.h"

#if defined(OS_WIN)
#include <windows.h>  // NOLINT
#endif

namespace base {
class CommandLine;
}

namespace installer {

class InstallationState;
class MasterPreferences;

class ProductState;

// Encapsulates the state of the current installation operation. This class
// interprets the command-line arguments and master preferences and determines
// the operations to be performed.
class InstallerState {
 public:
  enum Level {
    UNKNOWN_LEVEL,
    USER_LEVEL,
    SYSTEM_LEVEL
  };

  enum Operation {
    UNINITIALIZED,
    SINGLE_INSTALL_OR_UPDATE,
    UNINSTALL
  };

  // Constructs an uninitialized instance; see Initialize().
  InstallerState();

  // Constructs an initialized but empty instance.
  explicit InstallerState(Level level);

  ~InstallerState();

  // Initializes this object based on the current operation.
  void Initialize(const base::CommandLine& command_line,
                  const MasterPreferences& prefs,
                  const InstallationState& machine_state);

  // Adds a product constructed on the basis of |state|, setting this object's
  // msi flag if |state| is msi-installed. Returns the product that was added,
  // or NULL if |state| is incompatible with this object. Ownership is not
  // passed to the caller.
  Product* AddProductFromState(const ProductState& state);

  // Returns the product that was added, or NULL if |product| is incompatible
  // with this object. Ownership of the return value is not given to the caller.
  Product* AddProduct(std::unique_ptr<Product> product);

  // The level (user or system) of this operation.
  Level level() const { return level_; }

  // An identifier of this operation.
  Operation operation() const { return operation_; }

  // A convenience method returning level() == SYSTEM_LEVEL.
  bool system_install() const;

  // The full path to the place where the operand resides.
  const base::FilePath& target_path() const { return target_path_; }

  // Sets the value returned by target_path().
  void set_target_path_for_testing(const base::FilePath& target_path) {
    target_path_ = target_path;
  }

  // True if the "msi" preference is set or if a product with the "msi" state
  // flag is set is to be operated on.
  bool is_msi() const { return msi_; }

  // True if the process is running at a reduced "background" priority.
  bool is_background_mode() const { return background_mode_; }

  // Indicate that the process is or is not running in the background.
  void set_background_mode(bool bg) { background_mode_ = bg; }

  // True if the --verbose-logging command-line flag is set or if the
  // verbose_logging master preferences option is true.
  bool verbose_logging() const { return verbose_logging_; }

#if defined(OS_WIN)
  HKEY root_key() const { return root_key_; }
#endif

  // The ClientState key by which we interact with Google Update.
  const base::string16& state_key() const { return state_key_; }

  // Returns true if this is an update of multi-install Chrome to
  // single-install.
  bool is_migrating_to_single() const { return is_migrating_to_single_; }

  const Product& product() const {
    DCHECK(product_);
    return *product_;
  }

  // Returns the currently installed version in |target_path|, or NULL if no
  // products are installed. Ownership is passed to the caller.
  base::Version* GetCurrentVersion(
      const InstallationState& machine_state) const;

  // Returns the critical update version if all of the following are true:
  // * --critical-update-version=CUV was specified on the command-line.
  // * current_version == NULL or current_version < CUV.
  // * new_version >= CUV.
  // Otherwise, returns an invalid version.
  base::Version DetermineCriticalVersion(
      const base::Version* current_version,
      const base::Version& new_version) const;

  // Returns the path to the installer under Chrome version folder
  // (for example <target_path>\Google\Chrome\Application\<Version>\Installer)
  base::FilePath GetInstallerDirectory(const base::Version& version) const;

  // Sets the current stage of processing. This reports a progress value to
  // Google Update for presentation to a user.
  void SetStage(InstallerStage stage) const;

  // Strips all evidence of multi-install from Chrome's "ap" value.
  void UpdateChannels() const;

  // Sets installer result information in the registry for consumption by Google
  // Update. The InstallerResult value is set to 0 (SUCCESS) or 1
  // (FAILED_CUSTOM_ERROR) depending on whether |status| maps to success or not.
  // |status| itself is written to the InstallerError value.
  // |string_resource_id|, if non-zero, identifies a localized string written to
  // the InstallerResultUIString value. |launch_cmd|, if non-NULL and non-empty,
  // is written to the InstallerSuccessLaunchCmdLine value.
  void WriteInstallerResult(InstallStatus status,
                            int string_resource_id,
                            const base::string16* launch_cmd) const;

  // Returns true if this install needs to register an Active Setup command.
  bool RequiresActiveSetup() const;

 protected:
  // Clears the instance to an uninitialized state.
  void Clear();

  bool CanAddProduct(const base::FilePath* product_dir) const;
  Product* AddProductInDirectory(const base::FilePath* product_dir,
                                 std::unique_ptr<Product> product);
  Product* AddProductFromPreferences(
      const MasterPreferences& prefs,
      const InstallationState& machine_state);

  // Sets this object's level and updates the root_key_ accordingly.
  void set_level(Level level);

  Operation operation_;
  base::FilePath target_path_;
  base::string16 state_key_;
  std::unique_ptr<Product> product_;
  base::Version critical_update_version_;
  ProgressCalculator progress_calculator_;
  Level level_;
#if defined(OS_WIN)
  HKEY root_key_;
#endif
  bool msi_;
  bool background_mode_;
  bool verbose_logging_;
  bool is_migrating_to_single_;

 private:
  DISALLOW_COPY_AND_ASSIGN(InstallerState);
};  // class InstallerState

}  // namespace installer

#endif  // CHROME_INSTALLER_SETUP_INSTALLER_STATE_H_
