// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/history/core/browser/history_constants.h"

#include "base/files/file_path.h"

namespace history {

// filenames
const base::FilePath::CharType kFaviconsFilename[] =
    FILE_PATH_LITERAL("Favicons");
const base::FilePath::CharType kHistoryFilename[] =
    FILE_PATH_LITERAL("History");
const base::FilePath::CharType kTopSitesFilename[] =
    FILE_PATH_LITERAL("Top Sites");

const int kMaxTopHosts = 50;

}  // namespace history
