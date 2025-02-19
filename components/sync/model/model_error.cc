// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/sync/model/model_error.h"

namespace syncer {

ModelError::ModelError(const tracked_objects::Location& location,
                       const std::string& message)
    : location_(location), message_(message) {}

ModelError::~ModelError() = default;

const tracked_objects::Location& ModelError::location() const {
  return location_;
}

const std::string& ModelError::message() const {
  return message_;
}

}  // namespace syncer
