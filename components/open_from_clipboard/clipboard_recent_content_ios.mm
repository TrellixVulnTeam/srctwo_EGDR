// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "components/open_from_clipboard/clipboard_recent_content_ios.h"

#import <CommonCrypto/CommonDigest.h>
#include <stddef.h>
#include <stdint.h>
#import <UIKit/UIKit.h>

#import "base/ios/ios_util.h"
#include "base/logging.h"
#include "base/macros.h"
#include "base/metrics/user_metrics.h"
#include "base/strings/sys_string_conversions.h"
#include "base/sys_info.h"
#import "components/open_from_clipboard/clipboard_recent_content_impl_ios.h"
#import "net/base/mac/url_conversions.h"
#include "url/gurl.h"
#include "url/url_constants.h"

namespace {

// Schemes accepted by the ClipboardRecentContentIOS.
const char* kAuthorizedSchemes[] = {
    url::kHttpScheme, url::kHttpsScheme, url::kDataScheme, url::kAboutScheme,
};

// Get the list of authorized schemes.
NSSet<NSString*>* getAuthorizedSchemeList(
    const std::string& application_scheme) {
  NSMutableSet<NSString*>* schemes = [NSMutableSet set];
  for (size_t i = 0; i < arraysize(kAuthorizedSchemes); ++i) {
    [schemes addObject:base::SysUTF8ToNSString(kAuthorizedSchemes[i])];
  }
  if (!application_scheme.empty()) {
    [schemes addObject:base::SysUTF8ToNSString(application_scheme)];
  }

  return [schemes copy];
}

}  // namespace

@interface ClipboardRecentContentDelegateImpl
    : NSObject<ClipboardRecentContentDelegate>
@end

@implementation ClipboardRecentContentDelegateImpl

- (void)onClipboardChanged {
  base::RecordAction(base::UserMetricsAction("MobileOmniboxClipboardChanged"));
}

@end

ClipboardRecentContentIOS::ClipboardRecentContentIOS(
    const std::string& application_scheme,
    NSUserDefaults* group_user_defaults)
    : ClipboardRecentContentIOS([[ClipboardRecentContentImplIOS alloc]
             initWithMaxAge:MaximumAgeOfClipboard().InSecondsF()
          authorizedSchemes:getAuthorizedSchemeList(application_scheme)
               userDefaults:group_user_defaults
                   delegate:[[ClipboardRecentContentDelegateImpl alloc]
                                init]]) {}

ClipboardRecentContentIOS::ClipboardRecentContentIOS(
    ClipboardRecentContentImplIOS* implementation) {
  implementation_.reset(implementation);
}

bool ClipboardRecentContentIOS::GetRecentURLFromClipboard(GURL* url) {
  DCHECK(url);
  NSURL* url_from_pasteboard = [implementation_ recentURLFromClipboard];
  GURL converted_url = net::GURLWithNSURL(url_from_pasteboard);
  if (converted_url.is_valid()) {
    *url = std::move(converted_url);
    return true;
  }
  return false;
}

ClipboardRecentContentIOS::~ClipboardRecentContentIOS() {}

base::TimeDelta ClipboardRecentContentIOS::GetClipboardContentAge() const {
  return base::TimeDelta::FromSeconds(
      static_cast<int64_t>([implementation_ clipboardContentAge]));
}

void ClipboardRecentContentIOS::SuppressClipboardContent() {
  [implementation_ suppressClipboardContent];
}
