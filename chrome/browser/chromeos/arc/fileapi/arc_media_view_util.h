// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Utilities for ARC Media View.

#ifndef CHROME_BROWSER_CHROMEOS_ARC_FILEAPI_ARC_MEDIA_VIEW_UTIL_H_
#define CHROME_BROWSER_CHROMEOS_ARC_FILEAPI_ARC_MEDIA_VIEW_UTIL_H_

#include <string>

#include "base/feature_list.h"

namespace arc {

// base::FeatureList feature for ARC media view.
extern const base::Feature kMediaViewFeature;

// Authority of MediaDocumentsProvider in Android.
extern const char kMediaDocumentsProviderAuthority[];

// Document IDs of file system roots in MediaDocumentsProvider.
extern const char kImagesRootDocumentId[];
extern const char kVideosRootDocumentId[];
extern const char kAudioRootDocumentId[];

// Returns an ID of a Media View volume.
std::string GetMediaViewVolumeId(const std::string& root_document_id);

}  // namespace arc

#endif  // CHROME_BROWSER_CHROMEOS_ARC_FILEAPI_ARC_MEDIA_VIEW_UTIL_H_
