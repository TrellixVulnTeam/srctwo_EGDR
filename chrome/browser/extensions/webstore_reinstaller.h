// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_EXTENSIONS_WEBSTORE_REINSTALLER_H_
#define CHROME_BROWSER_EXTENSIONS_WEBSTORE_REINSTALLER_H_

#include "chrome/browser/extensions/webstore_standalone_installer.h"
#include "content/public/browser/web_contents_observer.h"

namespace extensions {

// Reinstalls an extension from the webstore. This will first prompt the user if
// they want to reinstall (using the verbase "Repair", since this is our action
// for repairing corrupted extensions), and, if the user agrees, will uninstall
// the extension and reinstall it directly from the webstore.
class WebstoreReinstaller : public WebstoreStandaloneInstaller,
                            public content::WebContentsObserver {
 public:
  WebstoreReinstaller(content::WebContents* web_contents,
                      const std::string& extension_id,
                      const WebstoreStandaloneInstaller::Callback& callback);

  // Begin the reinstall process. |callback| (from the constructor) will be
  // called upon completion.
  void BeginReinstall();

 private:
  ~WebstoreReinstaller() override;

  // WebstoreStandaloneInstaller:
  bool CheckRequestorAlive() const override;
  const GURL& GetRequestorURL() const override;
  bool ShouldShowPostInstallUI() const override;
  bool ShouldShowAppInstalledBubble() const override;
  content::WebContents* GetWebContents() const override;
  std::unique_ptr<ExtensionInstallPrompt::Prompt> CreateInstallPrompt()
      const override;
  bool CheckInlineInstallPermitted(const base::DictionaryValue& webstore_data,
                                   std::string* error) const override;
  bool CheckRequestorPermitted(const base::DictionaryValue& webstore_data,
                               std::string* error) const override;
  void OnInstallPromptDone(ExtensionInstallPrompt::Result result) override;

  // content::WebContentsObserver:
  void WebContentsDestroyed() override;

  // Called once all data from the old extension installation is removed.
  void OnDeletionDone();
};

}  // namespace extensions

#endif  // CHROME_BROWSER_EXTENSIONS_WEBSTORE_REINSTALLER_H_
