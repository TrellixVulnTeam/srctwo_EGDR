// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/css/resolver/FontStyleResolver.h"
#include "core/css/threaded/MultiThreadedTestUtil.h"
#include "platform/Language.h"
#include "platform/fonts/Font.h"
#include "platform/fonts/FontCustomPlatformData.h"
#include "platform/fonts/FontDescription.h"
#include "platform/fonts/FontSelector.h"
#include "platform/fonts/shaping/CachingWordShapeIterator.h"
#include "platform/fonts/shaping/HarfBuzzShaper.h"
#include "platform/graphics/test/MockPaintCanvas.h"
#include "platform/testing/FontTestHelpers.h"
#include "platform/testing/UnitTestHelpers.h"
#include "platform/text/TextDirection.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using ::testing::_;
using ::testing::Return;

using blink::testing::CreateTestFont;

namespace blink {

TSAN_TEST(TextRendererThreadedTest, MeasureText) {
  RunOnThreads([]() {
    String text = "measure this";

    FontDescription font_description;
    font_description.SetComputedSize(12.0);
    font_description.SetLocale(LayoutLocale::Get("en"));
    ASSERT_EQ(USCRIPT_LATIN, font_description.GetScript());
    font_description.SetGenericFamily(FontDescription::kStandardFamily);

    Font font = Font(font_description);
    font.Update(nullptr);

    const SimpleFontData* font_data = font.PrimaryFont();
    ASSERT_TRUE(font_data);

    TextRun text_run(
        text, 0, 0,
        TextRun::kAllowTrailingExpansion | TextRun::kForbidLeadingExpansion,
        TextDirection::kLtr, false);
    text_run.SetNormalizeSpace(true);
    FloatRect text_bounds = font.SelectionRectForText(
        text_run, FloatPoint(), font.GetFontDescription().ComputedSize(), 0,
        -1);

    // X direction.
    EXPECT_FLOAT_EQ(77.7363, font.Width(text_run));
    EXPECT_EQ(0, text_bounds.X());
    EXPECT_EQ(78, text_bounds.MaxX());

    // Y direction.
    const FontMetrics& font_metrics = font_data->GetFontMetrics();
    EXPECT_EQ(11, font_metrics.FloatAscent());
    EXPECT_EQ(3, font_metrics.FloatDescent());
    EXPECT_EQ(0, text_bounds.Y());
    EXPECT_EQ(12, text_bounds.MaxY());
  });
}

TSAN_TEST(TextRendererThreadedTest, DrawText) {
  callbacks_per_thread_ = 50;
  RunOnThreads([]() {
    String text = "draw this";

    FontDescription font_description;
    font_description.SetComputedSize(12.0);
    font_description.SetLocale(LayoutLocale::Get("en"));
    ASSERT_EQ(USCRIPT_LATIN, font_description.GetScript());
    font_description.SetGenericFamily(FontDescription::kStandardFamily);

    Font font = Font(font_description);
    font.Update(nullptr);

    const SimpleFontData* font_data = font.PrimaryFont();
    ASSERT_TRUE(font_data);

    const FontMetrics& font_metrics = font_data->GetFontMetrics();

    FloatPoint location(0, 0);
    TextRun text_run(text, 0, 0, TextRun::kAllowTrailingExpansion,
                     TextDirection::kLtr, false);
    text_run.SetNormalizeSpace(true);
    float width = font.Width(text_run);

    TextRunPaintInfo text_run_paint_info(text_run);

    text_run_paint_info.bounds =
        FloatRect(location.X() - font_metrics.Height() / 2,
                  location.Y() - font_metrics.Ascent() - font_metrics.LineGap(),
                  width + font_metrics.Height(), font_metrics.LineSpacing());

    MockPaintCanvas mpc;
    PaintFlags flags;

    EXPECT_CALL(mpc, getSaveCount()).WillOnce(Return(17));
    EXPECT_CALL(mpc, drawTextBlob(_, 0, 0, _)).Times(1);
    EXPECT_CALL(mpc, restoreToCount(17)).WillOnce(Return());

    font.DrawBidiText(&mpc, text_run_paint_info, location,
                      Font::kUseFallbackIfFontNotReady, 1.0, flags);
  });
}

}  // namespace blink
