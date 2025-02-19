// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_TABS_TAB_MODEL_H_
#define IOS_CHROME_BROWSER_TABS_TAB_MODEL_H_

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

#include <memory>

#import "ios/web/public/navigation_manager.h"
#include "ui/base/page_transition_types.h"

class GURL;
class SessionID;
@class SessionServiceIOS;
@class SessionWindowIOS;
@class Tab;
@protocol TabModelObserver;
class TabModelSyncedWindowDelegate;
class TabUsageRecorder;
class WebStateList;

namespace ios {
class ChromeBrowserState;
}

namespace web {
struct Referrer;
class WebState;
}

// A list of notifications about changes in the model or changes in tab
// state.

// A tab is about to load a URL. The tab in question is in the userInfo under
// kTabModelTabKey. This may fire multiple times during a load, for example, on
// redirects.
extern NSString* const kTabModelTabWillStartLoadingNotification;
// A tab started to load a URL. The tab in question is in the userInfo under
// kTabModelTabKey.
extern NSString* const kTabModelTabDidStartLoadingNotification;
// A tab finished loading a URL. The tab in question is in the userInfo under
// kTabModelTabKey.
extern NSString* const kTabModelTabDidFinishLoadingNotification;
// All tabs have finished their shutdown sequences.
// NOTE: This notification is not sent when closing a single tab that happens
// to be the last tab.
extern NSString* const kTabModelAllTabsDidCloseNotification;
// A tab has lost its status as the currently selected tab. The tab in question
// is in the userInfo under kTabModelTabKey.
extern NSString* const kTabModelTabDeselectedNotification;
// A new tab has been created from a link.
extern NSString* const kTabModelNewTabWillOpenNotification;

// Keys for the userInfo included with the above notifications:

// Key that indicates whether to open the next tab in the background.
extern NSString* const kTabModelOpenInBackgroundKey;
// Key for the tab.
extern NSString* const kTabModelTabKey;
// Key for the status of the page load. The value is a NSNumber.
extern NSString* const kTabModelPageLoadSuccess;

// ---------------------------------------------------------------------------

namespace TabModelConstants {

// Position the tab automatically. This value is used as index parameter in
// methods that require an index when the caller doesn't have a preference
// on the position where the tab will be open.
NSUInteger const kTabPositionAutomatically = NSNotFound;

}  // namespace TabModelConstants

// A model of a tab "strip". Although the UI representation may not be a
// traditional strip at all, tabs are still accessed via an integral index.
// The model knows about the currently selected tab in order to maintain
// consistency between multiple views that need the current tab to be
// synchronized.
@interface TabModel : NSObject<NSFastEnumeration>

// Currently active tab.
@property(nonatomic, weak) Tab* currentTab;

// The delegate for sync.
@property(nonatomic, readonly)
    TabModelSyncedWindowDelegate* syncedWindowDelegate;

// BrowserState associated with this TabModel.
@property(nonatomic, readonly) ios::ChromeBrowserState* browserState;

// Unique identifier of this browser for session restore. This ID is only
// unique within the current session, and is not guaranteed to be unique
// across sessions.
@property(nonatomic, readonly) const SessionID& sessionID;

// Records UMA metrics about Tab usage.
@property(nonatomic, readonly) TabUsageRecorder* tabUsageRecorder;

// Whether web usage is enabled (meaning web views can be created in its tabs).
// Defaults to NO.
// Note that generally this should be set via BVC, not directly.
@property(nonatomic, assign) BOOL webUsageEnabled;

// YES if this tab set is off the record.
@property(nonatomic, readonly, getter=isOffTheRecord) BOOL offTheRecord;

// NO if the model has at least one tab.
@property(nonatomic, readonly, getter=isEmpty) BOOL empty;

// Determines the number of tabs in the model.
@property(nonatomic, readonly) NSUInteger count;

// The WebStateList owned by the TabModel.
@property(nonatomic, readonly) WebStateList* webStateList;

// Initializes tabs from a restored session. |-setCurrentTab| needs to be called
// in order to display the views associated with the tabs. Waits until the views
// are ready. |browserState| cannot be nil. |service| cannot be nil; this class
// creates intermediate SessionWindowIOS objects which must be consumed by a
// session service before they are deallocated. |window| can be nil to create
// an empty TabModel. In that case no notification will be sent during object
// creation.
- (instancetype)initWithSessionWindow:(SessionWindowIOS*)window
                       sessionService:(SessionServiceIOS*)service
                         browserState:(ios::ChromeBrowserState*)browserState
    NS_DESIGNATED_INITIALIZER;

- (instancetype)init NS_UNAVAILABLE;

// Restores the given session window after a crash. If there is only one tab,
// showing the NTP, then this tab is clobberred, otherwise, the tab from the
// sessions are added at the end of the tab model.
// Returns YES if the single NTP tab is closed.
- (BOOL)restoreSessionWindow:(SessionWindowIOS*)window;

// Uses the SessionServiceIOS to persist the tab model to disk, either
// immediately or deferred based on the value of |immediately|.
- (void)saveSessionImmediately:(BOOL)immediately;

// Accesses the tab at the given index.
- (Tab*)tabAtIndex:(NSUInteger)index;
- (NSUInteger)indexOfTab:(Tab*)tab;

// Returns the tab which opened this tab, or nil if it's not a child.
- (Tab*)openerOfTab:(Tab*)tab;

// Add/modify tabs.

// Opens a tab at the specified URL. For certain transition types, will consult
// the order controller and thus may only use |index| as a hint. |parentTab| may
// be nil if there is no parent associated with this new tab. |openedByDOM| is
// YES if the page was opened by DOM. The |index| parameter can be set to
// TabModelConstants::kTabPositionAutomatically if the caller doesn't have a
// preference for the position of the tab.
- (Tab*)insertTabWithURL:(const GURL&)URL
                referrer:(const web::Referrer&)referrer
              transition:(ui::PageTransition)transition
                  opener:(Tab*)parentTab
             openedByDOM:(BOOL)openedByDOM
                 atIndex:(NSUInteger)index
            inBackground:(BOOL)inBackground;

// As above, but using WebLoadParams to specify various optional parameters.
- (Tab*)insertTabWithLoadParams:
            (const web::NavigationManager::WebLoadParams&)params
                         opener:(Tab*)parentTab
                    openedByDOM:(BOOL)openedByDOM
                        atIndex:(NSUInteger)index
                   inBackground:(BOOL)inBackground;

// Moves |tab| to the given |index|. |index| must be valid for this tab model
// (must be less than the current number of tabs). |tab| must already be in this
// tab model. If |tab| is already at |index|, this method does nothing and will
// not notify observers.
- (void)moveTab:(Tab*)tab toIndex:(NSUInteger)index;

// Closes the tab at the given |index|. |index| must be valid.
- (void)closeTabAtIndex:(NSUInteger)index;

// Closes the given tab.
- (void)closeTab:(Tab*)tab;

// Closes ALL the tabs.
- (void)closeAllTabs;

// Halts all tabs (terminating active requests) without closing them. Used
// when the app is shutting down.
- (void)haltAllTabs;

// Notifies observers that the given |tab| was changed.
- (void)notifyTabChanged:(Tab*)tab;

// Notifies observers that the snapshot for the given |tab| changed was changed
// to |image|.
- (void)notifyTabSnapshotChanged:(Tab*)tab withImage:(UIImage*)image;

// Adds |observer| to the list of observers. |observer| is not retained. Does
// nothing if |observer| is already in the list. Any added observers must be
// explicitly removed before the TabModel is destroyed.
- (void)addObserver:(id<TabModelObserver>)observer;

// Removes |observer| from the list of observers.
- (void)removeObserver:(id<TabModelObserver>)observer;

// Resets all session counters.
- (void)resetSessionMetrics;

// Records tab session metrics.
- (void)recordSessionMetrics;

// Sets whether the user is primarily interacting with this tab model.
- (void)setPrimary:(BOOL)primary;

// Returns a set with the names of the files received from other applications
// that are still referenced by an open or recently closed tab.
- (NSSet*)currentlyReferencedExternalFiles;

// Called when the browser state provided to this instance is being destroyed.
// At this point the tab model will no longer ever be active, and will likely be
// deallocated soon.
- (void)browserStateDestroyed;
// Called by |tab| to inform the model that a navigation has taken place.
// TODO(crbug.com/661983): once more of the navigation state has moved into WC,
// replace this with WebStateObserver.
- (void)navigationCommittedInTab:(Tab*)tab
                    previousItem:(web::NavigationItem*)previousItem;

@end

#endif  // IOS_CHROME_BROWSER_TABS_TAB_MODEL_H_
