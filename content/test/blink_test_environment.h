// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_TEST_BLINK_TEST_ENVIRONMENT_H_
#define CONTENT_TEST_BLINK_TEST_ENVIRONMENT_H_

// This package provides functions used by webkit_unit_tests.
namespace content {

// Initializes Blink test environment for unit tests.
void SetUpBlinkTestEnvironment();

// Terminates Blink test environment for unit tests.
void TearDownBlinkTestEnvironment();

}  // namespace content

#endif  // CONTENT_TEST_BLINK_TEST_ENVIRONMENT_H_
