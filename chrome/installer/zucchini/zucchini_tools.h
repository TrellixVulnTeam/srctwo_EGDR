// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_INSTALLER_ZUCCHINI_ZUCCHINI_TOOLS_H_
#define CHROME_INSTALLER_ZUCCHINI_ZUCCHINI_TOOLS_H_

#include <iosfwd>

#include "chrome/installer/zucchini/buffer_view.h"
#include "chrome/installer/zucchini/zucchini.h"

namespace zucchini {

// Prints stats on references found in |image|. If |do_dump| is true, then
// prints all references (locations and targets).
status::Code ReadReferences(ConstBufferView image,
                            bool do_dump,
                            std::ostream& out);

}  // namespace zucchini

#endif  // CHROME_INSTALLER_ZUCCHINI_ZUCCHINI_TOOLS_H_
