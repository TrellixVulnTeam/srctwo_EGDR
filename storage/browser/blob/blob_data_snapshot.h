// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STORAGE_BROWSER_BLOB_BLOB_DATA_SNAPSHOT_H_
#define STORAGE_BROWSER_BLOB_BLOB_DATA_SNAPSHOT_H_

#include <stddef.h>

#include <string>
#include <vector>

#include "base/memory/ref_counted.h"
#include "base/supports_user_data.h"
#include "storage/browser/blob/blob_data_item.h"
#include "storage/browser/storage_browser_export.h"

namespace storage {
class BlobDataBuilder;

// Snapshot of a Blob. This snapshot holds a refcount of the current
// blob item resources so the backing storage for these items will stick
// around for the lifetime of this object. (The data represented by a blob is
// immutable, but the backing store can change). Keeping this object alive
// guarantees that the resources stay alive, but it does not guarentee that
// the blob stays alive.  Use the BlobDataHandle to keep a blob alive.
// This class must be deleted on the IO thread.
class STORAGE_EXPORT BlobDataSnapshot : public base::SupportsUserData::Data {
 public:
  BlobDataSnapshot(const BlobDataSnapshot& other);
  ~BlobDataSnapshot() override;

  const std::vector<scoped_refptr<BlobDataItem>>& items() const {
    return items_;
  }
  const std::string& content_type() const { return content_type_; }
  const std::string& content_disposition() const {
    return content_disposition_;
  }
  size_t GetMemoryUsage() const;

  const std::string& uuid() const { return uuid_; }

 private:
  friend class BlobDataBuilder;
  friend class BlobStorageContext;
  friend STORAGE_EXPORT void PrintTo(const BlobDataSnapshot& x,
                                     ::std::ostream* os);
  BlobDataSnapshot(const std::string& uuid,
                   const std::string& content_type,
                   const std::string& content_disposition);
  BlobDataSnapshot(const std::string& uuid,
                   const std::string& content_type,
                   const std::string& content_disposition,
                   const std::vector<scoped_refptr<BlobDataItem>>& items);

  const std::string uuid_;
  const std::string content_type_;
  const std::string content_disposition_;

  // Non-const for constrution in BlobStorageContext
  std::vector<scoped_refptr<BlobDataItem>> items_;
};

}  // namespace storage
#endif  // STORAGE_BROWSER_BLOB_BLOB_DATA_SNAPSHOT_H_
