// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/media_galleries/media_galleries_test_util.h"

#include <stddef.h>

#include <utility>

#include "base/base_paths.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/memory/ptr_util.h"
#include "base/path_service.h"
#include "base/strings/string_number_conversions.h"
#include "base/threading/thread_restrictions.h"
#include "base/values.h"
#include "build/build_config.h"
#include "chrome/browser/extensions/extension_service.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/common/chrome_paths.h"
#include "components/crx_file/id_util.h"
#include "extensions/browser/extension_prefs.h"
#include "extensions/browser/extension_system.h"
#include "extensions/common/manifest_constants.h"
#include "testing/gtest/include/gtest/gtest.h"

#if defined(OS_MACOSX)
#include "base/mac/foundation_util.h"
#include "base/strings/sys_string_conversions.h"
#include "chrome/browser/media_galleries/fileapi/iapps_finder_impl.h"
#include "components/policy/core/common/preferences_mock_mac.h"
#endif  // OS_MACOSX

#if defined(OS_WIN)
#include <windows.h>
#include "base/test/test_reg_util_win.h"
#include "base/win/registry.h"
#endif  // OS_WIN

scoped_refptr<extensions::Extension> AddMediaGalleriesApp(
    const std::string& name,
    const std::vector<std::string>& media_galleries_permissions,
    Profile* profile) {
  auto manifest = base::MakeUnique<base::DictionaryValue>();
  manifest->SetString(extensions::manifest_keys::kName, name);
  manifest->SetString(extensions::manifest_keys::kVersion, "0.1");
  manifest->SetInteger(extensions::manifest_keys::kManifestVersion, 2);
  auto background_script_list = base::MakeUnique<base::ListValue>();
  background_script_list->AppendString("background.js");
  manifest->Set(extensions::manifest_keys::kPlatformAppBackgroundScripts,
                std::move(background_script_list));

  auto permission_detail_list = base::MakeUnique<base::ListValue>();
  for (size_t i = 0; i < media_galleries_permissions.size(); i++)
    permission_detail_list->AppendString(media_galleries_permissions[i]);
  auto media_galleries_permission = base::MakeUnique<base::DictionaryValue>();
  media_galleries_permission->Set("mediaGalleries",
                                  std::move(permission_detail_list));
  auto permission_list = base::MakeUnique<base::ListValue>();
  permission_list->Append(std::move(media_galleries_permission));
  manifest->Set(extensions::manifest_keys::kPermissions,
                std::move(permission_list));

  extensions::ExtensionPrefs* extension_prefs =
      extensions::ExtensionPrefs::Get(profile);
  base::FilePath path = extension_prefs->install_directory().AppendASCII(name);
  std::string errors;
  scoped_refptr<extensions::Extension> extension =
      extensions::Extension::Create(path, extensions::Manifest::INTERNAL,
                                    *manifest.get(),
                                    extensions::Extension::NO_FLAGS, &errors);
  EXPECT_TRUE(extension.get() != NULL) << errors;
  EXPECT_TRUE(crx_file::id_util::IdIsValid(extension->id()));
  if (!extension.get() || !crx_file::id_util::IdIsValid(extension->id()))
    return NULL;

  extension_prefs->OnExtensionInstalled(
      extension.get(),
      extensions::Extension::ENABLED,
      syncer::StringOrdinal::CreateInitialOrdinal(),
      std::string());
  ExtensionService* extension_service =
      extensions::ExtensionSystem::Get(profile)->extension_service();
  extension_service->AddExtension(extension.get());
  extension_service->EnableExtension(extension->id());

  return extension;
}

EnsureMediaDirectoriesExists::EnsureMediaDirectoriesExists()
    : num_galleries_(0), times_overrides_changed_(0) {
  Init();
}

EnsureMediaDirectoriesExists::~EnsureMediaDirectoriesExists() {
#if defined(OS_MACOSX)
  iapps::SetMacPreferencesForTesting(NULL);
#endif  // OS_MACOSX
  base::ThreadRestrictions::ScopedAllowIO allow_io;
  ignore_result(fake_dir_.Delete());
}

void EnsureMediaDirectoriesExists::ChangeMediaPathOverrides() {
  // Each pointer must be reset an extra time so as to destroy the existing
  // override prior to creating a new one. This is because the PathService,
  // which supports these overrides, only allows one override to exist per path
  // in its internal bookkeeping; attempting to add a second override invokes
  // a CHECK crash.
  music_override_.reset();
  std::string music_path_string("music");
  music_path_string.append(base::IntToString(times_overrides_changed_));
  music_override_.reset(new base::ScopedPathOverride(
      chrome::DIR_USER_MUSIC,
      fake_dir_.GetPath().AppendASCII(music_path_string)));

  pictures_override_.reset();
  std::string pictures_path_string("pictures");
  pictures_path_string.append(base::IntToString(times_overrides_changed_));
  pictures_override_.reset(new base::ScopedPathOverride(
      chrome::DIR_USER_PICTURES,
      fake_dir_.GetPath().AppendASCII(pictures_path_string)));

  video_override_.reset();
  std::string videos_path_string("videos");
  videos_path_string.append(base::IntToString(times_overrides_changed_));
  video_override_.reset(new base::ScopedPathOverride(
      chrome::DIR_USER_VIDEOS,
      fake_dir_.GetPath().AppendASCII(videos_path_string)));

  times_overrides_changed_++;

  num_galleries_ = 3;
}

base::FilePath EnsureMediaDirectoriesExists::GetFakeAppDataPath() const {
  base::ThreadRestrictions::ScopedAllowIO allow_io;
  DCHECK(fake_dir_.IsValid());
  return fake_dir_.GetPath().AppendASCII("appdata");
}

#if defined(OS_WIN)
base::FilePath EnsureMediaDirectoriesExists::GetFakeLocalAppDataPath() const {
  DCHECK(fake_dir_.IsValid());
  return fake_dir_.GetPath().AppendASCII("localappdata");
}
#endif  // OS_WIN

#if defined(OS_MACOSX)
base::FilePath EnsureMediaDirectoriesExists::GetFakeITunesRootPath() const {
  DCHECK(fake_dir_.IsValid());
  return fake_dir_.GetPath().AppendASCII("itunes");
}
#endif  // OS_MACOSX

void EnsureMediaDirectoriesExists::Init() {
#if defined(OS_CHROMEOS) || defined(OS_ANDROID)
  return;
#else

  ASSERT_TRUE(fake_dir_.CreateUniqueTempDir());

#if defined(OS_WIN) || defined(OS_MACOSX)
  // This is to control whether or not tests think iTunes (on Windows) is
  // installed.
  app_data_override_.reset(new base::ScopedPathOverride(
      base::DIR_APP_DATA, GetFakeAppDataPath()));
#endif  // OS_WIN || OS_MACOSX

#if defined(OS_MACOSX)
  mac_preferences_.reset(new MockPreferences);

  // iTunes override.
  base::FilePath itunes_xml =
      GetFakeITunesRootPath().AppendASCII("iTunes Library.xml");
  mac_preferences_->AddTestItem(
      base::mac::NSToCFCast(iapps::kITunesRecentDatabasePathsKey),
      base::mac::NSToCFCast(iapps::NSArrayFromFilePath(itunes_xml)),
      false);

  iapps::SetMacPreferencesForTesting(mac_preferences_.get());
#endif  // OS_MACOSX

  ChangeMediaPathOverrides();
#endif  // OS_CHROMEOS || OS_ANDROID
}

base::FilePath MakeMediaGalleriesTestingPath(const std::string& dir) {
#if defined(OS_WIN)
  return base::FilePath(FILE_PATH_LITERAL("C:\\")).AppendASCII(dir);
#elif defined(OS_POSIX)
  return base::FilePath(FILE_PATH_LITERAL("/")).Append(dir);
#else
  NOTREACHED();
#endif
}
