/*
 * This file is part of the WebKit project.
 *
 * Copyright (C) 2009 Michelangelo De Simone <micdesim@gmail.com>
 * Copyright (C) 2010 Apple Inc. All rights reserved.
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

#ifndef ValidityState_h
#define ValidityState_h

#include "core/html/ListedElement.h"
#include "platform/bindings/ScriptWrappable.h"

namespace blink {

class ValidityState final : public GarbageCollected<ValidityState>,
                            public ScriptWrappable {
  DEFINE_WRAPPERTYPEINFO();
  WTF_MAKE_NONCOPYABLE(ValidityState);

 public:
  static ValidityState* Create(ListedElement* control) {
    return new ValidityState(control);
  }
  DEFINE_INLINE_TRACE() { visitor->Trace(control_); }

  String ValidationMessage() const;

  void SetCustomErrorMessage(const String&);

  bool valueMissing() const;
  bool typeMismatch() const;
  bool patternMismatch() const;
  bool tooLong() const;
  bool tooShort() const;
  bool rangeUnderflow() const;
  bool rangeOverflow() const;
  bool stepMismatch() const;
  bool badInput() const;
  bool customError() const;
  bool valid() const;

 private:
  explicit ValidityState(ListedElement* control) : control_(control) {}

  Member<ListedElement> control_;
};

}  // namespace blink

#endif  // ValidityState_h
