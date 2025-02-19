// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ios/web/web_state/session_certificate_policy_cache_storage_builder.h"

#import <Foundation/Foundation.h>

#include "base/memory/ptr_util.h"
#import "ios/web/public/crw_session_certificate_policy_cache_storage.h"
#import "ios/web/web_state/session_certificate_policy_cache_impl.h"

#if !defined(__has_feature) || !__has_feature(objc_arc)
#error "This file requires ARC support."
#endif

namespace web {

CRWSessionCertificatePolicyCacheStorage*
SessionCertificatePolicyCacheStorageBuilder::BuildStorage(
    SessionCertificatePolicyCacheImpl* cache) const {
  CRWSessionCertificatePolicyCacheStorage* storage =
      [[CRWSessionCertificatePolicyCacheStorage alloc] init];
  storage.certificateStorages = [NSSet setWithSet:cache->GetAllowedCerts()];
  return storage;
}

std::unique_ptr<SessionCertificatePolicyCacheImpl>
SessionCertificatePolicyCacheStorageBuilder::BuildSessionCertificatePolicyCache(
    CRWSessionCertificatePolicyCacheStorage* cache_storage) const {
  std::unique_ptr<SessionCertificatePolicyCacheImpl> cache =
      base::MakeUnique<SessionCertificatePolicyCacheImpl>();
  cache->SetAllowedCerts(cache_storage.certificateStorages);
  return cache;
}

}  // namespace web
