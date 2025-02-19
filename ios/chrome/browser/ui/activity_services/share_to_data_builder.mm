// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ios/chrome/browser/ui/activity_services/share_to_data_builder.h"

#include "base/logging.h"
#include "ios/chrome/browser/tabs/tab.h"
#include "ios/chrome/browser/ui/activity_services/chrome_activity_item_thumbnail_generator.h"
#include "ios/chrome/browser/ui/activity_services/share_to_data.h"

#if !defined(__has_feature) || !__has_feature(objc_arc)
#error "This file requires ARC support."
#endif

namespace activity_services {

ShareToData* ShareToDataForTab(Tab* tab) {
  // For crash documented in crbug.com/503955, tab.url which is being passed
  // as a reference parameter caused a crash due to invalid address which
  // which suggests that |tab| may be deallocated along the way. Check that
  // tab is still valid by checking webState which would be deallocated if
  // tab is being closed.
  if (!tab.webState)
    return nil;
  DCHECK(tab);
  // If the original page title exists, it is expected to match the tab title.
  // If this ever changes, then a decision has to be made on which one should
  // be used for sharing.
  DCHECK(!tab.originalTitle || [tab.originalTitle isEqualToString:tab.title]);
  BOOL isPagePrintable = [tab viewForPrinting] != nil;
  ThumbnailGeneratorBlock thumbnailGenerator =
      activity_services::ThumbnailGeneratorForTab(tab);
  return [[ShareToData alloc] initWithURL:tab.visibleURL
                                    title:tab.title
                          isOriginalTitle:(tab.originalTitle != nil)
                          isPagePrintable:isPagePrintable
                       thumbnailGenerator:thumbnailGenerator];
}

}  // namespace activity_services
