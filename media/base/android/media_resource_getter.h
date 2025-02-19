// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_BASE_ANDROID_MEDIA_RESOURCE_GETTER_H_
#define MEDIA_BASE_ANDROID_MEDIA_RESOURCE_GETTER_H_

#include <stdint.h>

#include <string>

#include "base/callback.h"
#include "base/strings/string16.h"
#include "base/time/time.h"
#include "media/base/media_export.h"
#include "url/gurl.h"

namespace media {

// Class for asynchronously retrieving resources for a media URL. All callbacks
// are executed on the caller's thread.
class MEDIA_EXPORT MediaResourceGetter {
 public:
  // Callback to get the cookies. Args: cookies string.
  typedef base::OnceCallback<void(const std::string&)> GetCookieCB;

  // Callback to get the platform path. Args: platform path.
  typedef base::OnceCallback<void(const std::string&)> GetPlatformPathCB;

  // Callback to get the auth credentials. Args: username and password.
  typedef base::OnceCallback<void(const base::string16&, const base::string16&)>
      GetAuthCredentialsCB;

  // Callback to get the media metadata. Args: duration, width, height, and
  // whether the information is retrieved successfully.
  typedef base::OnceCallback<void(base::TimeDelta, int, int, bool)>
      ExtractMediaMetadataCB;
  virtual ~MediaResourceGetter();

  // Method for getting the auth credentials for a URL.
  virtual void GetAuthCredentials(const GURL& url,
                                  GetAuthCredentialsCB callback) = 0;

  // Method for getting the cookies for a given URL.
  virtual void GetCookies(const GURL& url,
                          const GURL& site_for_cookies,
                          GetCookieCB callback) = 0;

  // Method for getting the platform path from a file system URL.
  virtual void GetPlatformPathFromURL(const GURL& url,
                                      GetPlatformPathCB callback) = 0;

  // Extracts the metadata from a media URL. Once completed, the provided
  // callback function will be run.
  virtual void ExtractMediaMetadata(const std::string& url,
                                    const std::string& cookies,
                                    const std::string& user_agent,
                                    ExtractMediaMetadataCB callback) = 0;

  // Extracts the metadata from a file descriptor. Once completed, the
  // provided callback function will be run.
  virtual void ExtractMediaMetadata(const int fd,
                                    const int64_t offset,
                                    const int64_t size,
                                    ExtractMediaMetadataCB callback) = 0;
};

}  // namespace media

#endif  // MEDIA_BASE_ANDROID_MEDIA_RESOURCE_GETTER_H_
