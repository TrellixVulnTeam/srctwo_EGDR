// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_REPORTING_REPORTING_BROWSING_DATA_REMOVER_H_
#define NET_REPORTING_REPORTING_BROWSING_DATA_REMOVER_H_

#include <memory>

#include "base/callback.h"
#include "base/macros.h"
#include "net/base/net_export.h"
#include "url/gurl.h"

namespace net {

class ReportingCache;

// Clears browsing data (reports and clients) from the Reporting system.
class NET_EXPORT ReportingBrowsingDataRemover {
 public:
  enum DataType {
    DATA_TYPE_REPORTS = 0x1,
    DATA_TYPE_CLIENTS = 0x2,
  };

  // Removes browsing data from the Reporting system. |data_type_mask| specifies
  // which types of data to remove: reports queued by browser features and/or
  // clients (endpoints configured by origins). |origin_filter|, if not null,
  // specifies which origins' data to remove.
  //
  // Note: Currently this does not clear the endpoint backoff data in
  // ReportingEndpointManager because that's not persisted to disk. If it's ever
  // persisted, it will need to be cleared as well.
  static void RemoveBrowsingData(
      ReportingCache* cache,
      int data_type_mask,
      base::Callback<bool(const GURL&)> origin_filter);

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(ReportingBrowsingDataRemover);
};

}  // namespace net

#endif  // NET_REPORTING_REPORTING_BROWSING_DATA_REMOVER_H_
