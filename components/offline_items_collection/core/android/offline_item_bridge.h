// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_OFFLINE_ITEMS_COLLECTION_CORE_ANDROID_OFFLINE_ITEM_BRIDGE_H_
#define COMPONENTS_OFFLINE_ITEMS_COLLECTION_CORE_ANDROID_OFFLINE_ITEM_BRIDGE_H_

#include <vector>

#include "base/android/jni_android.h"
#include "base/android/scoped_java_ref.h"
#include "components/offline_items_collection/core/offline_item.h"

namespace offline_items_collection {
namespace android {

// A helper class for creating Java OfflineItem instances from the C++
// OfflineItem counterpart.
class OfflineItemBridge {
 public:
  // Creates a Java OfflineItem from |item|.
  static base::android::ScopedJavaLocalRef<jobject> CreateOfflineItem(
      JNIEnv* env,
      const OfflineItem* const item);

  // Creates an Java ArrayList<OfflineItem> from |items|.
  static base::android::ScopedJavaLocalRef<jobject> CreateOfflineItemList(
      JNIEnv* env,
      const std::vector<OfflineItem>& items);

 private:
  OfflineItemBridge();
};

}  // namespace android
}  // namespace offline_items_collection

#endif  // COMPONENTS_OFFLINE_ITEMS_COLLECTION_CORE_ANDROID_OFFLINE_ITEM_BRIDGE_H_
