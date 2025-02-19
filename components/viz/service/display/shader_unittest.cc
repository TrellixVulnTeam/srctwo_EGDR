// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/viz/service/display/shader.h"

#include "cc/test/test_gles2_interface.h"
#include "cc/test/test_web_graphics_context_3d.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/gfx/geometry/point.h"
#include "ui/gfx/geometry/size.h"

namespace viz {

TEST(ShaderTest, HighpThresholds) {
  // The test context always uses a mediump precision of 10 bits which
  // corresponds to a native highp threshold of 2^10 = 1024
  std::unique_ptr<cc::TestWebGraphicsContext3D> test_context =
      cc::TestWebGraphicsContext3D::Create();
  cc::TestGLES2Interface test_gl;
  test_gl.set_test_context(test_context.get());

  int threshold_cache = 0;
  int threshold_min;
  gfx::Point closePoint(512, 512);
  gfx::Size smallSize(512, 512);
  gfx::Point farPoint(2560, 2560);
  gfx::Size bigSize(2560, 2560);

  threshold_min = 0;
  EXPECT_EQ(TEX_COORD_PRECISION_MEDIUM,
            TexCoordPrecisionRequired(&test_gl, &threshold_cache, threshold_min,
                                      closePoint));
  EXPECT_EQ(TEX_COORD_PRECISION_MEDIUM,
            TexCoordPrecisionRequired(&test_gl, &threshold_cache, threshold_min,
                                      smallSize));
  EXPECT_EQ(TEX_COORD_PRECISION_HIGH,
            TexCoordPrecisionRequired(&test_gl, &threshold_cache, threshold_min,
                                      farPoint));
  EXPECT_EQ(TEX_COORD_PRECISION_HIGH,
            TexCoordPrecisionRequired(&test_gl, &threshold_cache, threshold_min,
                                      bigSize));

  threshold_min = 3000;
  EXPECT_EQ(TEX_COORD_PRECISION_MEDIUM,
            TexCoordPrecisionRequired(&test_gl, &threshold_cache, threshold_min,
                                      closePoint));
  EXPECT_EQ(TEX_COORD_PRECISION_MEDIUM,
            TexCoordPrecisionRequired(&test_gl, &threshold_cache, threshold_min,
                                      smallSize));
  EXPECT_EQ(TEX_COORD_PRECISION_MEDIUM,
            TexCoordPrecisionRequired(&test_gl, &threshold_cache, threshold_min,
                                      farPoint));
  EXPECT_EQ(TEX_COORD_PRECISION_MEDIUM,
            TexCoordPrecisionRequired(&test_gl, &threshold_cache, threshold_min,
                                      bigSize));
}

}  // namespace viz
