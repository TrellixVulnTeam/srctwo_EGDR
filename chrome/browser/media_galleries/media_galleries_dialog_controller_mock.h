// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_MEDIA_GALLERIES_MEDIA_GALLERIES_DIALOG_CONTROLLER_MOCK_H_
#define CHROME_BROWSER_MEDIA_GALLERIES_MEDIA_GALLERIES_DIALOG_CONTROLLER_MOCK_H_

#include <stddef.h>

#include "chrome/browser/media_galleries/media_galleries_dialog_controller.h"
#include "testing/gmock/include/gmock/gmock.h"

class MediaGalleriesDialogControllerMock
    : public MediaGalleriesDialogController {
 public:
  MediaGalleriesDialogControllerMock();
  virtual ~MediaGalleriesDialogControllerMock();

  MOCK_CONST_METHOD0(GetHeader, base::string16());
  MOCK_CONST_METHOD0(GetSubtext, base::string16());
  MOCK_CONST_METHOD0(IsAcceptAllowed, bool());
  MOCK_CONST_METHOD0(GetSectionHeaders, std::vector<base::string16>());
  MOCK_CONST_METHOD1(GetSectionEntries, Entries(size_t));
  MOCK_CONST_METHOD0(GetAuxiliaryButtonText, base::string16());
  MOCK_METHOD0(DidClickAuxiliaryButton, void());

  MOCK_METHOD2(DidToggleEntry, void(MediaGalleryPrefId id, bool selected));
  MOCK_METHOD1(DidForgetEntry, void(MediaGalleryPrefId id));
  MOCK_CONST_METHOD0(GetAcceptButtonText, base::string16());
  MOCK_METHOD1(DialogFinished, void(bool));
  MOCK_METHOD1(GetContextMenu, ui::MenuModel*(MediaGalleryPrefId id));
  MOCK_METHOD0(WebContents, content::WebContents*());
};

#endif  // CHROME_BROWSER_MEDIA_GALLERIES_MEDIA_GALLERIES_DIALOG_CONTROLLER_MOCK_H_
