// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_PAYMENTS_CONTENT_UTILITY_FINGERPRINT_PARSER_H_
#define COMPONENTS_PAYMENTS_CONTENT_UTILITY_FINGERPRINT_PARSER_H_

#include <stddef.h>

#include <string>
#include <vector>

namespace payments {

// Converts a string representation of a 32-byte array (e.g., "01:02:03:04")
// into a list of the corresponding bytes. Returns an empty list on error.
std::vector<uint8_t> FingerprintStringToByteArray(const std::string& input);

}  // namespace payments

#endif  // COMPONENTS_PAYMENTS_CONTENT_UTILITY_FINGERPRINT_PARSER_H_
