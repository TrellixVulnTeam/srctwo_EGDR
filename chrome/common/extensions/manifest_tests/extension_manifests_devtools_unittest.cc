// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/common/extensions/chrome_manifest_url_handlers.h"
#include "chrome/common/extensions/manifest_tests/chrome_manifest_test.h"
#include "extensions/common/extension.h"
#include "extensions/common/manifest_constants.h"
#include "extensions/common/permissions/permissions_data.h"
#include "testing/gtest/include/gtest/gtest.h"

class DevToolsPageManifestTest : public ChromeManifestTest {
};

TEST_F(DevToolsPageManifestTest, DevToolsExtensions) {
  LoadAndExpectError("devtools_extension_url_invalid_type.json",
                     extensions::manifest_errors::kInvalidDevToolsPage);

  scoped_refptr<extensions::Extension> extension;
  extension = LoadAndExpectSuccess("devtools_extension.json");
  EXPECT_EQ(extension->url().spec() + "devtools.html",
            extensions::chrome_manifest_urls::GetDevToolsPage(extension.get())
                .spec());
  EXPECT_TRUE(extension->permissions_data()->HasEffectiveAccessToAllHosts());
}
