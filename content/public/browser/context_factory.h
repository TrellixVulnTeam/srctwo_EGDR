// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_BROWSER_CONTEXT_FACTORY_H_
#define CONTENT_PUBLIC_BROWSER_CONTEXT_FACTORY_H_

#include "content/common/content_export.h"

namespace ui {
class ContextFactory;
class ContextFactoryPrivate;
}

namespace content {

// Returns the singleton ContextFactory used by content. The return value is
// owned by content.
CONTENT_EXPORT ui::ContextFactory* GetContextFactory();

// Returns the singleton ContextFactoryPrivate used by content. The return value
// is owned by content.
// TODO(fsamuel): Once Mus is used on all platforms, this private interface
// should not be necessary.
CONTENT_EXPORT ui::ContextFactoryPrivate* GetContextFactoryPrivate();

}  // namespace content

#endif  // CONTENT_PUBLIC_BROWSER_CONTEXT_FACTORY_H_
