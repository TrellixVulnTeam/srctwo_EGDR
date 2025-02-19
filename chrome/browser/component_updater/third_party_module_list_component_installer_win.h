// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_COMPONENT_UPDATER_THIRD_PARTY_MODULE_LIST_COMPONENT_INSTALLER_WIN_H_
#define CHROME_BROWSER_COMPONENT_UPDATER_THIRD_PARTY_MODULE_LIST_COMPONENT_INSTALLER_WIN_H_

#include <stdint.h>

#include <memory>
#include <string>
#include <vector>

#include "components/component_updater/default_component_installer.h"

namespace base {
class FilePath;
}  // namespace base

// Will receive notifications about a new module list being available.
class ModuleListManager;

namespace component_updater {

class ComponentUpdateService;

// Component for receiving Third Party Module Lists. The lists are in proto
// format, corresponding to the proto definition in
// chrome/browser/conflicts/proto/module_list.proto
//
// Notifications of a new version of the module list are sent to the
// ModuleListManager.
class ThirdPartyModuleListComponentInstallerTraits
    : public ComponentInstallerTraits {
 public:
  // The |manager| will be notified each time a new module list is available,
  // including once every startup when a component is already installed.
  explicit ThirdPartyModuleListComponentInstallerTraits(
      ModuleListManager* manager);
  ~ThirdPartyModuleListComponentInstallerTraits() override;

 private:
  friend class ThirdPartyModuleListComponentInstallerTraitsTest;

  // ComponentInstallerTraits implementation.
  bool SupportsGroupPolicyEnabledComponentUpdates() const override;
  bool RequiresNetworkEncryption() const override;
  update_client::CrxInstaller::Result OnCustomInstall(
      const base::DictionaryValue& manifest,
      const base::FilePath& install_dir) override;
  bool VerifyInstallation(const base::DictionaryValue& manifest,
                          const base::FilePath& install_dir) const override;
  void ComponentReady(const base::Version& version,
                      const base::FilePath& install_dir,
                      std::unique_ptr<base::DictionaryValue> manifest) override;
  base::FilePath GetRelativeInstallDir() const override;
  void GetHash(std::vector<uint8_t>* hash) const override;
  std::string GetName() const override;
  std::vector<std::string> GetMimeTypes() const override;
  update_client::InstallerAttributes GetInstallerAttributes() const override;

  // Returns the path to the proto file for the given |install_dir|.
  static base::FilePath GetModuleListPath(const base::FilePath& install_dir);

  // The manager is not owned by this class, so the code creating it is
  // expected to ensure the manager lives as long as this class does. Typically
  // the manager provided will be a global singleton.
  ModuleListManager* manager_;

  DISALLOW_COPY_AND_ASSIGN(ThirdPartyModuleListComponentInstallerTraits);
};

void RegisterThirdPartyModuleListComponent(ComponentUpdateService* cus);

}  // namespace component_updater

#endif  // CHROME_BROWSER_COMPONENT_UPDATER_THIRD_PARTY_MODULE_LIST_COMPONENT_INSTALLER_WIN_H_
