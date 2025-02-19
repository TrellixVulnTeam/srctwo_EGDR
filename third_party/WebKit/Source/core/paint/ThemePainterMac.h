/*
 * This file is part of the theme implementation for form controls in WebCore.
 *
 * Copyright (C) 2005 Apple Computer, Inc.
 * Copyright (C) 2008, 2009 Google, Inc.
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

#ifndef ThemePainterMac_h
#define ThemePainterMac_h

#import "core/paint/ThemePainter.h"

namespace blink {

class LayoutThemeMac;

class ThemePainterMac final : public ThemePainter {
 public:
  ThemePainterMac(LayoutThemeMac&);

 private:
  bool PaintButton(const LayoutObject&,
                   const PaintInfo&,
                   const IntRect&) override;
  bool PaintCheckbox(const LayoutObject&,
                     const PaintInfo&,
                     const IntRect&) override;
  bool PaintCapsLockIndicator(const LayoutObject&,
                              const PaintInfo&,
                              const IntRect&) override;
  bool PaintInnerSpinButton(const LayoutObject&,
                            const PaintInfo&,
                            const IntRect&) override;
  bool PaintMenuList(const LayoutObject&,
                     const PaintInfo&,
                     const IntRect&) override;
  bool PaintMenuListButton(const LayoutObject&,
                           const PaintInfo&,
                           const IntRect&) override;
  bool PaintProgressBar(const LayoutObject&,
                        const PaintInfo&,
                        const IntRect&) override;
  bool PaintRadio(const LayoutObject&,
                  const PaintInfo&,
                  const IntRect&) override;
  bool PaintSliderThumb(const LayoutObject&,
                        const PaintInfo&,
                        const IntRect&) override;
  bool PaintSliderTrack(const LayoutObject&,
                        const PaintInfo&,
                        const IntRect&) override;
  bool PaintSearchField(const LayoutObject&,
                        const PaintInfo&,
                        const IntRect&) override;
  bool PaintSearchFieldCancelButton(const LayoutObject&,
                                    const PaintInfo&,
                                    const IntRect&) override;
  bool PaintTextArea(const LayoutObject&,
                     const PaintInfo&,
                     const IntRect&) override;
  bool PaintTextField(const LayoutObject&,
                      const PaintInfo&,
                      const IntRect&) override;

  LayoutThemeMac& layout_theme_;
};

}  // namespace blink

#endif  // ThemePainterMac_h
