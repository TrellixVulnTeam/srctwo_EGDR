// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LineLayoutRubyRun_h
#define LineLayoutRubyRun_h

#include "core/layout/LayoutRubyRun.h"
#include "core/layout/api/LineLayoutBlockFlow.h"
#include "core/layout/api/LineLayoutRubyBase.h"
#include "core/layout/api/LineLayoutRubyText.h"

namespace blink {

class LineLayoutRubyRun : public LineLayoutBlockFlow {
 public:
  explicit LineLayoutRubyRun(LayoutRubyRun* layout_ruby_run)
      : LineLayoutBlockFlow(layout_ruby_run) {}

  explicit LineLayoutRubyRun(const LineLayoutItem& item)
      : LineLayoutBlockFlow(item) {
    SECURITY_DCHECK(!item || item.IsRubyRun());
  }

  explicit LineLayoutRubyRun(std::nullptr_t) : LineLayoutBlockFlow(nullptr) {}

  LineLayoutRubyRun() {}

  void GetOverhang(bool first_line,
                   LineLayoutItem start_layout_item,
                   LineLayoutItem end_layout_item,
                   int& start_overhang,
                   int& end_overhang) const {
    ToRubyRun()->GetOverhang(first_line, start_layout_item.GetLayoutObject(),
                             end_layout_item.GetLayoutObject(), start_overhang,
                             end_overhang);
  }

  LineLayoutRubyText RubyText() const {
    return LineLayoutRubyText(ToRubyRun()->RubyText());
  }

  LineLayoutRubyBase RubyBase() const {
    return LineLayoutRubyBase(ToRubyRun()->RubyBase());
  }

  bool CanBreakBefore(const LazyLineBreakIterator& iterator) const {
    return ToRubyRun()->CanBreakBefore(iterator);
  }

 private:
  LayoutRubyRun* ToRubyRun() { return ToLayoutRubyRun(GetLayoutObject()); }

  const LayoutRubyRun* ToRubyRun() const {
    return ToLayoutRubyRun(GetLayoutObject());
  }
};

}  // namespace blink

#endif  // LineLayoutRubyRun_h
