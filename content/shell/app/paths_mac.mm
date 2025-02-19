// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/shell/app/paths_mac.h"

#include "base/base_paths.h"
#include "base/mac/bundle_locations.h"
#include "base/mac/foundation_util.h"
#include "base/path_service.h"
#include "content/public/common/content_paths.h"

namespace {

base::FilePath GetContentsPath() {
  // Start out with the path to the running executable.
  base::FilePath path;
  PathService::Get(base::FILE_EXE, &path);

  // Up to Contents.
  if (base::mac::IsBackgroundOnlyProcess()) {
    // The running executable is the helper. Go up five steps:
    // Contents/Frameworks/Helper.app/Contents/MacOS/Helper
    // ^ to here                                     ^ from here
    path = path.DirName().DirName().DirName().DirName().DirName();
  } else {
    // One step up to MacOS, another to Contents.
    path = path.DirName().DirName();
  }
  DCHECK_EQ("Contents", path.BaseName().value());

  return path;
}

base::FilePath GetFrameworksPath() {
  return GetContentsPath().Append("Frameworks");
}

}  // namespace

void OverrideFrameworkBundlePath() {
  base::FilePath helper_path =
      GetFrameworksPath().Append("Content Shell Framework.framework");

  base::mac::SetOverrideFrameworkBundlePath(helper_path);
}

void OverrideChildProcessPath() {
  base::FilePath helper_path =
      GetFrameworksPath().Append("Content Shell Helper.app")
                                            .Append("Contents")
                                            .Append("MacOS")
                                            .Append("Content Shell Helper");

  PathService::Override(content::CHILD_PROCESS_EXE, helper_path);
}

void OverrideSourceRootPath() {
  // The base implementation to get base::DIR_SOURCE_ROOT assumes the current
  // process path is the top level app path, not a nested one.
  //
  // Going up 5 levels is needed, since frameworks path looks something like
  // src/out/foo/Content Shell.app/Contents/Framework/
  PathService::Override(
      base::DIR_SOURCE_ROOT,
      GetFrameworksPath().DirName().DirName().DirName().DirName().DirName());
}

base::FilePath GetResourcesPakFilePath() {
  NSString* pak_path =
      [base::mac::FrameworkBundle() pathForResource:@"content_shell"
                                             ofType:@"pak"];

  return base::FilePath([pak_path fileSystemRepresentation]);
}

base::FilePath GetInfoPlistPath() {
  return GetContentsPath().Append("Info.plist");
}
