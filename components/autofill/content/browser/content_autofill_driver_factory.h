// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_AUTOFILL_CONTENT_BROWSER_CONTENT_AUTOFILL_DRIVER_FACTORY_H_
#define COMPONENTS_AUTOFILL_CONTENT_BROWSER_CONTENT_AUTOFILL_DRIVER_FACTORY_H_

#include <string>

#include "base/supports_user_data.h"
#include "components/autofill/content/common/autofill_driver.mojom.h"
#include "components/autofill/core/browser/autofill_driver_factory.h"
#include "components/autofill/core/browser/autofill_manager.h"
#include "content/public/browser/web_contents_observer.h"

namespace content {
class RenderFrameHost;
}

namespace autofill {

class ContentAutofillDriver;
class AutofillProvider;

// Manages lifetime of ContentAutofillDriver. One Factory per WebContents
// creates one Driver per RenderFrame.
class ContentAutofillDriverFactory : public AutofillDriverFactory,
                                     public content::WebContentsObserver,
                                     public base::SupportsUserData::Data {
 public:
  ~ContentAutofillDriverFactory() override;

  static void CreateForWebContentsAndDelegate(
      content::WebContents* contents,
      AutofillClient* client,
      const std::string& app_locale,
      AutofillManager::AutofillDownloadManagerState enable_download_manager);

  static void CreateForWebContentsAndDelegate(
      content::WebContents* contents,
      AutofillClient* client,
      const std::string& app_locale,
      AutofillManager::AutofillDownloadManagerState enable_download_manager,
      AutofillProvider* provider);

  static ContentAutofillDriverFactory* FromWebContents(
      content::WebContents* contents);
  static void BindAutofillDriver(
      mojom::AutofillDriverRequest request,
      content::RenderFrameHost* render_frame_host);

  // Gets the |ContentAutofillDriver| associated with |render_frame_host|.
  // |render_frame_host| must be owned by |web_contents()|.
  ContentAutofillDriver* DriverForFrame(
      content::RenderFrameHost* render_frame_host);

  // content::WebContentsObserver:
  void RenderFrameCreated(content::RenderFrameHost* render_frame_host) override;
  void RenderFrameDeleted(content::RenderFrameHost* render_frame_host) override;
  void DidFinishNavigation(
      content::NavigationHandle* navigation_handle) override;
  void WasHidden() override;

  static const char kContentAutofillDriverFactoryWebContentsUserDataKey[];

 private:
  ContentAutofillDriverFactory(
      content::WebContents* web_contents,
      AutofillClient* client,
      const std::string& app_locale,
      AutofillManager::AutofillDownloadManagerState enable_download_manager,
      AutofillProvider* provider);

  std::string app_locale_;
  AutofillManager::AutofillDownloadManagerState enable_download_manager_;
  AutofillProvider* provider_;
};

}  // namespace autofill

#endif  // COMPONENTS_AUTOFILL_CONTENT_BROWSER_CONTENT_AUTOFILL_DRIVER_FACTORY_H_
