// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STORAGE_BROWSER_FILEAPI_QUOTA_QUOTA_BACKEND_IMPL_H_
#define STORAGE_BROWSER_FILEAPI_QUOTA_QUOTA_BACKEND_IMPL_H_

#include <stdint.h>

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "storage/browser/fileapi/quota/quota_reservation_manager.h"
#include "storage/browser/fileapi/sandbox_file_system_backend_delegate.h"
#include "storage/browser/storage_browser_export.h"
#include "storage/common/quota/quota_status_code.h"

namespace base {
class SequencedTaskRunner;
}

namespace content {
class QuotaBackendImplTest;
}

namespace storage {
class QuotaManagerProxy;
}

namespace storage {

class FileSystemUsageCache;
class ObfuscatedFileUtil;

// An instance of this class is owned by QuotaReservationManager.
class STORAGE_EXPORT QuotaBackendImpl
    : public QuotaReservationManager::QuotaBackend {
 public:
  typedef QuotaReservationManager::ReserveQuotaCallback
      ReserveQuotaCallback;

  QuotaBackendImpl(base::SequencedTaskRunner* file_task_runner,
                   ObfuscatedFileUtil* obfuscated_file_util,
                   FileSystemUsageCache* file_system_usage_cache,
                   storage::QuotaManagerProxy* quota_manager_proxy);
  ~QuotaBackendImpl() override;

  // QuotaReservationManager::QuotaBackend overrides.
  void ReserveQuota(const GURL& origin,
                    FileSystemType type,
                    int64_t delta,
                    const ReserveQuotaCallback& callback) override;
  void ReleaseReservedQuota(const GURL& origin,
                            FileSystemType type,
                            int64_t size) override;
  void CommitQuotaUsage(const GURL& origin,
                        FileSystemType type,
                        int64_t delta) override;
  void IncrementDirtyCount(const GURL& origin, FileSystemType type) override;
  void DecrementDirtyCount(const GURL& origin, FileSystemType type) override;

 private:
  friend class content::QuotaBackendImplTest;

  struct QuotaReservationInfo {
    QuotaReservationInfo(const GURL& origin,
                         FileSystemType type,
                         int64_t delta);
    ~QuotaReservationInfo();

    GURL origin;
    FileSystemType type;
    int64_t delta;
  };

  void DidGetUsageAndQuotaForReserveQuota(const QuotaReservationInfo& info,
                                          const ReserveQuotaCallback& callback,
                                          storage::QuotaStatusCode status,
                                          int64_t usage,
                                          int64_t quota);

  void ReserveQuotaInternal(
      const QuotaReservationInfo& info);
  base::File::Error GetUsageCachePath(
      const GURL& origin,
      FileSystemType type,
      base::FilePath* usage_file_path);

  scoped_refptr<base::SequencedTaskRunner> file_task_runner_;

  // Owned by SandboxFileSystemBackendDelegate.
  ObfuscatedFileUtil* obfuscated_file_util_;
  FileSystemUsageCache* file_system_usage_cache_;

  scoped_refptr<storage::QuotaManagerProxy> quota_manager_proxy_;

  base::WeakPtrFactory<QuotaBackendImpl> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(QuotaBackendImpl);
};

}  // namespace storage

#endif  // STORAGE_BROWSER_FILEAPI_QUOTA_QUOTA_BACKEND_IMPL_H_
