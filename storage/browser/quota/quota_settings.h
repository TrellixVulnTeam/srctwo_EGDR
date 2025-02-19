// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STORAGE_BROWSER_QUOTA_QUOTA_SETTINGS_H_
#define STORAGE_BROWSER_QUOTA_QUOTA_SETTINGS_H_

#include <stdint.h>

#include "base/callback.h"
#include "base/files/file_path.h"
#include "base/optional.h"
#include "base/time/time.h"
#include "storage/browser/storage_browser_export.h"

namespace storage {

// Settings the storage lib embedder must provide to the QuotaManager.
struct QuotaSettings {
  QuotaSettings() = default;
  QuotaSettings(int64_t pool_size,
                int64_t per_host_quota,
                int64_t should_remain_available,
                int64_t must_remain_available)
      : pool_size(pool_size),
        per_host_quota(per_host_quota),
        session_only_per_host_quota(per_host_quota),
        should_remain_available(should_remain_available),
        must_remain_available(must_remain_available) {}

  // The target size in bytes of the shared pool of disk space the quota
  // system allows for use by websites using HTML5 storage apis, for
  // example an embedder may use 50% of the total volume size.
  int64_t pool_size = 0;

  // The amount in bytes of the pool an individual site may consume. The
  // value must be less than or equal to the pool_size.
  int64_t per_host_quota = 0;

  // The amount allotted to origins that are considered session only
  // according to the SpecialStoragePolicy provided by the embedder.
  int64_t session_only_per_host_quota = 0;

  // The amount of space that should remain available on the storage
  // volume. As the volume approaches this limit, the quota system gets
  // more aggressive about evicting data.
  int64_t should_remain_available = 0;

  // The amount of space that must remain available on the storage
  // volume. As the volume approaches this limit, the quota system gets
  // very aggressive about disallowing new data.
  int64_t must_remain_available = 0;

  // The quota system querries the embedder for the QuataSettings,
  // but will rate limit the frequency of the querries to no more than once
  // per refresh interval.
  base::TimeDelta refresh_interval = base::TimeDelta::Max();
};

// Function type used to return the settings in response to a
// GetQuotaSettingsFunc invocation. If the embedder cannot
// produce a settings values, base::nullopt can be returned.
using OptionalQuotaSettingsCallback =
    base::OnceCallback<void(base::Optional<QuotaSettings>)>;

// Function type used to query the embedder about the quota manager settings.
// This function is invoked on the UI thread.
using GetQuotaSettingsFunc =
    base::RepeatingCallback<void(OptionalQuotaSettingsCallback callback)>;

// Posts a background task to calculate and report quota settings to the
// |callback| function based on the size of the volume containing the storage
// partition and a guestimate of the size required for the OS. The refresh
// interval is 60 seconds to accomodate changes to the size of the volume.
// Except, in the case of incognito, the poolize and quota values are based
// on the amount of physical memory and the rerfresh interval is max'd out.
STORAGE_EXPORT
void GetNominalDynamicSettings(const base::FilePath& partition_path,
                               bool is_incognito,
                               OptionalQuotaSettingsCallback callback);

// Returns settings with a poolsize of zero and no per host quota.
inline QuotaSettings GetNoQuotaSettings() {
  return QuotaSettings();
}

// Returns settings that provide given |per_host_quota| and a total poolsize of
// five times that.
inline QuotaSettings GetHardCodedSettings(int64_t per_host_quota) {
  return QuotaSettings(per_host_quota * 5, per_host_quota,
                       per_host_quota, per_host_quota);
}

}  // namespace storage

#endif  // STORAGE_BROWSER_QUOTA_QUOTA_MANAGER_H_
