// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ValidationMessageOverlayDelegate_h
#define ValidationMessageOverlayDelegate_h

#include "core/page/PageOverlay.h"
#include "platform/text/TextDirection.h"
#include "platform/wtf/Forward.h"

namespace blink {

class ChromeClient;
class Element;
class LocalFrameView;
class Page;

// A ValidationMessageOverlayDelegate is responsible for rendering a form
// validation message bubble.
//
// Lifetime: An instance is created by a ValidationMessageClientImpl when a
// bubble is shown, and deleted when the bubble is closed.
//
// Ownership: A PageOverlay instance owns a ValidationMessageOverlayDelegate.
class ValidationMessageOverlayDelegate : public PageOverlay::Delegate {
 public:
  static std::unique_ptr<ValidationMessageOverlayDelegate> Create(
      Page&,
      const Element& anchor,
      const String& message,
      TextDirection message_dir,
      const String& sub_message,
      TextDirection sub_message_dir);
  ~ValidationMessageOverlayDelegate() override;

  void PaintPageOverlay(const PageOverlay&,
                        GraphicsContext&,
                        const WebSize& view_size) const override;
  void StartToHide();

 private:
  ValidationMessageOverlayDelegate(Page&,
                                   const Element& anchor,
                                   const String& message,
                                   TextDirection message_dir,
                                   const String& sub_message,
                                   TextDirection sub_message_dir);
  LocalFrameView& FrameView() const;
  void UpdateFrameViewState(const PageOverlay&, const IntSize& view_size);
  void EnsurePage(const PageOverlay&, const IntSize& view_size);
  void WriteDocument(SharedBuffer*);
  Element& GetElementById(const AtomicString&) const;
  void AdjustBubblePosition(const IntSize& view_size);
  bool IsHiding() const;

  // An internal Page and a ChromeClient for it.
  Persistent<Page> page_;
  Persistent<ChromeClient> chrome_client_;

  IntSize bubble_size_;

  // A page which triggered this validation message.
  Persistent<Page> main_page_;

  Persistent<const Element> anchor_;
  String message_;
  String sub_message_;
  TextDirection message_dir_;
  TextDirection sub_message_dir_;
};

}  // namespace blink
#endif  // ValidationMessageOverlayDelegate_h
