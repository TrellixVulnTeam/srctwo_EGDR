// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/html/canvas/CanvasFontCache.h"

#include <memory>
#include "core/dom/Document.h"
#include "core/frame/LocalFrameView.h"
#include "core/html/canvas/CanvasContextCreationAttributes.h"
#include "core/html/canvas/CanvasRenderingContext.h"
#include "core/loader/EmptyClients.h"
#include "core/testing/DummyPageHolder.h"
#include "platform/graphics/UnacceleratedImageBufferSurface.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using ::testing::Mock;

namespace blink {

class CanvasFontCacheTest : public ::testing::Test {
 protected:
  CanvasFontCacheTest();
  void SetUp() override;

  DummyPageHolder& Page() const { return *dummy_page_holder_; }
  Document& GetDocument() const { return *document_; }
  HTMLCanvasElement& CanvasElement() const { return *canvas_element_; }
  CanvasRenderingContext* Context2d() const;
  CanvasFontCache* Cache() { return document_->GetCanvasFontCache(); }

 private:
  std::unique_ptr<DummyPageHolder> dummy_page_holder_;
  Persistent<Document> document_;
  Persistent<HTMLCanvasElement> canvas_element_;
};

CanvasFontCacheTest::CanvasFontCacheTest() {}

CanvasRenderingContext* CanvasFontCacheTest::Context2d() const {
  // If the following check fails, perhaps you forgot to call createContext
  // in your test?
  EXPECT_NE(nullptr, CanvasElement().RenderingContext());
  EXPECT_TRUE(CanvasElement().RenderingContext()->Is2d());
  return CanvasElement().RenderingContext();
}

void CanvasFontCacheTest::SetUp() {
  Page::PageClients page_clients;
  FillWithEmptyClients(page_clients);
  dummy_page_holder_ =
      DummyPageHolder::Create(IntSize(800, 600), &page_clients);
  document_ = &dummy_page_holder_->GetDocument();
  document_->documentElement()->setInnerHTML(
      "<body><canvas id='c'></canvas></body>");
  document_->View()->UpdateAllLifecyclePhases();
  canvas_element_ = toHTMLCanvasElement(document_->getElementById("c"));
  String canvas_type("2d");
  CanvasContextCreationAttributes attributes;
  attributes.setAlpha(true);
  canvas_element_->GetCanvasRenderingContext(canvas_type, attributes);
  Context2d();  // Calling this for the checks
}

TEST_F(CanvasFontCacheTest, CacheHardLimit) {
  String font_string;
  unsigned i;
  for (i = 0; i < Cache()->HardMaxFonts() + 1; i++) {
    font_string = String::Number(i + 1) + "px sans-serif";
    Context2d()->setFont(font_string);
    if (i < Cache()->HardMaxFonts()) {
      EXPECT_TRUE(Cache()->IsInCache("1px sans-serif"));
    } else {
      EXPECT_FALSE(Cache()->IsInCache("1px sans-serif"));
    }
    EXPECT_TRUE(Cache()->IsInCache(font_string));
  }
}

TEST_F(CanvasFontCacheTest, PageVisibilityChange) {
  Context2d()->setFont("10px sans-serif");
  EXPECT_TRUE(Cache()->IsInCache("10px sans-serif"));
  Page().GetPage().SetVisibilityState(kPageVisibilityStateHidden, false);
  EXPECT_FALSE(Cache()->IsInCache("10px sans-serif"));

  Context2d()->setFont("15px sans-serif");
  EXPECT_FALSE(Cache()->IsInCache("10px sans-serif"));
  EXPECT_TRUE(Cache()->IsInCache("15px sans-serif"));

  Context2d()->setFont("10px sans-serif");
  EXPECT_TRUE(Cache()->IsInCache("10px sans-serif"));
  EXPECT_FALSE(Cache()->IsInCache("15px sans-serif"));

  Page().GetPage().SetVisibilityState(kPageVisibilityStateVisible, false);
  Context2d()->setFont("15px sans-serif");
  Context2d()->setFont("10px sans-serif");
  EXPECT_TRUE(Cache()->IsInCache("10px sans-serif"));
  EXPECT_TRUE(Cache()->IsInCache("15px sans-serif"));
}

}  // namespace blink
