// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_BROWSING_DATA_CORE_HISTORY_NOTICE_UTILS_H_
#define COMPONENTS_BROWSING_DATA_CORE_HISTORY_NOTICE_UTILS_H_

#include <string>

#include "base/callback_forward.h"

namespace history {
class WebHistoryService;
}

namespace syncer {
class SyncService;
}

namespace version_info {
enum class Channel;
}

namespace browsing_data {

namespace testing {

// TODO(crbug.com/595332): A boolean flag indicating that
// ShouldShowNoticeAboutOtherFormsOfBrowsingHistory() should skip the query
// for other forms of browsing history and just assume some such forms were
// found. Used only for testing. The default is false.
extern bool g_override_other_forms_of_browsing_history_query;

}  // testing

// Whether the Clear Browsing Data UI should show a notice about the existence
// of other forms of browsing history stored in user's account. The response
// is returned in a |callback|.
void ShouldShowNoticeAboutOtherFormsOfBrowsingHistory(
    const syncer::SyncService* sync_service,
    history::WebHistoryService* history_service,
    base::Callback<void(bool)> callback);

// Whether the Clear Browsing Data UI should popup a dialog with information
// about the existence of other forms of browsing history stored in user's
// account when the user deletes their browsing history for the first time.
// The response is returned in a |callback|. The |channel| parameter
// must be provided for successful communication with the Sync server, but
// the result does not depend on it.
void ShouldPopupDialogAboutOtherFormsOfBrowsingHistory(
    const syncer::SyncService* sync_service,
    history::WebHistoryService* history_service,
    version_info::Channel channel,
    base::Callback<void(bool)> callback);

}  // namespace browsing_data

#endif  // COMPONENTS_BROWSING_DATA_CORE_HISTORY_NOTICE_UTILS_H_
