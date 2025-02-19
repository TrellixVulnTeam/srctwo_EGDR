// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/printing/print_preview_data_service.h"

#include "base/macros.h"
#include "base/memory/ptr_util.h"
#include "base/memory/ref_counted_memory.h"
#include "base/memory/singleton.h"
#include "base/stl_util.h"
#include "printing/print_job_constants.h"

// PrintPreviewDataStore stores data for preview workflow and preview printing
// workflow.
//
// NOTE:
//   This class stores a list of PDFs. The list |index| is zero-based and can
// be |printing::COMPLETE_PREVIEW_DOCUMENT_INDEX| to represent complete preview
// document. The PDF stored at |printing::COMPLETE_PREVIEW_DOCUMENT_INDEX| is
// optimized with font subsetting, compression, etc. PDF's stored at all other
// indices are unoptimized.
//
// PrintPreviewDataStore owns the data and is responsible for freeing it when
// either:
//    a) There is a new data.
//    b) When PrintPreviewDataStore is destroyed.
//
class PrintPreviewDataStore {
 public:
  PrintPreviewDataStore() {}
  ~PrintPreviewDataStore() {}

  // Get the preview page for the specified |index|.
  void GetPreviewDataForIndex(
      int index,
      scoped_refptr<base::RefCountedBytes>* data) const {
    if (IsInvalidIndex(index))
      return;

    PreviewPageDataMap::const_iterator it = page_data_map_.find(index);
    if (it != page_data_map_.end())
      *data = it->second.get();
  }

  // Set/Update the preview data entry for the specified |index|.
  void SetPreviewDataForIndex(int index,
                              scoped_refptr<base::RefCountedBytes> data) {
    if (IsInvalidIndex(index))
      return;

    page_data_map_[index] = std::move(data);
  }

  // Returns the available draft page count.
  int GetAvailableDraftPageCount() const {
    int page_data_map_size = page_data_map_.size();
    if (base::ContainsKey(page_data_map_,
                          printing::COMPLETE_PREVIEW_DOCUMENT_INDEX))
      page_data_map_size--;
    return page_data_map_size;
  }

 private:
  // 1:1 relationship between page index and its associated preview data.
  // Key: Page index is zero-based and can be
  // |printing::COMPLETE_PREVIEW_DOCUMENT_INDEX| to represent complete preview
  // document.
  // Value: Preview data.
  using PreviewPageDataMap =
      std::map<int, scoped_refptr<base::RefCountedBytes>>;

  static bool IsInvalidIndex(int index) {
    return (index != printing::COMPLETE_PREVIEW_DOCUMENT_INDEX &&
            index < printing::FIRST_PAGE_INDEX);
  }

  PreviewPageDataMap page_data_map_;

  DISALLOW_COPY_AND_ASSIGN(PrintPreviewDataStore);
};

// static
PrintPreviewDataService* PrintPreviewDataService::GetInstance() {
  return base::Singleton<PrintPreviewDataService>::get();
}

PrintPreviewDataService::PrintPreviewDataService() {
}

PrintPreviewDataService::~PrintPreviewDataService() {
}

void PrintPreviewDataService::GetDataEntry(
    int32_t preview_ui_id,
    int index,
    scoped_refptr<base::RefCountedBytes>* data_bytes) const {
  *data_bytes = nullptr;
  PreviewDataStoreMap::const_iterator it = data_store_map_.find(preview_ui_id);
  if (it != data_store_map_.end())
    it->second->GetPreviewDataForIndex(index, data_bytes);
}

void PrintPreviewDataService::SetDataEntry(
    int32_t preview_ui_id,
    int index,
    scoped_refptr<base::RefCountedBytes> data_bytes) {
  if (!base::ContainsKey(data_store_map_, preview_ui_id))
    data_store_map_[preview_ui_id] = base::MakeUnique<PrintPreviewDataStore>();
  data_store_map_[preview_ui_id]->SetPreviewDataForIndex(index,
                                                         std::move(data_bytes));
}

void PrintPreviewDataService::RemoveEntry(int32_t preview_ui_id) {
  data_store_map_.erase(preview_ui_id);
}

int PrintPreviewDataService::GetAvailableDraftPageCount(
    int32_t preview_ui_id) const {
  PreviewDataStoreMap::const_iterator it = data_store_map_.find(preview_ui_id);
  return (it == data_store_map_.end()) ?
      0 : it->second->GetAvailableDraftPageCount();
}
