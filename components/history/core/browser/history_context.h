// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_HISTORY_CORE_BROWSER_HISTORY_CONTEXT_H_
#define COMPONENTS_HISTORY_CORE_BROWSER_HISTORY_CONTEXT_H_

#include "base/macros.h"

namespace history {

// Context is an empty struct that is used to scope the lifetime of
// navigation entry references. They don't have any data and their
// lifetime is controlled by the embedder, thus they don't need a
// virtual destructor.
struct Context {
 protected:
  Context() {}
  ~Context() {}

 private:
  DISALLOW_COPY_AND_ASSIGN(Context);
};

// Identifier for a context to scope the lifetime of navigation entry
// references. ContextIDs are used in comparison only and are never
// dereferenced.
typedef Context* ContextID;

}  // namespace history

#endif  // COMPONENTS_HISTORY_CORE_BROWSER_HISTORY_CONTEXT_H_
