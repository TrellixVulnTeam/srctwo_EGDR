/**
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2003, 2004, 2005, 2006, 2010 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Andrew Wellington (proton@wiretapped.net)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef BidiRun_h
#define BidiRun_h

#include "core/layout/api/LineLayoutItem.h"
#include "platform/text/BidiResolver.h"

namespace blink {

class InlineBox;

struct BidiRun : BidiCharacterRun {
  BidiRun(bool override,
          unsigned char level,
          int start,
          int stop,
          LineLayoutItem line_layout_item,
          WTF::Unicode::CharDirection dir,
          WTF::Unicode::CharDirection override_dir)
      : BidiCharacterRun(override, level, start, stop, dir, override_dir),
        line_layout_item_(line_layout_item),
        box_(nullptr) {
    // Stored in base class to save space.
    has_hyphen_ = false;
  }

  BidiRun(int start,
          int stop,
          unsigned char level,
          LineLayoutItem line_layout_item)
      : BidiCharacterRun(start, stop, level),
        line_layout_item_(line_layout_item),
        box_(nullptr) {
    // Stored in base class to save space.
    has_hyphen_ = false;
  }

  BidiRun* Next() { return static_cast<BidiRun*>(next_); }

 public:
  LineLayoutItem line_layout_item_;
  InlineBox* box_;
};

}  // namespace blink

#endif  // BidiRun_h
