// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_LOGIN_UI_WEB_CONTENTS_FORCED_TITLE_H_
#define CHROME_BROWSER_CHROMEOS_LOGIN_UI_WEB_CONTENTS_FORCED_TITLE_H_

#include "base/strings/string16.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/web_contents_user_data.h"

namespace chromeos {

// Ensures that the title of the WebContents instance this object is attached
// to is always set to the given title value.
class WebContentsForcedTitle
    : public content::WebContentsObserver,
      public content::WebContentsUserData<WebContentsForcedTitle> {
 public:
  static void CreateForWebContentsWithTitle(content::WebContents* web_contents,
                                            const base::string16& title);

  ~WebContentsForcedTitle() override;

 private:
  WebContentsForcedTitle(content::WebContents* web_contents,
                         const base::string16& title);

  // content::WebContentsObserver:
  void TitleWasSet(content::NavigationEntry* entry, bool explicit_set) override;

  base::string16 title_;

  DISALLOW_COPY_AND_ASSIGN(WebContentsForcedTitle);
};

}  // namespace chromeos

#endif  // CHROME_BROWSER_CHROMEOS_LOGIN_UI_WEB_CONTENTS_FORCED_TITLE_H_
