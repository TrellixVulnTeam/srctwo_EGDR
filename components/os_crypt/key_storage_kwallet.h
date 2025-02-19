// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_OS_CRYPT_KEY_STORAGE_KWALLET_H_
#define COMPONENTS_OS_CRYPT_KEY_STORAGE_KWALLET_H_

#include <memory>
#include <string>

#include "base/macros.h"
#include "components/os_crypt/key_storage_linux.h"
#include "components/os_crypt/kwallet_dbus.h"

class KeyStorageKWallet : public KeyStorageLinux {
 public:
  KeyStorageKWallet(base::nix::DesktopEnvironment desktop_env,
                    std::string app_name);
  ~KeyStorageKWallet() override;

  // KeyStorageLinux
  std::string GetKey() override;

  // Initialize using an optional KWalletDBus mock.
  // A DBus session will not be created if a mock is provided.
  bool InitWithKWalletDBus(
      std::unique_ptr<KWalletDBus> optional_kwallet_dbus_ptr);

 protected:
  // KeyStorageLinux
  bool Init() override;

 private:
  enum class InitResult {
    SUCCESS,
    TEMPORARY_FAIL,
    PERMANENT_FAIL,
  };

  // Check whether KWallet is enabled and set |wallet_name_|
  InitResult InitWallet();

  // Create Chrome's folder in the wallet, if it doesn't exist.
  bool InitFolder();

  const base::nix::DesktopEnvironment desktop_env_;
  int32_t handle_;
  std::string wallet_name_;
  const std::string app_name_;
  std::unique_ptr<KWalletDBus> kwallet_dbus_;

  DISALLOW_COPY_AND_ASSIGN(KeyStorageKWallet);
};

#endif  // COMPONENTS_OS_CRYPT_KEY_STORAGE_KWALLET_H_
