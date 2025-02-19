// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_INDEXED_DB_INDEXED_DB_RETURN_VALUE_H_
#define CONTENT_BROWSER_INDEXED_DB_INDEXED_DB_RETURN_VALUE_H_

#include "content/browser/indexed_db/indexed_db_value.h"
#include "content/common/content_export.h"
#include "content/common/indexed_db/indexed_db_key.h"
#include "content/common/indexed_db/indexed_db_key_path.h"

namespace content {

// Values returned to the IDB client may contain a primary key value generated
// by IDB. This is optional and only done when using a key generator. This key
// value cannot (at least easily) be amended to the object being written to the
// database, so they are kept separately, and sent back with the original data
// so that the render process can amend the returned object.
struct CONTENT_EXPORT IndexedDBReturnValue : public IndexedDBValue {
  IndexedDBKey primary_key;  // primary key (only when using key generator)
  IndexedDBKeyPath key_path;
};

}  // namespace content

#endif  // CONTENT_BROWSER_INDEXED_DB_INDEXED_DB_RETURN_VALUE_H_
