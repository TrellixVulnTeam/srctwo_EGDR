// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/platform/WebMediaPlayerClient.h"

// This WebMediaPlayerClient.cpp, which includes only
// and WebMediaPlayerClient.h, should be in
// Source/platform/exported, because WebMediaPlayerClient is not
// compiled without this cpp.
// So if we don't have this cpp, we will see unresolved symbol error
// when constructor/destructor's address is required.
