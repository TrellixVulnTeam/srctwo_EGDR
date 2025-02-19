// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media/gpu/android/video_frame_factory_impl.h"

#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace media {

using testing::NiceMock;
using testing::Return;

// The dimensions for specifying MockImage behavior.
enum ImageKind { kSurfaceTexture, kOverlay };
enum Phase { kInCodec, kInFrontBuffer, kInvalidated };
enum Expectation { kRenderToFrontBuffer, kRenderToBackBuffer, kNone };

// A mock image with the same interface as CodecImage.
struct MockImage {
  MockImage(ImageKind kind, Phase phase, Expectation expectation) {
    ON_CALL(*this, is_surface_texture_backed())
        .WillByDefault(Return(kind == kSurfaceTexture));

    ON_CALL(*this, was_rendered_to_front_buffer())
        .WillByDefault(Return(phase == kInFrontBuffer));

    if (expectation == kRenderToFrontBuffer) {
      EXPECT_CALL(*this, RenderToFrontBuffer())
          .WillOnce(Return(phase != kInvalidated));
    } else {
      EXPECT_CALL(*this, RenderToFrontBuffer()).Times(0);
    }

    if (expectation == kRenderToBackBuffer) {
      EXPECT_CALL(*this, RenderToSurfaceTextureBackBuffer())
          .WillOnce(Return(phase != kInvalidated));
    } else {
      EXPECT_CALL(*this, RenderToSurfaceTextureBackBuffer()).Times(0);
    }
  }

  MOCK_METHOD0(was_rendered_to_front_buffer, bool());
  MOCK_METHOD0(is_surface_texture_backed, bool());
  MOCK_METHOD0(RenderToFrontBuffer, bool());
  MOCK_METHOD0(RenderToSurfaceTextureBackBuffer, bool());
};

class MaybeRenderEarlyTest : public testing::Test {
 public:
  MaybeRenderEarlyTest() = default;
  ~MaybeRenderEarlyTest() override = default;

  void AddImage(ImageKind kind, Phase phase, Expectation expectation) {
    owned_images_.push_back(
        base::MakeUnique<NiceMock<MockImage>>(kind, phase, expectation));
    images_.push_back(owned_images_.back().get());
  }

  std::vector<std::unique_ptr<NiceMock<MockImage>>> owned_images_;
  std::vector<MockImage*> images_;
};

TEST_F(MaybeRenderEarlyTest, EmptyVector) {
  internal::MaybeRenderEarly(&images_);
}

TEST_F(MaybeRenderEarlyTest, SingleUnrenderedSTImageIsRendered) {
  AddImage(kSurfaceTexture, kInCodec, Expectation::kRenderToFrontBuffer);
  internal::MaybeRenderEarly(&images_);
}

TEST_F(MaybeRenderEarlyTest, SingleUnrenderedOverlayImageIsRendered) {
  AddImage(kOverlay, kInCodec, Expectation::kRenderToFrontBuffer);
  internal::MaybeRenderEarly(&images_);
}

TEST_F(MaybeRenderEarlyTest, InvalidatedImagesAreSkippedOver) {
  AddImage(kSurfaceTexture, kInvalidated, Expectation::kRenderToFrontBuffer);
  AddImage(kSurfaceTexture, kInvalidated, Expectation::kRenderToFrontBuffer);
  AddImage(kSurfaceTexture, kInCodec, Expectation::kRenderToFrontBuffer);
  internal::MaybeRenderEarly(&images_);
}

// This also serves as a test that Overlay images are not back buffer rendered.
TEST_F(MaybeRenderEarlyTest, NoFrontBufferRenderingIfAlreadyPopulated) {
  AddImage(kOverlay, kInFrontBuffer, Expectation::kNone);
  AddImage(kOverlay, kInCodec, Expectation::kNone);
  internal::MaybeRenderEarly(&images_);
}

TEST_F(MaybeRenderEarlyTest,
       ImageFollowingLatestFrontBufferIsBackBufferRendered) {
  AddImage(kSurfaceTexture, kInCodec, Expectation::kNone);
  AddImage(kSurfaceTexture, kInFrontBuffer, Expectation::kNone);
  AddImage(kSurfaceTexture, kInCodec, Expectation::kRenderToBackBuffer);
  AddImage(kSurfaceTexture, kInCodec, Expectation::kNone);
  internal::MaybeRenderEarly(&images_);
}

}  // namespace media
