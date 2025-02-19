// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_LOGIN_USERS_AVATAR_USER_IMAGE_MANAGER_H_
#define CHROME_BROWSER_CHROMEOS_LOGIN_USERS_AVATAR_USER_IMAGE_MANAGER_H_

#include <string>

#include "components/user_manager/user.h"

class PrefRegistrySimple;

namespace base {
class FilePath;
}

namespace gfx {
class ImageSkia;
}

namespace user_manager {
class UserImage;
}

namespace chromeos {

class UserImageSyncObserver;

// Base class that provides a mechanism for updating user images.
// There is an instance of this class for each user in the system.
class UserImageManager {
 public:
  // Registers user image manager preferences.
  static void RegisterPrefs(PrefRegistrySimple* registry);

  explicit UserImageManager(const std::string& user_id);
  virtual ~UserImageManager();

  // Loads user image data from Local State.
  virtual void LoadUserImage() = 0;

  // Indicates that a user has just logged in.
  virtual void UserLoggedIn(bool user_is_new, bool user_is_local) = 0;

  // Indicates that a user profile was created.
  virtual void UserProfileCreated() = 0;

  // Sets user image to the default image with index |image_index|, sends
  // LOGIN_USER_IMAGE_CHANGED notification and updates Local State.
  virtual void SaveUserDefaultImageIndex(int image_index) = 0;

  // Saves image to file, sends LOGIN_USER_IMAGE_CHANGED notification and
  // updates Local State.
  virtual void SaveUserImage(
      std::unique_ptr<user_manager::UserImage> user_image) = 0;

  // Tries to load user image from disk; if successful, sets it for the user,
  // sends LOGIN_USER_IMAGE_CHANGED notification and updates Local State.
  virtual void SaveUserImageFromFile(const base::FilePath& path) = 0;

  // Sets profile image as user image for the user, sends
  // LOGIN_USER_IMAGE_CHANGED notification and updates Local State. If
  // the user is not logged-in or the last |DownloadProfileImage| call
  // has failed, a default grey avatar will be used until the user logs
  // in and profile image is downloaded successfully.
  virtual void SaveUserImageFromProfileImage() = 0;

  // Deletes user image and the corresponding image file.
  virtual void DeleteUserImage() = 0;

  // Starts downloading the profile image for the user.  If user's image
  // index is |USER_IMAGE_PROFILE|, newly downloaded image is immediately
  // set as user's current picture.  |reason| is an arbitrary string
  // (used to report UMA histograms with download times).
  virtual void DownloadProfileImage(const std::string& reason) = 0;

  // Returns the result of the last successful profile image download, if any.
  // Otherwise, returns an empty bitmap.
  virtual const gfx::ImageSkia& DownloadedProfileImage() const = 0;

  // Returns sync observer attached to the user. Returns NULL if current
  // user can't sync images or user is not logged in.
  virtual UserImageSyncObserver* GetSyncObserver() const = 0;

  // Unregisters preference observers before browser process shutdown.
  // Also cancels any profile image download in progress.
  virtual void Shutdown() = 0;

  // Invoked when an external data reference is set for the user.
  virtual void OnExternalDataSet(const std::string& policy) = 0;

  // Invoked when the external data reference is cleared for the user.
  virtual void OnExternalDataCleared(const std::string& policy) = 0;

  // Invoked when the external data referenced for the user has been
  // fetched.  Failed fetches are retried and the method is called only
  // when a fetch eventually succeeds. If a fetch fails permanently
  // (e.g. because the external data reference specifies an invalid URL),
  // the method is not called at all.
  virtual void OnExternalDataFetched(const std::string& policy,
                                     std::unique_ptr<std::string> data) = 0;

 protected:
  const std::string& user_id() const { return user_id_; }

  // ID of user which images are managed by current instance of
  // UserImageManager.
  const std::string user_id_;
};

}  // namespace chromeos

#endif  // CHROME_BROWSER_CHROMEOS_LOGIN_USERS_AVATAR_USER_IMAGE_MANAGER_H_
