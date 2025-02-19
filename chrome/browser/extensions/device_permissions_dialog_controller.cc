// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/extensions/device_permissions_dialog_controller.h"

#include "chrome/grit/generated_resources.h"
#include "extensions/strings/grit/extensions_strings.h"
#include "ui/base/l10n/l10n_util.h"

DevicePermissionsDialogController::DevicePermissionsDialogController(
    content::RenderFrameHost* owner,
    scoped_refptr<extensions::DevicePermissionsPrompt::Prompt> prompt)
    : ChooserController(
          owner,
          prompt->multiple() ? IDS_DEVICE_PERMISSIONS_PROMPT_MULTIPLE_SELECTION
                             : IDS_DEVICE_PERMISSIONS_PROMPT_SINGLE_SELECTION,
          prompt->multiple() ? IDS_DEVICE_PERMISSIONS_PROMPT_MULTIPLE_SELECTION
                             : IDS_DEVICE_PERMISSIONS_PROMPT_SINGLE_SELECTION),
      prompt_(prompt) {
  prompt_->SetObserver(this);
}

DevicePermissionsDialogController::~DevicePermissionsDialogController() {
  prompt_->SetObserver(nullptr);
}

bool DevicePermissionsDialogController::ShouldShowFootnoteView() const {
  return false;
}

bool DevicePermissionsDialogController::AllowMultipleSelection() const {
  return prompt_->multiple();
}

base::string16 DevicePermissionsDialogController::GetNoOptionsText() const {
  return l10n_util::GetStringUTF16(IDS_DEVICE_CHOOSER_NO_DEVICES_FOUND_PROMPT);
}

base::string16 DevicePermissionsDialogController::GetOkButtonLabel() const {
  return l10n_util::GetStringUTF16(IDS_DEVICE_PERMISSIONS_DIALOG_SELECT);
}

size_t DevicePermissionsDialogController::NumOptions() const {
  return prompt_->GetDeviceCount();
}

base::string16 DevicePermissionsDialogController::GetOption(
    size_t index) const {
  base::string16 device_name = prompt_->GetDeviceName(index);
  const auto& it = device_name_map_.find(device_name);
  DCHECK(it != device_name_map_.end());
  return it->second == 1
             ? device_name
             : l10n_util::GetStringFUTF16(
                   IDS_DEVICE_CHOOSER_DEVICE_NAME_WITH_ID, device_name,
                   prompt_->GetDeviceSerialNumber(index));
}

void DevicePermissionsDialogController::RefreshOptions() {}

base::string16 DevicePermissionsDialogController::GetStatus() const {
  return base::string16();
}

void DevicePermissionsDialogController::Select(
    const std::vector<size_t>& indices) {
  for (size_t index : indices)
    prompt_->GrantDevicePermission(index);
  prompt_->Dismissed();
}

void DevicePermissionsDialogController::Cancel() {
  prompt_->Dismissed();
}

void DevicePermissionsDialogController::Close() {
  prompt_->Dismissed();
}

void DevicePermissionsDialogController::OpenHelpCenterUrl() const {}

void DevicePermissionsDialogController::OnDeviceAdded(
    size_t index,
    const base::string16& device_name) {
  if (view()) {
    ++device_name_map_[device_name];
    view()->OnOptionAdded(index);
  }
}

void DevicePermissionsDialogController::OnDeviceRemoved(
    size_t index,
    const base::string16& device_name) {
  if (view()) {
    DCHECK_GT(device_name_map_[device_name], 0);
    if (--device_name_map_[device_name] == 0)
      device_name_map_.erase(device_name);
    view()->OnOptionRemoved(index);
  }
}
