/*
 * Copyright (C) 2014 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "platform/transforms/TransformOperations.h"

#include "platform/geometry/FloatBox.h"
#include "platform/geometry/FloatBoxTestHelpers.h"
#include "platform/transforms/IdentityTransformOperation.h"
#include "platform/transforms/Matrix3DTransformOperation.h"
#include "platform/transforms/MatrixTransformOperation.h"
#include "platform/transforms/PerspectiveTransformOperation.h"
#include "platform/transforms/RotateTransformOperation.h"
#include "platform/transforms/ScaleTransformOperation.h"
#include "platform/transforms/SkewTransformOperation.h"
#include "platform/transforms/TranslateTransformOperation.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace blink {

static const TransformOperations kIdentityOperations;

static void EmpiricallyTestBounds(const TransformOperations& from,
                                  const TransformOperations& to,
                                  const double& min_progress,
                                  const double& max_progress) {
  FloatBox box(200, 500, 100, 100, 300, 200);
  FloatBox bounds;

  EXPECT_TRUE(
      to.BlendedBoundsForBox(box, from, min_progress, max_progress, &bounds));
  bool first_time = true;

  FloatBox empirical_bounds;
  static const size_t kNumSteps = 10;
  for (size_t step = 0; step < kNumSteps; ++step) {
    float t = step / (kNumSteps - 1);
    t = min_progress + (max_progress - min_progress) * t;
    TransformOperations operations = from.Blend(to, t);
    TransformationMatrix matrix;
    operations.Apply(FloatSize(0, 0), matrix);
    FloatBox transformed = box;
    matrix.TransformBox(transformed);

    if (first_time)
      empirical_bounds = transformed;
    else
      empirical_bounds.UnionBounds(transformed);
    first_time = false;
  }

  ASSERT_PRED_FORMAT2(FloatBoxTest::AssertContains, bounds, empirical_bounds);
}

TEST(TransformOperationsTest, AbsoluteAnimatedTranslatedBoundsTest) {
  TransformOperations from_ops;
  TransformOperations to_ops;
  from_ops.Operations().push_back(TranslateTransformOperation::Create(
      Length(-30, blink::kFixed), Length(20, blink::kFixed), 15,
      TransformOperation::kTranslate3D));
  to_ops.Operations().push_back(TranslateTransformOperation::Create(
      Length(10, blink::kFixed), Length(10, blink::kFixed), 200,
      TransformOperation::kTranslate3D));
  FloatBox box(0, 0, 0, 10, 10, 10);
  FloatBox bounds;

  EXPECT_TRUE(
      to_ops.BlendedBoundsForBox(box, kIdentityOperations, 0, 1, &bounds));
  EXPECT_PRED_FORMAT2(FloatBoxTest::AssertAlmostEqual,
                      FloatBox(0, 0, 0, 20, 20, 210), bounds);

  EXPECT_TRUE(
      kIdentityOperations.BlendedBoundsForBox(box, to_ops, 0, 1, &bounds));
  EXPECT_PRED_FORMAT2(FloatBoxTest::AssertAlmostEqual,
                      FloatBox(0, 0, 0, 20, 20, 210), bounds);

  EXPECT_TRUE(
      kIdentityOperations.BlendedBoundsForBox(box, from_ops, 0, 1, &bounds));
  EXPECT_PRED_FORMAT2(FloatBoxTest::AssertAlmostEqual,
                      FloatBox(-30, 0, 0, 40, 30, 25), bounds);

  EXPECT_TRUE(to_ops.BlendedBoundsForBox(box, from_ops, 0, 1, &bounds));
  EXPECT_PRED_FORMAT2(FloatBoxTest::AssertAlmostEqual,
                      FloatBox(-30, 10, 15, 50, 20, 195), bounds);

  EXPECT_TRUE(to_ops.BlendedBoundsForBox(box, from_ops, -0.5, 1.25, &bounds));
  EXPECT_PRED_FORMAT2(FloatBoxTest::AssertAlmostEqual,
                      FloatBox(-50, 7.5, -77.5, 80, 27.5, 333.75), bounds);
}

TEST(TransformOperationsTest, EmpiricalAnimatedTranslatedBoundsTest) {
  float test_transforms[][2][3] = {{{0, 0, 0}, {10, 10, 0}},
                                   {{-100, 202.5, -32.6}, {43.2, 56.1, 89.75}},
                                   {{43.2, 56.1, 89.75}, {-100, 202.5, -32.6}}};

  // All progressions for animations start and end at 0, 1 respectively,
  // we can go outside of these bounds, but will always at least contain
  // [0,1].
  float progress[][2] = {{0, 1}, {-.25, 1.25}};

  for (size_t i = 0; i < WTF_ARRAY_LENGTH(test_transforms); ++i) {
    for (size_t j = 0; j < WTF_ARRAY_LENGTH(progress); ++j) {
      TransformOperations from_ops;
      TransformOperations to_ops;
      from_ops.Operations().push_back(TranslateTransformOperation::Create(
          Length(test_transforms[i][0][0], blink::kFixed),
          Length(test_transforms[i][0][1], blink::kFixed),
          test_transforms[i][0][2], TransformOperation::kTranslate3D));
      to_ops.Operations().push_back(TranslateTransformOperation::Create(
          Length(test_transforms[i][1][0], blink::kFixed),
          Length(test_transforms[i][1][1], blink::kFixed),
          test_transforms[i][1][2], TransformOperation::kTranslate3D));
      EmpiricallyTestBounds(from_ops, to_ops, progress[j][0], progress[j][1]);
    }
  }
}

TEST(TransformOperationsTest, AbsoluteAnimatedScaleBoundsTest) {
  TransformOperations from_ops;
  TransformOperations to_ops;
  from_ops.Operations().push_back(
      ScaleTransformOperation::Create(4, -3, TransformOperation::kScale));
  to_ops.Operations().push_back(
      ScaleTransformOperation::Create(5, 2, TransformOperation::kScale));

  FloatBox box(0, 0, 0, 10, 10, 10);
  FloatBox bounds;

  EXPECT_TRUE(
      to_ops.BlendedBoundsForBox(box, kIdentityOperations, 0, 1, &bounds));
  EXPECT_PRED_FORMAT2(FloatBoxTest::AssertAlmostEqual,
                      FloatBox(0, 0, 0, 50, 20, 10), bounds);

  EXPECT_TRUE(
      kIdentityOperations.BlendedBoundsForBox(box, to_ops, 0, 1, &bounds));
  EXPECT_PRED_FORMAT2(FloatBoxTest::AssertAlmostEqual,
                      FloatBox(0, 0, 0, 50, 20, 10), bounds);

  EXPECT_TRUE(
      kIdentityOperations.BlendedBoundsForBox(box, from_ops, 0, 1, &bounds));
  EXPECT_PRED_FORMAT2(FloatBoxTest::AssertAlmostEqual,
                      FloatBox(0, -30, 0, 40, 40, 10), bounds);

  EXPECT_TRUE(to_ops.BlendedBoundsForBox(box, from_ops, 0, 1, &bounds));
  EXPECT_PRED_FORMAT2(FloatBoxTest::AssertAlmostEqual,
                      FloatBox(0, -30, 0, 50, 50, 10), bounds);

  EXPECT_TRUE(to_ops.BlendedBoundsForBox(box, from_ops, -0.5, 1.25, &bounds));
  EXPECT_PRED_FORMAT2(FloatBoxTest::AssertAlmostEqual,
                      FloatBox(0, -55, 0, 52.5, 87.5, 10), bounds);
}

TEST(TransformOperationsTest, EmpiricalAnimatedScaleBoundsTest) {
  float test_transforms[][2][3] = {{{1, 1, 1}, {10, 10, -32}},
                                   {{1, 2, 5}, {-1, -2, -4}},
                                   {{0, 0, 0}, {1, 2, 3}},
                                   {{0, 0, 0}, {0, 0, 0}}};

  // All progressions for animations start and end at 0, 1 respectively,
  // we can go outside of these bounds, but will always at least contain
  // [0,1].
  float progress[][2] = {{0, 1}, {-.25f, 1.25f}};

  for (size_t i = 0; i < WTF_ARRAY_LENGTH(test_transforms); ++i) {
    for (size_t j = 0; j < WTF_ARRAY_LENGTH(progress); ++j) {
      TransformOperations from_ops;
      TransformOperations to_ops;
      from_ops.Operations().push_back(TranslateTransformOperation::Create(
          Length(test_transforms[i][0][0], blink::kFixed),
          Length(test_transforms[i][0][1], blink::kFixed),
          test_transforms[i][0][2], TransformOperation::kTranslate3D));
      to_ops.Operations().push_back(TranslateTransformOperation::Create(
          Length(test_transforms[i][1][0], blink::kFixed),
          Length(test_transforms[i][1][1], blink::kFixed),
          test_transforms[i][1][2], TransformOperation::kTranslate3D));
      EmpiricallyTestBounds(from_ops, to_ops, progress[j][0], progress[j][1]);
    }
  }
}

TEST(TransformOperationsTest, AbsoluteAnimatedRotationBounds) {
  TransformOperations from_ops;
  TransformOperations to_ops;
  from_ops.Operations().push_back(
      RotateTransformOperation::Create(0, TransformOperation::kRotate));
  to_ops.Operations().push_back(
      RotateTransformOperation::Create(360, TransformOperation::kRotate));
  float sqrt2 = sqrt(2.0f);
  FloatBox box(-sqrt2, -sqrt2, 0, sqrt2, sqrt2, 0);
  FloatBox bounds;

  // Since we're rotating 360 degrees, any box with dimensions between 0 and
  // 2 * sqrt(2) should give the same result.
  float sizes[] = {0, 0.1f, sqrt2, 2 * sqrt2};
  to_ops.BlendedBoundsForBox(box, from_ops, 0, 1, &bounds);
  for (size_t i = 0; i < WTF_ARRAY_LENGTH(sizes); ++i) {
    box.SetSize(FloatPoint3D(sizes[i], sizes[i], 0));

    EXPECT_TRUE(to_ops.BlendedBoundsForBox(box, from_ops, 0, 1, &bounds));
    EXPECT_PRED_FORMAT2(FloatBoxTest::AssertAlmostEqual,
                        FloatBox(-2, -2, 0, 4, 4, 0), bounds);
  }
}

TEST(TransformOperationsTest, AbsoluteAnimatedExtremeRotationBounds) {
  // If the normal is off-plane, we can have up to 6 exrema (min/max in each
  // dimension between) the endpoints of the arg. This makes sure we are
  // catching all 6.
  TransformOperations from_ops;
  TransformOperations to_ops;
  from_ops.Operations().push_back(RotateTransformOperation::Create(
      1, 1, 1, 30, TransformOperation::kRotate3D));
  to_ops.Operations().push_back(RotateTransformOperation::Create(
      1, 1, 1, 390, TransformOperation::kRotate3D));

  FloatBox box(1, 0, 0, 0, 0, 0);
  FloatBox bounds;
  float min = -1 / 3.0f;
  float max = 1;
  float size = max - min;
  EXPECT_TRUE(to_ops.BlendedBoundsForBox(box, from_ops, 0, 1, &bounds));
  EXPECT_PRED_FORMAT2(FloatBoxTest::AssertAlmostEqual,
                      FloatBox(min, min, min, size, size, size), bounds);
}

TEST(TransformOperationsTest, AbsoluteAnimatedAxisRotationBounds) {
  // We can handle rotations about a single axis. If the axes are different,
  // we revert to matrix interpolation for which inflated bounds cannot be
  // computed.
  TransformOperations from_ops;
  TransformOperations to_same;
  TransformOperations to_opposite;
  TransformOperations to_different;
  from_ops.Operations().push_back(RotateTransformOperation::Create(
      1, 1, 1, 30, TransformOperation::kRotate3D));
  to_same.Operations().push_back(RotateTransformOperation::Create(
      1, 1, 1, 390, TransformOperation::kRotate3D));
  to_opposite.Operations().push_back(RotateTransformOperation::Create(
      -1, -1, -1, 390, TransformOperation::kRotate3D));
  to_different.Operations().push_back(RotateTransformOperation::Create(
      1, 3, 1, 390, TransformOperation::kRotate3D));

  FloatBox box(1, 0, 0, 0, 0, 0);
  FloatBox bounds;
  EXPECT_TRUE(to_same.BlendedBoundsForBox(box, from_ops, 0, 1, &bounds));
  EXPECT_FALSE(to_opposite.BlendedBoundsForBox(box, from_ops, 0, 1, &bounds));
  EXPECT_FALSE(to_different.BlendedBoundsForBox(box, from_ops, 0, 1, &bounds));
}

TEST(TransformOperationsTest, AbsoluteAnimatedOnAxisRotationBounds) {
  // If we rotate a point that is on the axis of rotation, the box should not
  // change at all.
  TransformOperations from_ops;
  TransformOperations to_ops;
  from_ops.Operations().push_back(RotateTransformOperation::Create(
      1, 1, 1, 30, TransformOperation::kRotate3D));
  to_ops.Operations().push_back(RotateTransformOperation::Create(
      1, 1, 1, 390, TransformOperation::kRotate3D));

  FloatBox box(1, 1, 1, 0, 0, 0);
  FloatBox bounds;

  EXPECT_TRUE(to_ops.BlendedBoundsForBox(box, from_ops, 0, 1, &bounds));
  EXPECT_PRED_FORMAT2(FloatBoxTest::AssertAlmostEqual, box, bounds);
}

// This would have been best as anonymous structs, but |WTF_ARRAY_LENGTH|
// does not get along with anonymous structs once we support C++11
// WTF_ARRAY_LENGTH will automatically support anonymous structs.

struct ProblematicAxisTest {
  double x;
  double y;
  double z;
  FloatBox expected;
};

TEST(TransformOperationsTest, AbsoluteAnimatedProblematicAxisRotationBounds) {
  // Zeros in the components of the axis osf rotation turned out to be tricky to
  // deal with in practice. This function tests some potentially problematic
  // axes to ensure sane behavior.

  // Some common values used in the expected boxes.
  float dim1 = 0.292893f;
  float dim2 = sqrt(2.0f);
  float dim3 = 2 * dim2;

  ProblematicAxisTest tests[] = {
      {0, 0, 0, FloatBox(1, 1, 1, 0, 0, 0)},
      {1, 0, 0, FloatBox(1, -dim2, -dim2, 0, dim3, dim3)},
      {0, 1, 0, FloatBox(-dim2, 1, -dim2, dim3, 0, dim3)},
      {0, 0, 1, FloatBox(-dim2, -dim2, 1, dim3, dim3, 0)},
      {1, 1, 0, FloatBox(dim1, dim1, -1, dim2, dim2, 2)},
      {0, 1, 1, FloatBox(-1, dim1, dim1, 2, dim2, dim2)},
      {1, 0, 1, FloatBox(dim1, -1, dim1, dim2, 2, dim2)}};

  for (size_t i = 0; i < WTF_ARRAY_LENGTH(tests); ++i) {
    float x = tests[i].x;
    float y = tests[i].y;
    float z = tests[i].z;
    TransformOperations from_ops;
    from_ops.Operations().push_back(RotateTransformOperation::Create(
        x, y, z, 0, TransformOperation::kRotate3D));
    TransformOperations to_ops;
    to_ops.Operations().push_back(RotateTransformOperation::Create(
        x, y, z, 360, TransformOperation::kRotate3D));
    FloatBox box(1, 1, 1, 0, 0, 0);
    FloatBox bounds;

    EXPECT_TRUE(to_ops.BlendedBoundsForBox(box, from_ops, 0, 1, &bounds));
    EXPECT_PRED_FORMAT2(FloatBoxTest::AssertAlmostEqual, tests[i].expected,
                        bounds);
  }
}

TEST(TransformOperationsTest, BlendedBoundsForRotationEmpiricalTests) {
  float axes[][3] = {{1, 1, 1},  {-1, -1, -1}, {-1, 2, 3},  {1, -2, 3},
                     {0, 0, 0},  {1, 0, 0},    {0, 1, 0},   {0, 0, 1},
                     {1, 1, 0},  {0, 1, 1},    {1, 0, 1},   {-1, 0, 0},
                     {0, -1, 0}, {0, 0, -1},   {-1, -1, 0}, {0, -1, -1},
                     {-1, 0, -1}};

  float angles[][2] = {{5, 100},     {10, 5},       {0, 360},   {20, 180},
                       {-20, -180},  {180, -220},   {220, 320}, {1020, 1120},
                       {-3200, 120}, {-9000, -9050}};

  float progress[][2] = {{0, 1}, {-0.25f, 1.25f}};

  for (size_t i = 0; i < WTF_ARRAY_LENGTH(axes); ++i) {
    for (size_t j = 0; j < WTF_ARRAY_LENGTH(angles); ++j) {
      for (size_t k = 0; k < WTF_ARRAY_LENGTH(progress); ++k) {
        float x = axes[i][0];
        float y = axes[i][1];
        float z = axes[i][2];

        TransformOperations from_ops;
        TransformOperations to_ops;

        from_ops.Operations().push_back(RotateTransformOperation::Create(
            x, y, z, angles[j][0], TransformOperation::kRotate3D));
        to_ops.Operations().push_back(RotateTransformOperation::Create(
            x, y, z, angles[j][1], TransformOperation::kRotate3D));
        EmpiricallyTestBounds(from_ops, to_ops, progress[k][0], progress[k][1]);
      }
    }
  }
}

TEST(TransformOperationsTest, AbsoluteAnimatedPerspectiveBoundsTest) {
  TransformOperations from_ops;
  TransformOperations to_ops;
  from_ops.Operations().push_back(PerspectiveTransformOperation::Create(20));
  to_ops.Operations().push_back(PerspectiveTransformOperation::Create(40));
  FloatBox box(0, 0, 0, 10, 10, 10);
  FloatBox bounds;
  to_ops.BlendedBoundsForBox(box, from_ops, 0, 1, &bounds);
  EXPECT_PRED_FORMAT2(FloatBoxTest::AssertAlmostEqual,
                      FloatBox(0, 0, 0, 20, 20, 20), bounds);

  from_ops.BlendedBoundsForBox(box, to_ops, -0.25, 1.25, &bounds);
  // The perspective range was [20, 40] and blending will extrapolate that to
  // [17, 53].  The cube has w/h/d of 10 and the observer is at 17, so the face
  // closest the observer is 17-10=7.
  double projected_size = 10.0 / 7.0 * 17.0;
  EXPECT_PRED_FORMAT2(
      FloatBoxTest::AssertAlmostEqual,
      FloatBox(0, 0, 0, projected_size, projected_size, projected_size),
      bounds);
}

TEST(TransformOperationsTest, EmpiricalAnimatedPerspectiveBoundsTest) {
  float depths[][2] = {
      {600, 400}, {800, 1000}, {800, std::numeric_limits<float>::infinity()}};

  float progress[][2] = {{0, 1}, {-0.1f, 1.1f}};

  for (size_t i = 0; i < WTF_ARRAY_LENGTH(depths); ++i) {
    for (size_t j = 0; j < WTF_ARRAY_LENGTH(progress); ++j) {
      TransformOperations from_ops;
      TransformOperations to_ops;

      from_ops.Operations().push_back(
          PerspectiveTransformOperation::Create(depths[i][0]));
      to_ops.Operations().push_back(
          PerspectiveTransformOperation::Create(depths[i][1]));

      EmpiricallyTestBounds(from_ops, to_ops, progress[j][0], progress[j][1]);
    }
  }
}

TEST(TransformOperationsTest, AnimatedSkewBoundsTest) {
  TransformOperations from_ops;
  TransformOperations to_ops;
  from_ops.Operations().push_back(
      SkewTransformOperation::Create(-45, 0, TransformOperation::kSkew));
  to_ops.Operations().push_back(
      SkewTransformOperation::Create(0, 45, TransformOperation::kSkew));
  FloatBox box(0, 0, 0, 10, 10, 10);
  FloatBox bounds;

  to_ops.BlendedBoundsForBox(box, kIdentityOperations, 0, 1, &bounds);
  ASSERT_PRED_FORMAT2(FloatBoxTest::AssertAlmostEqual,
                      FloatBox(0, 0, 0, 10, 20, 10), bounds);

  kIdentityOperations.BlendedBoundsForBox(box, from_ops, 0, 1, &bounds);
  ASSERT_PRED_FORMAT2(FloatBoxTest::AssertAlmostEqual,
                      FloatBox(-10, 0, 0, 20, 10, 10), bounds);

  to_ops.BlendedBoundsForBox(box, from_ops, 0, 1, &bounds);
  ASSERT_PRED_FORMAT2(FloatBoxTest::AssertAlmostEqual,
                      FloatBox(-10, 0, 0, 20, 20, 10), bounds);

  from_ops.BlendedBoundsForBox(box, to_ops, 0, 1, &bounds);
  ASSERT_PRED_FORMAT2(FloatBoxTest::AssertAlmostEqual,
                      FloatBox(-10, 0, 0, 20, 20, 10), bounds);
}

TEST(TransformOperationsTest, NonCommutativeRotations) {
  TransformOperations from_ops;
  from_ops.Operations().push_back(RotateTransformOperation::Create(
      1, 0, 0, 0, TransformOperation::kRotate3D));
  from_ops.Operations().push_back(RotateTransformOperation::Create(
      0, 1, 0, 0, TransformOperation::kRotate3D));
  TransformOperations to_ops;
  to_ops.Operations().push_back(RotateTransformOperation::Create(
      1, 0, 0, 45, TransformOperation::kRotate3D));
  to_ops.Operations().push_back(RotateTransformOperation::Create(
      0, 1, 0, 135, TransformOperation::kRotate3D));

  FloatBox box(0, 0, 0, 1, 1, 1);
  FloatBox bounds;

  double min_progress = 0;
  double max_progress = 1;
  EXPECT_TRUE(to_ops.BlendedBoundsForBox(box, from_ops, min_progress,
                                         max_progress, &bounds));

  TransformOperations operations = to_ops.Blend(from_ops, max_progress);
  TransformationMatrix blended_transform;
  operations.Apply(FloatSize(0, 0), blended_transform);

  FloatPoint3D blended_point(0.9f, 0.9f, 0);
  blended_point = blended_transform.MapPoint(blended_point);
  FloatBox expanded_bounds = bounds;
  expanded_bounds.ExpandTo(blended_point);

  ASSERT_PRED_FORMAT2(FloatBoxTest::AssertAlmostEqual, bounds, expanded_bounds);
}

TEST(TransformOperationsTest, AbsoluteSequenceBoundsTest) {
  TransformOperations from_ops;
  TransformOperations to_ops;

  from_ops.Operations().push_back(
      TranslateTransformOperation::Create(Length(1, kFixed), Length(-5, kFixed),
                                          1, TransformOperation::kTranslate3D));
  from_ops.Operations().push_back(
      ScaleTransformOperation::Create(-1, 2, 3, TransformOperation::kScale3D));
  from_ops.Operations().push_back(TranslateTransformOperation::Create(
      Length(2, kFixed), Length(4, kFixed), -1,
      TransformOperation::kTranslate3D));

  to_ops.Operations().push_back(TranslateTransformOperation::Create(
      Length(13, kFixed), Length(-1, kFixed), 5,
      TransformOperation::kTranslate3D));
  to_ops.Operations().push_back(
      ScaleTransformOperation::Create(-3, -2, 5, TransformOperation::kScale3D));
  to_ops.Operations().push_back(
      TranslateTransformOperation::Create(Length(6, kFixed), Length(-2, kFixed),
                                          3, TransformOperation::kTranslate3D));

  FloatBox box(1, 2, 3, 4, 4, 4);
  FloatBox bounds;

  EXPECT_TRUE(to_ops.BlendedBoundsForBox(box, from_ops, -0.5, 1.5, &bounds));
  EXPECT_PRED_FORMAT2(FloatBoxTest::AssertAlmostEqual,
                      FloatBox(-57, -59, -1, 76, 112, 80), bounds);

  EXPECT_TRUE(to_ops.BlendedBoundsForBox(box, from_ops, 0, 1, &bounds));
  EXPECT_PRED_FORMAT2(FloatBoxTest::AssertAlmostEqual,
                      FloatBox(-32, -25, 7, 42, 44, 48), bounds);

  EXPECT_TRUE(
      to_ops.BlendedBoundsForBox(box, kIdentityOperations, 0, 1, &bounds));
  EXPECT_PRED_FORMAT2(FloatBoxTest::AssertAlmostEqual,
                      FloatBox(-33, -13, 3, 57, 19, 52), bounds);

  EXPECT_TRUE(
      kIdentityOperations.BlendedBoundsForBox(box, from_ops, 0, 1, &bounds));
  EXPECT_PRED_FORMAT2(FloatBoxTest::AssertAlmostEqual,
                      FloatBox(-7, -3, 2, 15, 23, 20), bounds);
}

TEST(TransformOperationsTest, ZoomTest) {
  double zoom_factor = 1.25;

  FloatPoint3D original_point(2, 3, 4);

  TransformOperations ops;
  ops.Operations().push_back(
      TranslateTransformOperation::Create(Length(1, kFixed), Length(2, kFixed),
                                          3, TransformOperation::kTranslate3D));
  ops.Operations().push_back(PerspectiveTransformOperation::Create(1234));
  ops.Operations().push_back(
      Matrix3DTransformOperation::Create(TransformationMatrix(
          1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16)));

  // Apply unzoomed ops to unzoomed units, then zoom in
  FloatPoint3D unzoomed_point = original_point;
  TransformOperations unzoomed_ops = ops;
  TransformationMatrix unzoomed_matrix;
  ops.Apply(FloatSize(0, 0), unzoomed_matrix);
  FloatPoint3D result1 = unzoomed_matrix.MapPoint(unzoomed_point);
  result1.Scale(zoom_factor, zoom_factor, zoom_factor);

  // Apply zoomed ops to zoomed units
  FloatPoint3D zoomed_point = original_point;
  zoomed_point.Scale(zoom_factor, zoom_factor, zoom_factor);
  TransformOperations zoomed_ops = ops.Zoom(zoom_factor);
  TransformationMatrix zoomed_matrix;
  zoomed_ops.Apply(FloatSize(0, 0), zoomed_matrix);
  FloatPoint3D result2 = zoomed_matrix.MapPoint(zoomed_point);

  EXPECT_EQ(result1, result2);
}

}  // namespace blink
