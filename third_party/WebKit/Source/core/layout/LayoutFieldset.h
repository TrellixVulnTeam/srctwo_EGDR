/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2004, 2006, 2009 Apple Inc. All rights reserved.
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

#ifndef LayoutFieldset_h
#define LayoutFieldset_h

#include "core/layout/LayoutBlockFlow.h"

namespace blink {

class LayoutFieldset final : public LayoutBlockFlow {
 public:
  explicit LayoutFieldset(Element*);

  LayoutBox* FindInFlowLegend() const;

  const char* GetName() const override { return "LayoutFieldset"; }

 private:
  bool IsOfType(LayoutObjectType type) const override {
    return type == kLayoutObjectFieldset || LayoutBlockFlow::IsOfType(type);
  }

  LayoutObject* LayoutSpecialExcludedChild(bool relayout_children,
                                           SubtreeLayoutScope&) override;

  void ComputePreferredLogicalWidths() override;

  void PaintBoxDecorationBackground(const PaintInfo&,
                                    const LayoutPoint&) const override;
  void PaintMask(const PaintInfo&, const LayoutPoint&) const override;
};

DEFINE_LAYOUT_OBJECT_TYPE_CASTS(LayoutFieldset, IsFieldset());

}  // namespace blink

#endif  // LayoutFieldset_h
