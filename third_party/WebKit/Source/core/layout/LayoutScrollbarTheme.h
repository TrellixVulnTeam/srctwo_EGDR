/*
 * Copyright (C) 2008 Apple Inc. All Rights Reserved.
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

#ifndef LayoutScrollbarTheme_h
#define LayoutScrollbarTheme_h

#include "platform/scroll/ScrollbarTheme.h"

namespace blink {

class WebMouseEvent;

class LayoutScrollbarTheme final : public ScrollbarTheme {
 public:
  ~LayoutScrollbarTheme() override {}

  int ScrollbarThickness(ScrollbarControlSize control_size) override {
    return ScrollbarTheme::GetTheme().ScrollbarThickness(control_size);
  }

  WebScrollbarButtonsPlacement ButtonsPlacement() const override {
    return ScrollbarTheme::GetTheme().ButtonsPlacement();
  }

  void PaintScrollCorner(GraphicsContext&,
                         const DisplayItemClient&,
                         const IntRect& corner_rect) override;

  bool ShouldCenterOnThumb(const ScrollbarThemeClient& scrollbar,
                           const WebMouseEvent& event) override {
    return ScrollbarTheme::GetTheme().ShouldCenterOnThumb(scrollbar, event);
  }
  bool ShouldSnapBackToDragOrigin(const ScrollbarThemeClient& scrollbar,
                                  const WebMouseEvent& event) override {
    return ScrollbarTheme::GetTheme().ShouldSnapBackToDragOrigin(scrollbar,
                                                                 event);
  }

  double InitialAutoscrollTimerDelay() override {
    return ScrollbarTheme::GetTheme().InitialAutoscrollTimerDelay();
  }
  double AutoscrollTimerDelay() override {
    return ScrollbarTheme::GetTheme().AutoscrollTimerDelay();
  }

  void RegisterScrollbar(ScrollbarThemeClient& scrollbar) override {
    return ScrollbarTheme::GetTheme().RegisterScrollbar(scrollbar);
  }
  void UnregisterScrollbar(ScrollbarThemeClient& scrollbar) override {
    return ScrollbarTheme::GetTheme().UnregisterScrollbar(scrollbar);
  }

  int MinimumThumbLength(const ScrollbarThemeClient&) override;

  void ButtonSizesAlongTrackAxis(const ScrollbarThemeClient&,
                                 int& before_size,
                                 int& after_size);

  static LayoutScrollbarTheme* GetLayoutScrollbarTheme();

 protected:
  bool HasButtons(const ScrollbarThemeClient&) override;
  bool HasThumb(const ScrollbarThemeClient&) override;

  IntRect BackButtonRect(const ScrollbarThemeClient&,
                         ScrollbarPart,
                         bool painting = false) override;
  IntRect ForwardButtonRect(const ScrollbarThemeClient&,
                            ScrollbarPart,
                            bool painting = false) override;
  IntRect TrackRect(const ScrollbarThemeClient&,
                    bool painting = false) override;

  void PaintScrollbarBackground(GraphicsContext&, const Scrollbar&) override;
  void PaintTrackBackground(GraphicsContext&,
                            const Scrollbar&,
                            const IntRect&) override;
  void PaintTrackPiece(GraphicsContext&,
                       const Scrollbar&,
                       const IntRect&,
                       ScrollbarPart) override;
  void PaintButton(GraphicsContext&,
                   const Scrollbar&,
                   const IntRect&,
                   ScrollbarPart) override;
  void PaintThumb(GraphicsContext&, const Scrollbar&, const IntRect&) override;
  void PaintTickmarks(GraphicsContext&,
                      const Scrollbar&,
                      const IntRect&) override;

  IntRect ConstrainTrackRectToTrackPieces(const ScrollbarThemeClient&,
                                          const IntRect&) override;
};

}  // namespace blink

#endif  // LayoutScrollbarTheme_h
