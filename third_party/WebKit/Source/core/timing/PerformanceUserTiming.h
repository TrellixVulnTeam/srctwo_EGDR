/*
 * Copyright (C) 2012 Intel Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef PerformanceUserTiming_h
#define PerformanceUserTiming_h

#include "core/timing/PerformanceBase.h"
#include "core/timing/PerformanceTiming.h"
#include "platform/heap/Handle.h"
#include "platform/wtf/text/WTFString.h"

namespace blink {

class ExceptionState;
class PerformanceBase;

typedef unsigned long long (
    PerformanceTiming::*NavigationTimingFunction)() const;
using PerformanceEntryMap = HeapHashMap<String, PerformanceEntryVector>;

class UserTiming final : public GarbageCollected<UserTiming> {
 public:
  static UserTiming* Create(PerformanceBase& performance) {
    return new UserTiming(performance);
  }

  PerformanceEntry* Mark(const String& mark_name, ExceptionState&);
  void ClearMarks(const String& mark_name);

  PerformanceEntry* Measure(const String& measure_name,
                            const String& start_mark,
                            const String& end_mark,
                            ExceptionState&);
  void ClearMeasures(const String& measure_name);

  PerformanceEntryVector GetMarks() const;
  PerformanceEntryVector GetMeasures() const;

  PerformanceEntryVector GetMarks(const String& name) const;
  PerformanceEntryVector GetMeasures(const String& name) const;

  DECLARE_TRACE();

 private:
  explicit UserTiming(PerformanceBase&);

  double FindExistingMarkStartTime(const String& mark_name, ExceptionState&);

  Member<PerformanceBase> performance_;
  PerformanceEntryMap marks_map_;
  PerformanceEntryMap measures_map_;
};

}  // namespace blink

#endif  // !defined(PerformanceUserTiming_h)
