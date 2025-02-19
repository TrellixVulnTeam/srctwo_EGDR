// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_CONTENT_SUGGESTIONS_CONTENT_SUGGESTIONS_MEDIATOR_H_
#define IOS_CHROME_BROWSER_CONTENT_SUGGESTIONS_CONTENT_SUGGESTIONS_MEDIATOR_H_

#import <UIKit/UIKit.h>

#include <memory>

#import "ios/chrome/browser/content_suggestions/content_suggestions_mediator.h"
#import "ios/chrome/browser/content_suggestions/content_suggestions_metrics_recorder.h"
#import "ios/chrome/browser/ui/content_suggestions/content_suggestions_data_source.h"

namespace favicon {
class LargeIconService;
}

namespace ntp_snippets {
class ContentSuggestionsService;
}

namespace ntp_tiles {
class MostVisitedSites;
}

@protocol ContentSuggestionsCommands;
@protocol ContentSuggestionsGestureCommands;
@protocol ContentSuggestionsHeaderProvider;
@class ContentSuggestionIdentifier;
class GURL;
class LargeIconCache;
class NotificationPromoWhatsNew;

// Mediator for ContentSuggestions. Makes the interface between a
// ntp_snippets::ContentSuggestionsService and the Objective-C services using
// its data.
@interface ContentSuggestionsMediator
    : NSObject<ContentSuggestionsDataSource,
               ContentSuggestionsMetricsRecorderDelegate>

// Initialize the mediator with the |contentService| to mediate.
- (nullable instancetype)
initWithContentService:
    (nonnull ntp_snippets::ContentSuggestionsService*)contentService
      largeIconService:(nonnull favicon::LargeIconService*)largeIconService
        largeIconCache:(nullable LargeIconCache*)largeIconCache
       mostVisitedSite:
           (std::unique_ptr<ntp_tiles::MostVisitedSites>)mostVisitedSites
    NS_DESIGNATED_INITIALIZER;

- (nullable instancetype)init NS_UNAVAILABLE;

// Command handler for the mediator.
@property(nonatomic, weak, nullable)
    id<ContentSuggestionsCommands, ContentSuggestionsGestureCommands>
        commandHandler;

@property(nonatomic, weak, nullable) id<ContentSuggestionsHeaderProvider>
    headerProvider;

// Whether to force the reload the Reading List section next time it is updated.
// Reset to NO after actual reload.
@property(nonatomic, assign) BOOL readingListNeedsReload;

// The notification promo owned by this mediator.
- (nonnull NotificationPromoWhatsNew*)notificationPromo;

// Blacklists the URL from the Most Visited sites.
- (void)blacklistMostVisitedURL:(GURL)URL;

// Whitelists the URL from the Most Visited sites.
- (void)whitelistMostVisitedURL:(GURL)URL;

@end

#endif  // IOS_CHROME_BROWSER_CONTENT_SUGGESTIONS_CONTENT_SUGGESTIONS_MEDIATOR_H_
