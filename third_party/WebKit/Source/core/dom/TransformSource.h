/*
 * Copyright (C) 2009 Jakub Wieczorek <faw217@gmail.com>
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
 */

#ifndef TransformSource_h
#define TransformSource_h

#include <libxml/tree.h>
#include "platform/wtf/Allocator.h"
#include "platform/wtf/Noncopyable.h"

namespace blink {

class TransformSource {
  WTF_MAKE_NONCOPYABLE(TransformSource);
  USING_FAST_MALLOC(TransformSource);

 public:
  explicit TransformSource(xmlDocPtr source);
  ~TransformSource();

  xmlDocPtr PlatformSource() const { return source_; }

 private:
  xmlDocPtr source_;
};

}  // namespace blink

#endif  // TransformSource_h
