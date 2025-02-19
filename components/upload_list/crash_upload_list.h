// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_UPLOAD_LIST_CRASH_UPLOAD_LIST_H_
#define COMPONENTS_UPLOAD_LIST_CRASH_UPLOAD_LIST_H_

// TODO(rsesek): Remove this class. Clients only use it for the constant
// it defines.
class CrashUploadList {
 public:
  // Should match kReporterLogFilename in
  // breakpad/src/client/apple/Framework/BreakpadDefines.h.
  static const char kReporterLogFilename[];

 private:
  CrashUploadList() {}
};

#endif  // COMPONENTS_UPLOAD_LIST_CRASH_UPLOAD_LIST_H_
