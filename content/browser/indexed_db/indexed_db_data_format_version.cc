// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/indexed_db/indexed_db_data_format_version.h"

#include "third_party/WebKit/public/web/WebSerializedScriptValueVersion.h"
#include "v8/include/v8-value-serializer-version.h"

namespace content {

// static
IndexedDBDataFormatVersion IndexedDBDataFormatVersion::current_(
    v8::CurrentValueSerializerFormatVersion(),
    blink::kSerializedScriptValueVersion);

}  // namespace content
