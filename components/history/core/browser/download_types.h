// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_HISTORY_CORE_BROWSER_DOWNLOAD_TYPES_H_
#define COMPONENTS_HISTORY_CORE_BROWSER_DOWNLOAD_TYPES_H_

#include <stdint.h>
#include <iosfwd>

#include "base/compiler_specific.h"

namespace history {

// DownloadState represents the state of a DownloadRow saved into the
// DownloadDatabase. The values must not be changed as they are saved
// to disk in the database.
enum class DownloadState;

// Utility functions to convert between int and DownloadState for
// serialization to the download database.
DownloadState IntToDownloadState(int state);
int DownloadStateToInt(DownloadState state);

// operator<< is defined to allow DownloadState to work with DCHECK/EXPECT.
std::ostream& operator<<(std::ostream& stream, DownloadState state);

// DownloadDangerType represents the danger of a DownloadRow into the
// DownloadDatabase. The values must not be changed as they are saved
// to disk in the database.
enum class DownloadDangerType;

// Utility functions to convert between int and DownloadDangerType for
// serialization to the download database.
DownloadDangerType IntToDownloadDangerType(int danger_type);
int DownloadDangerTypeToInt(DownloadDangerType danger_type);

// operator<< is defined to allow DownloadDangerType to work with DCHECK/EXPECT.
std::ostream& operator<<(std::ostream& stream, DownloadDangerType danger_type);

// DownloadInterruptReason represents the reason a download was interrupted
// of a DownloadRow into the DownloadDatabase. The values must not be changed
// as they are saved to disk in the database. They have no meaning for the
// history component.
typedef int32_t DownloadInterruptReason;

// Utility functions to convert between int and DownloadInterruptReason for
// serialization to the download database.
DownloadInterruptReason IntToDownloadInterruptReason(int interrupt_reason);
int DownloadInterruptReasonToInt(DownloadInterruptReason interrupt_reason);

// DownloadId represents the id of a DownloadRow into the DownloadDatabase.
// The value is controlled by the embedder except for the reserved id
// kInvalidDownloadId.
typedef uint32_t DownloadId;

// Utility functions to convert between int and DownloadId for
// serialization to the download database.
bool ConvertIntToDownloadId(int64_t id, DownloadId* out) WARN_UNUSED_RESULT;
int64_t DownloadIdToInt(DownloadId id);

}  // namespace history

#endif  // COMPONENTS_HISTORY_CORE_BROWSER_DOWNLOAD_TYPES_H_
