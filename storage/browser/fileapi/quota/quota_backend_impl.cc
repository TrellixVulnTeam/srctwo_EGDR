// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "storage/browser/fileapi/quota/quota_backend_impl.h"

#include <stdint.h>

#include <string>

#include "base/bind.h"
#include "base/callback.h"
#include "base/logging.h"
#include "base/numerics/safe_conversions.h"
#include "base/sequenced_task_runner.h"
#include "storage/browser/fileapi/file_system_usage_cache.h"
#include "storage/browser/quota/quota_client.h"
#include "storage/browser/quota/quota_manager_proxy.h"
#include "storage/common/fileapi/file_system_util.h"

namespace storage {

QuotaBackendImpl::QuotaBackendImpl(
    base::SequencedTaskRunner* file_task_runner,
    ObfuscatedFileUtil* obfuscated_file_util,
    FileSystemUsageCache* file_system_usage_cache,
    storage::QuotaManagerProxy* quota_manager_proxy)
    : file_task_runner_(file_task_runner),
      obfuscated_file_util_(obfuscated_file_util),
      file_system_usage_cache_(file_system_usage_cache),
      quota_manager_proxy_(quota_manager_proxy),
      weak_ptr_factory_(this) {
}

QuotaBackendImpl::~QuotaBackendImpl() {
}

void QuotaBackendImpl::ReserveQuota(const GURL& origin,
                                    FileSystemType type,
                                    int64_t delta,
                                    const ReserveQuotaCallback& callback) {
  DCHECK(file_task_runner_->RunsTasksInCurrentSequence());
  DCHECK(origin.is_valid());
  if (!delta) {
    callback.Run(base::File::FILE_OK, 0);
    return;
  }
  DCHECK(quota_manager_proxy_.get());
  quota_manager_proxy_->GetUsageAndQuota(
      file_task_runner_.get(),
      origin,
      FileSystemTypeToQuotaStorageType(type),
      base::Bind(&QuotaBackendImpl::DidGetUsageAndQuotaForReserveQuota,
                 weak_ptr_factory_.GetWeakPtr(),
                 QuotaReservationInfo(origin, type, delta),
                 callback));
}

void QuotaBackendImpl::ReleaseReservedQuota(const GURL& origin,
                                            FileSystemType type,
                                            int64_t size) {
  DCHECK(file_task_runner_->RunsTasksInCurrentSequence());
  DCHECK(origin.is_valid());
  DCHECK_LE(0, size);
  if (!size)
    return;
  ReserveQuotaInternal(QuotaReservationInfo(origin, type, -size));
}

void QuotaBackendImpl::CommitQuotaUsage(const GURL& origin,
                                        FileSystemType type,
                                        int64_t delta) {
  DCHECK(file_task_runner_->RunsTasksInCurrentSequence());
  DCHECK(origin.is_valid());
  if (!delta)
    return;
  ReserveQuotaInternal(QuotaReservationInfo(origin, type, delta));
  base::FilePath path;
  if (GetUsageCachePath(origin, type, &path) != base::File::FILE_OK)
    return;
  bool result = file_system_usage_cache_->AtomicUpdateUsageByDelta(path, delta);
  DCHECK(result);
}

void QuotaBackendImpl::IncrementDirtyCount(const GURL& origin,
                                           FileSystemType type) {
  DCHECK(file_task_runner_->RunsTasksInCurrentSequence());
  DCHECK(origin.is_valid());
  base::FilePath path;
  if (GetUsageCachePath(origin, type, &path) != base::File::FILE_OK)
    return;
  DCHECK(file_system_usage_cache_);
  file_system_usage_cache_->IncrementDirty(path);
}

void QuotaBackendImpl::DecrementDirtyCount(const GURL& origin,
                                           FileSystemType type) {
  DCHECK(file_task_runner_->RunsTasksInCurrentSequence());
  DCHECK(origin.is_valid());
  base::FilePath path;
  if (GetUsageCachePath(origin, type, &path) != base::File::FILE_OK)
    return;
  DCHECK(file_system_usage_cache_);
  file_system_usage_cache_->DecrementDirty(path);
}

void QuotaBackendImpl::DidGetUsageAndQuotaForReserveQuota(
    const QuotaReservationInfo& info,
    const ReserveQuotaCallback& callback,
    storage::QuotaStatusCode status,
    int64_t usage,
    int64_t quota) {
  DCHECK(file_task_runner_->RunsTasksInCurrentSequence());
  DCHECK(info.origin.is_valid());
  DCHECK_LE(0, usage);
  DCHECK_LE(0, quota);
  if (status != storage::kQuotaStatusOk) {
    callback.Run(base::File::FILE_ERROR_FAILED, 0);
    return;
  }

  QuotaReservationInfo normalized_info = info;
  if (info.delta > 0) {
    int64_t new_usage = base::saturated_cast<int64_t>(
        usage + static_cast<uint64_t>(info.delta));
    if (quota < new_usage)
      new_usage = quota;
    normalized_info.delta =
        std::max(static_cast<int64_t>(0), new_usage - usage);
  }

  ReserveQuotaInternal(normalized_info);
  if (callback.Run(base::File::FILE_OK, normalized_info.delta))
    return;
  // The requester could not accept the reserved quota. Revert it.
  ReserveQuotaInternal(
      QuotaReservationInfo(normalized_info.origin,
                           normalized_info.type,
                           -normalized_info.delta));
}

void QuotaBackendImpl::ReserveQuotaInternal(const QuotaReservationInfo& info) {
  DCHECK(file_task_runner_->RunsTasksInCurrentSequence());
  DCHECK(info.origin.is_valid());
  DCHECK(quota_manager_proxy_.get());
  quota_manager_proxy_->NotifyStorageModified(
      storage::QuotaClient::kFileSystem,
      info.origin,
      FileSystemTypeToQuotaStorageType(info.type),
      info.delta);
}

base::File::Error QuotaBackendImpl::GetUsageCachePath(
    const GURL& origin,
    FileSystemType type,
    base::FilePath* usage_file_path) {
  DCHECK(file_task_runner_->RunsTasksInCurrentSequence());
  DCHECK(origin.is_valid());
  DCHECK(usage_file_path);
  base::File::Error error = base::File::FILE_OK;
  *usage_file_path =
      SandboxFileSystemBackendDelegate::GetUsageCachePathForOriginAndType(
          obfuscated_file_util_, origin, type, &error);
  return error;
}

QuotaBackendImpl::QuotaReservationInfo::QuotaReservationInfo(
    const GURL& origin,
    FileSystemType type,
    int64_t delta)
    : origin(origin), type(type), delta(delta) {}

QuotaBackendImpl::QuotaReservationInfo::~QuotaReservationInfo() {
}

}  // namespace storage
