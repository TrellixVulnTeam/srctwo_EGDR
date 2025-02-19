// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXTENSIONS_BROWSER_API_EXTENSIONS_API_CLIENT_H_
#define EXTENSIONS_BROWSER_API_EXTENSIONS_API_CLIENT_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/memory/ref_counted.h"
#include "extensions/browser/api/clipboard/clipboard_api.h"
#include "extensions/browser/api/declarative_content/content_rules_registry.h"
#include "extensions/browser/api/storage/settings_namespace.h"
#include "extensions/common/api/clipboard.h"

class GURL;

namespace base {
template <class T>
class ObserverListThreadSafe;
}

namespace content {
class BrowserContext;
class WebContents;
}

namespace guest_view {
class GuestViewManagerDelegate;
}  // namespace guest_view

namespace extensions {

class AppViewGuestDelegate;
class ContentRulesRegistry;
class DevicePermissionsPrompt;
class ExtensionOptionsGuest;
class ExtensionOptionsGuestDelegate;
class FeedbackPrivateDelegate;
class FileSystemDelegate;
class ManagementAPIDelegate;
class MessagingDelegate;
class MetricsPrivateDelegate;
class MimeHandlerViewGuest;
class MimeHandlerViewGuestDelegate;
class NetworkingCastPrivateDelegate;
class NonNativeFileSystemDelegate;
class RulesCacheDelegate;
class SettingsObserver;
class ValueStoreCache;
class ValueStoreFactory;
class VirtualKeyboardDelegate;
class WebRequestEventRouterDelegate;
class WebViewGuest;
class WebViewGuestDelegate;
class WebViewPermissionHelper;
class WebViewPermissionHelperDelegate;

// Allows the embedder of the extensions module to customize its support for
// API features. The embedder must create a single instance in the browser
// process. Provides a default implementation that does nothing.
class ExtensionsAPIClient {
 public:
  // Construction sets the single instance.
  ExtensionsAPIClient();

  // Destruction clears the single instance.
  virtual ~ExtensionsAPIClient();

  // Returns the single instance of |this|.
  static ExtensionsAPIClient* Get();

  // Storage API support.

  // Add any additional value store caches (e.g. for chrome.storage.managed)
  // to |caches|. By default adds nothing.
  virtual void AddAdditionalValueStoreCaches(
      content::BrowserContext* context,
      const scoped_refptr<ValueStoreFactory>& factory,
      const scoped_refptr<base::ObserverListThreadSafe<SettingsObserver>>&
          observers,
      std::map<settings_namespace::Namespace, ValueStoreCache*>* caches);

  // Attaches any extra web contents helpers (like ExtensionWebContentsObserver)
  // to |web_contents|.
  virtual void AttachWebContentsHelpers(content::WebContents* web_contents)
      const;

  // Returns true if the header should be hidden to extensions.
  virtual bool ShouldHideResponseHeader(const GURL& url,
                                        const std::string& header_name) const;

  // Creates the AppViewGuestDelegate.
  virtual AppViewGuestDelegate* CreateAppViewGuestDelegate() const;

  // Returns a delegate for ExtensionOptionsGuest. The caller owns the returned
  // ExtensionOptionsGuestDelegate.
  virtual ExtensionOptionsGuestDelegate* CreateExtensionOptionsGuestDelegate(
      ExtensionOptionsGuest* guest) const;

  // Returns a delegate for GuestViewManagerDelegate.
  virtual std::unique_ptr<guest_view::GuestViewManagerDelegate>
  CreateGuestViewManagerDelegate(content::BrowserContext* context) const;

  // Creates a delegate for MimeHandlerViewGuest.
  virtual std::unique_ptr<MimeHandlerViewGuestDelegate>
  CreateMimeHandlerViewGuestDelegate(MimeHandlerViewGuest* guest) const;

  // Returns a delegate for some of WebViewGuest's behavior. The caller owns the
  // returned WebViewGuestDelegate.
  virtual WebViewGuestDelegate* CreateWebViewGuestDelegate(
      WebViewGuest* web_view_guest) const;

  // Returns a delegate for some of WebViewPermissionHelper's behavior. The
  // caller owns the returned WebViewPermissionHelperDelegate.
  virtual WebViewPermissionHelperDelegate*
  CreateWebViewPermissionHelperDelegate(
      WebViewPermissionHelper* web_view_permission_helper) const;

  // Creates a delegate for WebRequestEventRouter.
  virtual std::unique_ptr<WebRequestEventRouterDelegate>
  CreateWebRequestEventRouterDelegate() const;

  // TODO(wjmaclean): Remove this when (if) ContentRulesRegistry code moves
  // to extensions/browser/api.
  virtual scoped_refptr<ContentRulesRegistry> CreateContentRulesRegistry(
      content::BrowserContext* browser_context,
      RulesCacheDelegate* cache_delegate) const;

  // Creates a DevicePermissionsPrompt appropriate for the embedder.
  virtual std::unique_ptr<DevicePermissionsPrompt>
  CreateDevicePermissionsPrompt(content::WebContents* web_contents) const;

  // Returns a delegate for some of VirtualKeyboardAPI's behavior.
  virtual std::unique_ptr<VirtualKeyboardDelegate>
  CreateVirtualKeyboardDelegate() const;

  // Creates a delegate for handling the management extension api.
  virtual ManagementAPIDelegate* CreateManagementAPIDelegate() const;

  // If supported by the embedder, returns a delegate for embedder-dependent
  // MetricsPrivateAPI behavior.
  virtual MetricsPrivateDelegate* GetMetricsPrivateDelegate();

  // Creates a delegate for networking.castPrivate's API behavior.
  virtual NetworkingCastPrivateDelegate* GetNetworkingCastPrivateDelegate();

  // Returns a delegate for embedder-specific chrome.fileSystem behavior.
  virtual FileSystemDelegate* GetFileSystemDelegate();

  // Returns a delegate for embedder-specific extension messaging.
  virtual MessagingDelegate* GetMessagingDelegate();

  // Returns a delegate for the chrome.feedbackPrivate API.
  virtual FeedbackPrivateDelegate* GetFeedbackPrivateDelegate();

#if defined(OS_CHROMEOS)
  // If supported by the embedder, returns a delegate for querying non-native
  // file systems.
  virtual NonNativeFileSystemDelegate* GetNonNativeFileSystemDelegate();

  // Saves image data on clipboard.
  virtual void SaveImageDataToClipboard(
      const std::vector<char>& image_data,
      api::clipboard::ImageType type,
      AdditionalDataItemList additional_items,
      const base::Closure& success_callback,
      const base::Callback<void(const std::string&)>& error_callback);
#endif

  // NOTE: If this interface gains too many methods (perhaps more than 20) it
  // should be split into one interface per API.
};

}  // namespace extensions

#endif  // EXTENSIONS_BROWSER_API_EXTENSIONS_API_CLIENT_H_
