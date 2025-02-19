// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/chrome/browser/ui/authentication/resized_avatar_cache.h"

#import "ios/chrome/browser/ui/uikit_ui_util.h"
#import "ios/public/provider/chrome/browser/chrome_browser_provider.h"
#import "ios/public/provider/chrome/browser/signin/chrome_identity.h"
#import "ios/public/provider/chrome/browser/signin/chrome_identity_service.h"
#import "ios/public/provider/chrome/browser/signin/signin_resources_provider.h"

#if !defined(__has_feature) || !__has_feature(objc_arc)
#error "This file requires ARC support."
#endif

namespace {
const CGFloat kAccountProfilePhotoDimension = 40.0f;
}  // namespace

@implementation ResizedAvatarCache {
  // Retains resized images. Key is Chrome Identity.
  NSCache<ChromeIdentity*, UIImage*>* _resizedImages;

  // Holds weak references to the cached avatar image from the
  // ChromeIdentityService. Key is Chrome Identity.
  NSMapTable<ChromeIdentity*, UIImage*>* _originalImages;
}

- (instancetype)init {
  self = [super init];
  if (self) {
    _resizedImages = [[NSCache alloc] init];
    _originalImages = [NSMapTable strongToWeakObjectsMapTable];
  }
  return self;
}

- (UIImage*)resizedAvatarForIdentity:(ChromeIdentity*)identity {
  UIImage* image = ios::GetChromeBrowserProvider()
                       ->GetChromeIdentityService()
                       ->GetCachedAvatarForIdentity(identity);
  if (!image) {
    image = ios::GetChromeBrowserProvider()
                ->GetSigninResourcesProvider()
                ->GetDefaultAvatar();
    // No cached image, trigger a fetch, which will notify all observers.
    ios::GetChromeBrowserProvider()
        ->GetChromeIdentityService()
        ->GetAvatarForIdentity(identity, ^(UIImage*){
                               });
  }

  // If the currently used image has already been resized, use it.
  if ([_resizedImages objectForKey:identity] &&
      [_originalImages objectForKey:identity] == image)
    return [_resizedImages objectForKey:identity];

  [_originalImages setObject:image forKey:identity];

  // Resize the profile image.
  CGFloat dimension = kAccountProfilePhotoDimension;
  if (image.size.width != dimension || image.size.height != dimension) {
    image = ResizeImage(image, CGSizeMake(dimension, dimension),
                        ProjectionMode::kAspectFit);
  }
  [_resizedImages setObject:image forKey:identity];
  return image;
}

@end
