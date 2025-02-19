/*
 * Copyright (C) 2012 Google Inc. All rights reserved.
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

#include "platform/animation/AnimationTranslationUtil.h"

#include "platform/animation/CompositorTransformOperations.h"
#include "platform/transforms/Matrix3DTransformOperation.h"
#include "platform/transforms/RotateTransformOperation.h"
#include "platform/transforms/ScaleTransformOperation.h"
#include "platform/transforms/TransformOperations.h"
#include "platform/transforms/TranslateTransformOperation.h"
#include "testing/gtest/include/gtest/gtest.h"
#include <memory>

namespace blink {

TEST(AnimationTranslationUtilTest, transformsWork) {
  TransformOperations ops;
  CompositorTransformOperations out_ops;

  ops.Operations().push_back(TranslateTransformOperation::Create(
      Length(2, kFixed), Length(0, kFixed), TransformOperation::kTranslateX));
  ops.Operations().push_back(RotateTransformOperation::Create(
      0.1, 0.2, 0.3, 200000.4, TransformOperation::kRotate3D));
  ops.Operations().push_back(ScaleTransformOperation::Create(
      50.2, 100, -4, TransformOperation::kScale3D));
  ToCompositorTransformOperations(ops, &out_ops);

  EXPECT_EQ(3UL, out_ops.AsCcTransformOperations().size());
  const float kErr = 0.0001;

  auto& op0 = out_ops.AsCcTransformOperations().at(0);
  EXPECT_EQ(cc::TransformOperation::TRANSFORM_OPERATION_TRANSLATE, op0.type);
  EXPECT_NEAR(op0.translate.x, 2.0f, kErr);
  EXPECT_NEAR(op0.translate.y, 0.0f, kErr);
  EXPECT_NEAR(op0.translate.z, 0.0f, kErr);

  auto& op1 = out_ops.AsCcTransformOperations().at(1);
  EXPECT_EQ(cc::TransformOperation::TRANSFORM_OPERATION_ROTATE, op1.type);
  EXPECT_NEAR(op1.rotate.axis.x, 0.1f, kErr);
  EXPECT_NEAR(op1.rotate.axis.y, 0.2f, kErr);
  EXPECT_NEAR(op1.rotate.axis.z, 0.3f, kErr);
  EXPECT_NEAR(op1.rotate.angle, 200000.4f, 0.01f);

  auto& op2 = out_ops.AsCcTransformOperations().at(2);
  EXPECT_EQ(cc::TransformOperation::TRANSFORM_OPERATION_SCALE, op2.type);
  EXPECT_NEAR(op2.scale.x, 50.2f, kErr);
  EXPECT_NEAR(op2.scale.y, 100.0f, kErr);
  EXPECT_NEAR(op2.scale.z, -4.0f, kErr);
}

}  // namespace blink
