// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <utility>

#include "base/values.h"
#include "chrome/common/extensions/manifest_handlers/app_launch_info.h"
#include "chrome/common/extensions/manifest_tests/chrome_manifest_test.h"
#include "testing/gtest/include/gtest/gtest.h"

typedef ChromeManifestTest ValidAppManifestTest;

TEST_F(ValidAppManifestTest, ValidApp) {
  scoped_refptr<extensions::Extension> extension(
      LoadAndExpectSuccess("valid_app.json"));
  extensions::URLPatternSet expected_patterns;
  AddPattern(&expected_patterns, "http://www.google.com/mail/*");
  AddPattern(&expected_patterns, "http://www.google.com/foobar/*");
  EXPECT_EQ(expected_patterns, extension->web_extent());
  EXPECT_EQ(extensions::LAUNCH_CONTAINER_TAB,
            extensions::AppLaunchInfo::GetLaunchContainer(extension.get()));
  EXPECT_EQ(GURL("http://www.google.com/mail/"),
            extensions::AppLaunchInfo::GetLaunchWebURL(extension.get()));
}

TEST_F(ValidAppManifestTest, AllowUnrecognizedPermissions) {
  std::string error;
  std::unique_ptr<base::DictionaryValue> manifest(
      LoadManifest("valid_app.json", &error));
  base::ListValue* permissions = NULL;
  ASSERT_TRUE(manifest->GetList("permissions", &permissions));
  permissions->AppendString("not-a-valid-permission");
  LoadAndExpectSuccess(ManifestData(std::move(manifest), ""));
}
