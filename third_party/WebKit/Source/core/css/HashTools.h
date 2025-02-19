/*
 * Copyright (C) 2010 Andras Becsi <abecsi@inf.u-szeged.hu>, University of
 * Szeged
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

#ifndef HashTools_h
#define HashTools_h

#include "platform/wtf/Allocator.h"

namespace blink {

struct Property {
  DISALLOW_NEW();
  int name_offset;
  int id;
};

struct Value {
  DISALLOW_NEW();
  int name_offset;
  int id;
};

const Property* FindProperty(register const char* str, register unsigned len);
const Value* FindValue(register const char* str, register unsigned len);

}  // namespace blink

#endif  // HashTools_h
