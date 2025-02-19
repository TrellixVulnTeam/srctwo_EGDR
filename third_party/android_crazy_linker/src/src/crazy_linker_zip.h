// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CRAZY_ZIP_H
#define CRAZY_ZIP_H

// Definitions related to supporting loading libraries from zip files.

namespace crazy {

// Find "filename" in the specified "zip_file" and return the offset
// in the file of the start of the data for the file. Return
// CRAZY_OFFSET_FAILED on error or if the file is compressed. This routine
// replaces code which used the minizip library, but is about 150 times faster,
// locating the offset in less than 0.5ms on a Nexus 4.
int FindStartOffsetOfFileInZipFile(const char* zip_file, const char* filename);

}

#endif  // CRAZY_ZIP_H
