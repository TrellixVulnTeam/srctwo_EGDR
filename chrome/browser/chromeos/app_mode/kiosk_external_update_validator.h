// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_APP_MODE_KIOSK_EXTERNAL_UPDATE_VALIDATOR_H_
#define CHROME_BROWSER_CHROMEOS_APP_MODE_KIOSK_EXTERNAL_UPDATE_VALIDATOR_H_

#include <string>

#include "base/files/file_path.h"
#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "base/sequenced_task_runner.h"
#include "extensions/browser/sandboxed_unpacker.h"

namespace extensions {
class Extension;
}

namespace chromeos {

// Delegate class for KioskExternalUpdateValidator, derived class must support
// WeakPtr.
class KioskExternalUpdateValidatorDelegate {
 public:
  virtual void OnExtenalUpdateUnpackSuccess(
      const std::string& app_id,
      const std::string& version,
      const std::string& min_browser_version,
      const base::FilePath& temp_dir) = 0;
  virtual void OnExternalUpdateUnpackFailure(const std::string& app_id) = 0;

 protected:
  virtual ~KioskExternalUpdateValidatorDelegate() {}
};

// Unpacks the crx file of the kiosk app and validates its signature.
class KioskExternalUpdateValidator
    : public extensions::SandboxedUnpackerClient {
 public:
  KioskExternalUpdateValidator(
      const scoped_refptr<base::SequencedTaskRunner>& backend_task_runner,
      const extensions::CRXFileInfo& file,
      const base::FilePath& crx_unpack_dir,
      const base::WeakPtr<KioskExternalUpdateValidatorDelegate>& delegate);

  // Starts validating the external crx file.
  void Start();

 private:
  ~KioskExternalUpdateValidator() override;

  // SandboxedUnpackerClient overrides.
  void OnUnpackFailure(const extensions::CrxInstallError& error) override;
  void OnUnpackSuccess(const base::FilePath& temp_dir,
                       const base::FilePath& extension_dir,
                       std::unique_ptr<base::DictionaryValue> original_manifest,
                       const extensions::Extension* extension,
                       const SkBitmap& install_icon) override;

  // Task runner for executing file I/O tasks.
  const scoped_refptr<base::SequencedTaskRunner> backend_task_runner_;

  // Information about the external crx file.
  extensions::CRXFileInfo crx_file_;

  // The temporary directory used by SandBoxedUnpacker for unpacking extensions.
  const base::FilePath crx_unpack_dir_;

  base::WeakPtr<KioskExternalUpdateValidatorDelegate> delegate_;

  DISALLOW_COPY_AND_ASSIGN(KioskExternalUpdateValidator);
};

}  // namespace chromeos

#endif  // CHROME_BROWSER_CHROMEOS_APP_MODE_KIOSK_EXTERNAL_UPDATE_VALIDATOR_H_
