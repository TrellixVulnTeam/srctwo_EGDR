// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include <vector>

#include "base/bind.h"
#include "base/message_loop/message_loop.h"
#include "base/run_loop.h"
#include "base/test/simple_test_tick_clock.h"
#include "build/build_config.h"
#include "media/base/decoder_buffer.h"
#include "media/base/limits.h"
#include "media/base/test_data_util.h"
#include "media/base/test_helpers.h"
#include "media/base/video_frame.h"
#include "media/ffmpeg/ffmpeg_common.h"
#include "media/filters/in_memory_url_protocol.h"
#include "media/filters/vpx_video_decoder.h"
#include "testing/gmock/include/gmock/gmock.h"

using ::testing::_;

namespace media {

class VpxVideoDecoderTest : public testing::Test {
 public:
  VpxVideoDecoderTest()
      : decoder_(new VpxVideoDecoder()),
        i_frame_buffer_(ReadTestDataFile("vp9-I-frame-320x240")) {}

  ~VpxVideoDecoderTest() override { Destroy(); }

  void Initialize() {
    InitializeWithConfig(TestVideoConfig::Normal(kCodecVP9));
  }

  void InitializeWithConfigWithResult(const VideoDecoderConfig& config,
                                      bool success) {
    decoder_->Initialize(
        config, false, nullptr, NewExpectedBoolCB(success),
        base::Bind(&VpxVideoDecoderTest::FrameReady, base::Unretained(this)));
    decoder_->SetTickClockForTesting(&test_clock_);
    base::RunLoop().RunUntilIdle();
  }

  void InitializeWithConfig(const VideoDecoderConfig& config) {
    InitializeWithConfigWithResult(config, true);
  }

  void Reinitialize() {
    InitializeWithConfig(TestVideoConfig::Large(kCodecVP9));
  }

  void Reset() {
    decoder_->Reset(NewExpectedClosure());
    base::RunLoop().RunUntilIdle();
  }

  void Destroy() {
    decoder_.reset();
    base::RunLoop().RunUntilIdle();
  }

  // Sets up expectations and actions to put VpxVideoDecoder in an active
  // decoding state.
  void ExpectDecodingState() {
    EXPECT_EQ(DecodeStatus::OK, DecodeSingleFrame(i_frame_buffer_));
    ASSERT_EQ(1U, output_frames_.size());
  }

  // Sets up expectations and actions to put VpxVideoDecoder in an end
  // of stream state.
  void ExpectEndOfStreamState() {
    EXPECT_EQ(DecodeStatus::OK,
              DecodeSingleFrame(DecoderBuffer::CreateEOSBuffer()));
    ASSERT_FALSE(output_frames_.empty());
  }

  using InputBuffers = std::vector<scoped_refptr<DecoderBuffer>>;
  using OutputFrames = std::vector<scoped_refptr<VideoFrame>>;

  // Decodes all buffers in |input_buffers| and push all successfully decoded
  // output frames into |output_frames|.
  // Returns the last decode status returned by the decoder.
  DecodeStatus DecodeMultipleFrames(const InputBuffers& input_buffers) {
    for (InputBuffers::const_iterator iter = input_buffers.begin();
         iter != input_buffers.end(); ++iter) {
      DecodeStatus status = Decode(*iter);
      switch (status) {
        case DecodeStatus::OK:
          break;
        case DecodeStatus::ABORTED:
          NOTREACHED();
        case DecodeStatus::DECODE_ERROR:
          DCHECK(output_frames_.empty());
          return status;
      }
    }
    return DecodeStatus::OK;
  }

  // Decodes the single compressed frame in |buffer| and writes the
  // uncompressed output to |video_frame|. This method works with single
  // and multithreaded decoders. End of stream buffers are used to trigger
  // the frame to be returned in the multithreaded decoder case.
  DecodeStatus DecodeSingleFrame(scoped_refptr<DecoderBuffer> buffer) {
    InputBuffers input_buffers;
    input_buffers.push_back(std::move(buffer));
    input_buffers.push_back(DecoderBuffer::CreateEOSBuffer());

    return DecodeMultipleFrames(input_buffers);
  }

  // Decodes |i_frame_buffer_| and then decodes the data contained in
  // the file named |test_file_name|. This function expects both buffers
  // to decode to frames that are the same size.
  void DecodeIFrameThenTestFile(const std::string& test_file_name,
                                const gfx::Size& expected_size) {
    Initialize();
    scoped_refptr<DecoderBuffer> buffer = ReadTestDataFile(test_file_name);

    InputBuffers input_buffers;
    input_buffers.push_back(i_frame_buffer_);
    input_buffers.push_back(buffer);
    input_buffers.push_back(DecoderBuffer::CreateEOSBuffer());

    DecodeStatus status = DecodeMultipleFrames(input_buffers);

    EXPECT_EQ(DecodeStatus::OK, status);
    ASSERT_EQ(2U, output_frames_.size());

    gfx::Size original_size = TestVideoConfig::NormalCodedSize();
    EXPECT_EQ(original_size.width(),
              output_frames_[0]->visible_rect().size().width());
    EXPECT_EQ(original_size.height(),
              output_frames_[0]->visible_rect().size().height());
    EXPECT_EQ(expected_size.width(),
              output_frames_[1]->visible_rect().size().width());
    EXPECT_EQ(expected_size.height(),
              output_frames_[1]->visible_rect().size().height());
  }

  DecodeStatus Decode(scoped_refptr<DecoderBuffer> buffer) {
    DecodeStatus status;
    EXPECT_CALL(*this, DecodeDone(_)).WillOnce(testing::SaveArg<0>(&status));

    decoder_->Decode(
        std::move(buffer),
        base::Bind(&VpxVideoDecoderTest::DecodeDone, base::Unretained(this)));
    base::RunLoop().RunUntilIdle();

    return status;
  }

  void FrameReady(const scoped_refptr<VideoFrame>& frame) {
    DCHECK(!frame->metadata()->IsTrue(VideoFrameMetadata::END_OF_STREAM));
    output_frames_.push_back(frame);
  }

  MOCK_METHOD1(DecodeDone, void(DecodeStatus));

  base::MessageLoop message_loop_;
  base::SimpleTestTickClock test_clock_;
  std::unique_ptr<VpxVideoDecoder> decoder_;

  scoped_refptr<DecoderBuffer> i_frame_buffer_;
  OutputFrames output_frames_;

 private:
  DISALLOW_COPY_AND_ASSIGN(VpxVideoDecoderTest);
};

TEST_F(VpxVideoDecoderTest, Initialize_Normal) {
  Initialize();
}

TEST_F(VpxVideoDecoderTest, Reinitialize_Normal) {
  Initialize();
  Reinitialize();
}

TEST_F(VpxVideoDecoderTest, Reinitialize_AfterDecodeFrame) {
  Initialize();
  ExpectDecodingState();
  Reinitialize();
}

TEST_F(VpxVideoDecoderTest, Reinitialize_AfterReset) {
  Initialize();
  ExpectDecodingState();
  Reset();
  Reinitialize();
}

TEST_F(VpxVideoDecoderTest, DecodeFrame_Normal) {
  Initialize();

  // Simulate decoding a single frame.
  EXPECT_EQ(DecodeStatus::OK, DecodeSingleFrame(i_frame_buffer_));
  ASSERT_EQ(1U, output_frames_.size());
}

// Decode |i_frame_buffer_| and then a frame with a larger width and verify
// the output size was adjusted.
TEST_F(VpxVideoDecoderTest, DecodeFrame_LargerWidth) {
  DecodeIFrameThenTestFile("vp9-I-frame-1280x720", gfx::Size(1280, 720));
}

// Test resetting when decoder has initialized but not decoded.
TEST_F(VpxVideoDecoderTest, Reset_Initialized) {
  Initialize();
  Reset();
}

// Test resetting when decoder has decoded single frame.
TEST_F(VpxVideoDecoderTest, Reset_Decoding) {
  Initialize();
  ExpectDecodingState();
  Reset();
}

// Test resetting when decoder has hit end of stream.
TEST_F(VpxVideoDecoderTest, Reset_EndOfStream) {
  Initialize();
  ExpectDecodingState();
  ExpectEndOfStreamState();
  Reset();
}

// Test destruction when decoder has initialized but not decoded.
TEST_F(VpxVideoDecoderTest, Destroy_Initialized) {
  Initialize();
  Destroy();
}

// Test destruction when decoder has decoded single frame.
TEST_F(VpxVideoDecoderTest, Destroy_Decoding) {
  Initialize();
  ExpectDecodingState();
  Destroy();
}

// Test destruction when decoder has hit end of stream.
TEST_F(VpxVideoDecoderTest, Destroy_EndOfStream) {
  Initialize();
  ExpectDecodingState();
  ExpectEndOfStreamState();
  Destroy();
}

TEST_F(VpxVideoDecoderTest, SimpleFrameReuse) {
  Initialize();
  Decode(i_frame_buffer_);

  ASSERT_EQ(1u, output_frames_.size());
  scoped_refptr<VideoFrame> frame = std::move(output_frames_.front());
  const uint8_t* old_y_data = frame->data(VideoFrame::kYPlane);
  output_frames_.pop_back();

  // Clear frame reference to return the frame to the pool.
  frame = nullptr;
  EXPECT_EQ(1u, decoder_->GetPoolSizeForTesting());

  // Since we're decoding I-frames which are marked as having dependent frames,
  // libvpx will still have a ref on the previous buffer. So verify we see an
  // increase to two frames.
  Decode(i_frame_buffer_);
  EXPECT_EQ(2u, decoder_->GetPoolSizeForTesting());
  EXPECT_NE(old_y_data, output_frames_.front()->data(VideoFrame::kYPlane));

  // Issuing another decode should reuse the first buffer now that the refs have
  // been dropped by the previous decode.
  Decode(i_frame_buffer_);
  EXPECT_EQ(2u, decoder_->GetPoolSizeForTesting());

  ASSERT_EQ(2u, output_frames_.size());
  EXPECT_EQ(old_y_data, output_frames_.back()->data(VideoFrame::kYPlane));
}

TEST_F(VpxVideoDecoderTest, SimpleFormatChange) {
  scoped_refptr<DecoderBuffer> large_frame =
      ReadTestDataFile("vp9-I-frame-1280x720");

  Initialize();
  Decode(i_frame_buffer_);
  Decode(i_frame_buffer_);
  EXPECT_EQ(2u, decoder_->GetPoolSizeForTesting());
  output_frames_.clear();
  base::RunLoop().RunUntilIdle();

  // Verify decoding a larger frame simply resizes one of the existing buffers.
  Decode(large_frame);
  EXPECT_EQ(2u, decoder_->GetPoolSizeForTesting());
}

TEST_F(VpxVideoDecoderTest, FrameValidAfterPoolDestruction) {
  Initialize();
  Decode(i_frame_buffer_);
  Destroy();

  // Write to the Y plane. The memory tools should detect a
  // use-after-free if the storage was actually removed by pool destruction.
  memset(output_frames_.front()->data(VideoFrame::kYPlane), 0xff,
         output_frames_.front()->rows(VideoFrame::kYPlane) *
             output_frames_.front()->stride(VideoFrame::kYPlane));
}

TEST_F(VpxVideoDecoderTest, StaleFramesAreExpired) {
  Initialize();
  Decode(i_frame_buffer_);
  Decode(i_frame_buffer_);
  Decode(i_frame_buffer_);
  EXPECT_EQ(3u, decoder_->GetPoolSizeForTesting());

  output_frames_.pop_back();
  output_frames_.pop_back();
  base::RunLoop().RunUntilIdle();
  EXPECT_EQ(3u, decoder_->GetPoolSizeForTesting());

  // Advance clock far enough to hit stale timer.
  test_clock_.Advance(base::TimeDelta::FromMinutes(1));
  output_frames_.clear();
  base::RunLoop().RunUntilIdle();

  // The last frame should still have a ref internal to libvpx and the frame we
  // just released isn't stale yet, so only 2 frames should remain.
  EXPECT_EQ(2u, decoder_->GetPoolSizeForTesting());
}

TEST_F(VpxVideoDecoderTest, MemoryPoolAllowsMultipleDisplay) {
  // Initialize with dummy data, we could read it from the test clip, but it's
  // not necessary for this test.
  Initialize();

  scoped_refptr<DecoderBuffer> data =
      ReadTestDataFile("vp9-duplicate-frame.webm");
  InMemoryUrlProtocol protocol(data->data(), data->data_size(), false);
  FFmpegGlue glue(&protocol);
  ASSERT_TRUE(glue.OpenContext());

  AVPacket packet;
  while (av_read_frame(glue.format_context(), &packet) >= 0) {
    if (Decode(DecoderBuffer::CopyFrom(packet.data, packet.size)) !=
        DecodeStatus::OK) {
      av_packet_unref(&packet);
      break;
    }
    av_packet_unref(&packet);
  }

  // Android returns 25 frames while other platforms return 26 for some reason;
  // we don't really care about the exact number.
  ASSERT_GE(output_frames_.size(), 25u);

  scoped_refptr<VideoFrame> last_frame = output_frames_.back();

  // Duplicate frame is actually two before the last in this bitstream.
  scoped_refptr<VideoFrame> dupe_frame =
      output_frames_[output_frames_.size() - 3];

#if !defined(OS_ANDROID)
  // Android doesn't seem to expose this bug, but the rest of the test is still
  // reasonable to complete even on Android.
  EXPECT_EQ(last_frame->data(VideoFrame::kYPlane),
            dupe_frame->data(VideoFrame::kYPlane));
  EXPECT_EQ(last_frame->data(VideoFrame::kUPlane),
            dupe_frame->data(VideoFrame::kUPlane));
  EXPECT_EQ(last_frame->data(VideoFrame::kVPlane),
            dupe_frame->data(VideoFrame::kVPlane));
#endif

  // This will release all frames held by the memory pool, but should not
  // release |last_frame| since we still have a ref despite sharing the same
  // memory as |dupe_frame|.
  output_frames_.clear();
  dupe_frame = nullptr;
  Destroy();

  // ASAN will be very unhappy with this line if the above is incorrect.
  memset(last_frame->data(VideoFrame::kYPlane), 0,
         last_frame->row_bytes(VideoFrame::kYPlane));
}

}  // namespace media
