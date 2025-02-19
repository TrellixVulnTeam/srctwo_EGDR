// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_BROWSING_DATA_SITE_DATA_COUNTING_HELPER_H_
#define CHROME_BROWSER_BROWSING_DATA_SITE_DATA_COUNTING_HELPER_H_

#include <set>
#include "components/content_settings/core/common/content_settings_types.h"
#include "net/cookies/canonical_cookie.h"
#include "net/ssl/channel_id_store.h"
#include "storage/common/quota/quota_types.h"

class Profile;
class BrowsingDataFlashLSOHelper;
class HostContentSettingsMap;

namespace net {
class URLRequestContextGetter;
}

namespace content {
struct LocalStorageUsageInfo;
struct SessionStorageUsageInfo;
}

namespace storage {
class SpecialStoragePolicy;
}

// Helper class that counts the number of unique origins, that are affected by
// deleting "cookies and site data" in the CBD dialog.
class SiteDataCountingHelper {
 public:
  explicit SiteDataCountingHelper(
      Profile* profile,
      base::Time begin,
      base::Callback<void(int)> completion_callback);
  ~SiteDataCountingHelper();

  void CountAndDestroySelfWhenFinished();

 private:
  void GetOriginsFromHostContentSettignsMap(HostContentSettingsMap* hcsm,
                                            ContentSettingsType type);
  void GetCookiesOnIOThread(
      const scoped_refptr<net::URLRequestContextGetter>& rq_context);
  void GetCookiesCallback(const net::CookieList& cookies);
  void GetSessionStorageUsageInfoCallback(
      const scoped_refptr<storage::SpecialStoragePolicy>&
          special_storage_policy,
      const std::vector<content::SessionStorageUsageInfo>& infos);
  void GetLocalStorageUsageInfoCallback(
      const scoped_refptr<storage::SpecialStoragePolicy>&
          special_storage_policy,
      const std::vector<content::LocalStorageUsageInfo>& infos);
  void GetQuotaOriginsCallback(const std::set<GURL>& origin_set,
                               storage::StorageType type);
  void SitesWithFlashDataCallback(const std::vector<std::string>& sites);
  void GetChannelIDsOnIOThread(
      const scoped_refptr<net::URLRequestContextGetter>& rq_context);
  void GetChannelIDsCallback(
      const net::ChannelIDStore::ChannelIDList& channel_ids);

  void Done(const std::vector<GURL>& origins);

  Profile* profile_;
  base::Time begin_;
  base::Callback<void(int)> completion_callback_;
  int tasks_;
  std::set<GURL> unique_origins_;
  scoped_refptr<BrowsingDataFlashLSOHelper> flash_lso_helper_;
};

#endif  // CHROME_BROWSER_BROWSING_DATA_SITE_DATA_COUNTING_HELPER_H_
