/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
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

#ifndef SVGPathBlender_h
#define SVGPathBlender_h

#include "platform/heap/Handle.h"
#include "platform/wtf/Noncopyable.h"

namespace blink {

class SVGPathConsumer;
class SVGPathByteStreamSource;

class SVGPathBlender final {
  WTF_MAKE_NONCOPYABLE(SVGPathBlender);
  STACK_ALLOCATED();

 public:
  SVGPathBlender(SVGPathByteStreamSource* from_source,
                 SVGPathByteStreamSource* to_source,
                 SVGPathConsumer*);

  bool AddAnimatedPath(unsigned repeat_count);
  bool BlendAnimatedPath(float);

 private:
  class BlendState;
  bool BlendAnimatedPath(BlendState&);

  SVGPathByteStreamSource* from_source_;
  SVGPathByteStreamSource* to_source_;
  SVGPathConsumer* consumer_;
};

}  // namespace blink

#endif  // SVGPathBlender_h
