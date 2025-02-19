/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "platform/image-decoders/webp/WEBPImageDecoder.h"

#include "build/build_config.h"
#include "third_party/skia/include/core/SkData.h"

#if defined(ARCH_CPU_BIG_ENDIAN)
#error Blink assumes a little-endian target.
#endif

#if SK_B32_SHIFT  // Output little-endian RGBA pixels (Android).
inline WEBP_CSP_MODE outputMode(bool hasAlpha) {
  return hasAlpha ? MODE_rgbA : MODE_RGBA;
}
#else  // Output little-endian BGRA pixels.
inline WEBP_CSP_MODE outputMode(bool hasAlpha) {
  return hasAlpha ? MODE_bgrA : MODE_BGRA;
}
#endif

namespace {

// Returns two point ranges (<left, width> pairs) at row |canvasY| which belong
// to |src| but not |dst|. A range is empty if its width is 0.
inline void findBlendRangeAtRow(const blink::IntRect& src,
                                const blink::IntRect& dst,
                                int canvasY,
                                int& left1,
                                int& width1,
                                int& left2,
                                int& width2) {
  SECURITY_DCHECK(canvasY >= src.Y() && canvasY < src.MaxY());
  left1 = -1;
  width1 = 0;
  left2 = -1;
  width2 = 0;

  if (canvasY < dst.Y() || canvasY >= dst.MaxY() || src.X() >= dst.MaxX() ||
      src.MaxX() <= dst.X()) {
    left1 = src.X();
    width1 = src.Width();
    return;
  }

  if (src.X() < dst.X()) {
    left1 = src.X();
    width1 = dst.X() - src.X();
  }

  if (src.MaxX() > dst.MaxX()) {
    left2 = dst.MaxX();
    width2 = src.MaxX() - dst.MaxX();
  }
}

// alphaBlendPremultiplied and alphaBlendNonPremultiplied are separate methods,
// even though they only differ by one line. This is done so that the compiler
// can inline BlendSrcOverDstPremultiplied() and BlensSrcOverDstRaw() calls.
// For GIF images, this optimization reduces decoding time by 15% for 3MB
// images.
void alphaBlendPremultiplied(blink::ImageFrame& src,
                             blink::ImageFrame& dst,
                             int canvasY,
                             int left,
                             int width) {
  for (int x = 0; x < width; ++x) {
    int canvasX = left + x;
    blink::ImageFrame::PixelData* pixel = src.GetAddr(canvasX, canvasY);
    if (SkGetPackedA32(*pixel) != 0xff) {
      blink::ImageFrame::PixelData prevPixel = *dst.GetAddr(canvasX, canvasY);
      blink::ImageFrame::BlendSrcOverDstPremultiplied(pixel, prevPixel);
    }
  }
}

void alphaBlendNonPremultiplied(blink::ImageFrame& src,
                                blink::ImageFrame& dst,
                                int canvasY,
                                int left,
                                int width) {
  for (int x = 0; x < width; ++x) {
    int canvasX = left + x;
    blink::ImageFrame::PixelData* pixel = src.GetAddr(canvasX, canvasY);
    if (SkGetPackedA32(*pixel) != 0xff) {
      blink::ImageFrame::PixelData prevPixel = *dst.GetAddr(canvasX, canvasY);
      blink::ImageFrame::BlendSrcOverDstRaw(pixel, prevPixel);
    }
  }
}

}  // namespace

namespace blink {

WEBPImageDecoder::WEBPImageDecoder(AlphaOption alpha_option,
                                   const ColorBehavior& color_behavior,
                                   size_t max_decoded_bytes)
    : ImageDecoder(alpha_option, color_behavior, max_decoded_bytes),
      decoder_(0),
      format_flags_(0),
      frame_background_has_alpha_(false),
      demux_(0),
      demux_state_(WEBP_DEMUX_PARSING_HEADER),
      have_already_parsed_this_data_(false),
      repetition_count_(kAnimationLoopOnce),
      decoded_height_(0) {
  blend_function_ = (alpha_option == kAlphaPremultiplied)
                        ? alphaBlendPremultiplied
                        : alphaBlendNonPremultiplied;
}

WEBPImageDecoder::~WEBPImageDecoder() {
  Clear();
}

void WEBPImageDecoder::Clear() {
  WebPDemuxDelete(demux_);
  demux_ = 0;
  consolidated_data_.reset();
  ClearDecoder();
}

void WEBPImageDecoder::ClearDecoder() {
  WebPIDelete(decoder_);
  decoder_ = 0;
  decoded_height_ = 0;
  frame_background_has_alpha_ = false;
}

void WEBPImageDecoder::OnSetData(SegmentReader*) {
  have_already_parsed_this_data_ = false;
}

int WEBPImageDecoder::RepetitionCount() const {
  return Failed() ? kAnimationLoopOnce : repetition_count_;
}

bool WEBPImageDecoder::FrameIsReceivedAtIndex(size_t index) const {
  if (!demux_ || demux_state_ <= WEBP_DEMUX_PARSING_HEADER)
    return false;
  if (!(format_flags_ & ANIMATION_FLAG))
    return ImageDecoder::FrameIsReceivedAtIndex(index);
  bool frame_is_received_at_index = index < frame_buffer_cache_.size();
  return frame_is_received_at_index;
}

TimeDelta WEBPImageDecoder::FrameDurationAtIndex(size_t index) const {
  return index < frame_buffer_cache_.size()
             ? frame_buffer_cache_[index].Duration()
             : TimeDelta();
}

bool WEBPImageDecoder::UpdateDemuxer() {
  if (Failed())
    return false;

  if (have_already_parsed_this_data_)
    return true;

  have_already_parsed_this_data_ = true;

  const unsigned kWebpHeaderSize = 30;
  if (data_->size() < kWebpHeaderSize)
    return false;  // Await VP8X header so WebPDemuxPartial succeeds.

  WebPDemuxDelete(demux_);
  consolidated_data_ = data_->GetAsSkData();
  WebPData input_data = {
      reinterpret_cast<const uint8_t*>(consolidated_data_->data()),
      consolidated_data_->size()};
  demux_ = WebPDemuxPartial(&input_data, &demux_state_);
  if (!demux_ || (IsAllDataReceived() && demux_state_ != WEBP_DEMUX_DONE)) {
    if (!demux_)
      consolidated_data_.reset();
    return SetFailed();
  }

  DCHECK_GT(demux_state_, WEBP_DEMUX_PARSING_HEADER);
  if (!WebPDemuxGetI(demux_, WEBP_FF_FRAME_COUNT))
    return false;  // Wait until the encoded image frame data arrives.

  if (!IsDecodedSizeAvailable()) {
    int width = WebPDemuxGetI(demux_, WEBP_FF_CANVAS_WIDTH);
    int height = WebPDemuxGetI(demux_, WEBP_FF_CANVAS_HEIGHT);
    if (!SetSize(width, height))
      return SetFailed();

    format_flags_ = WebPDemuxGetI(demux_, WEBP_FF_FORMAT_FLAGS);
    if (!(format_flags_ & ANIMATION_FLAG)) {
      repetition_count_ = kAnimationNone;
    } else {
      // Since we have parsed at least one frame, even if partially,
      // the global animation (ANIM) properties have been read since
      // an ANIM chunk must precede the ANMF frame chunks.
      repetition_count_ = WebPDemuxGetI(demux_, WEBP_FF_LOOP_COUNT);
      // Repetition count is always <= 16 bits.
      DCHECK_EQ(repetition_count_, repetition_count_ & 0xffff);
      if (!repetition_count_)
        repetition_count_ = kAnimationLoopInfinite;
      // FIXME: Implement ICC profile support for animated images.
      format_flags_ &= ~ICCP_FLAG;
    }

    if ((format_flags_ & ICCP_FLAG) && !IgnoresColorSpace())
      ReadColorProfile();
  }

  DCHECK(IsDecodedSizeAvailable());

  size_t frame_count = WebPDemuxGetI(demux_, WEBP_FF_FRAME_COUNT);
  UpdateAggressivePurging(frame_count);

  return true;
}

void WEBPImageDecoder::OnInitFrameBuffer(size_t frame_index) {
  // ImageDecoder::InitFrameBuffer does a DCHECK if |frame_index| exists.
  ImageFrame& buffer = frame_buffer_cache_[frame_index];

  const size_t required_previous_frame_index =
      buffer.RequiredPreviousFrameIndex();
  if (required_previous_frame_index == kNotFound) {
    frame_background_has_alpha_ =
        !buffer.OriginalFrameRect().Contains(IntRect(IntPoint(), Size()));
  } else {
    const ImageFrame& prev_buffer =
        frame_buffer_cache_[required_previous_frame_index];
    frame_background_has_alpha_ =
        prev_buffer.HasAlpha() || (prev_buffer.GetDisposalMethod() ==
                                   ImageFrame::kDisposeOverwriteBgcolor);
  }

  // The buffer is transparent outside the decoded area while the image is
  // loading. The correct alpha value for the frame will be set when it is fully
  // decoded.
  buffer.SetHasAlpha(true);
}

bool WEBPImageDecoder::CanReusePreviousFrameBuffer(size_t frame_index) const {
  DCHECK(frame_index < frame_buffer_cache_.size());
  return frame_buffer_cache_[frame_index].GetAlphaBlendSource() !=
         ImageFrame::kBlendAtopPreviousFrame;
}

void WEBPImageDecoder::ClearFrameBuffer(size_t frame_index) {
  if (demux_ && demux_state_ >= WEBP_DEMUX_PARSED_HEADER &&
      frame_buffer_cache_[frame_index].GetStatus() ==
          ImageFrame::kFramePartial) {
    // Clear the decoder state so that this partial frame can be decoded again
    // when requested.
    ClearDecoder();
  }
  ImageDecoder::ClearFrameBuffer(frame_index);
}

void WEBPImageDecoder::ReadColorProfile() {
  WebPChunkIterator chunk_iterator;
  if (!WebPDemuxGetChunk(demux_, "ICCP", 1, &chunk_iterator)) {
    WebPDemuxReleaseChunkIterator(&chunk_iterator);
    return;
  }

  const char* profile_data =
      reinterpret_cast<const char*>(chunk_iterator.chunk.bytes);
  size_t profile_size = chunk_iterator.chunk.size;

  SetEmbeddedColorProfile(profile_data, profile_size);

  WebPDemuxReleaseChunkIterator(&chunk_iterator);
}

void WEBPImageDecoder::ApplyPostProcessing(size_t frame_index) {
  ImageFrame& buffer = frame_buffer_cache_[frame_index];
  int width;
  int decoded_height;
  if (!WebPIDecGetRGB(decoder_, &decoded_height, &width, 0, 0))
    return;  // See also https://bugs.webkit.org/show_bug.cgi?id=74062
  if (decoded_height <= 0)
    return;

  const IntRect& frame_rect = buffer.OriginalFrameRect();
  SECURITY_DCHECK(width == frame_rect.Width());
  SECURITY_DCHECK(decoded_height <= frame_rect.Height());
  const int left = frame_rect.X();
  const int top = frame_rect.Y();

  // TODO (msarett):
  // Here we apply the color space transformation to the dst space.
  // It does not really make sense to transform to a gamma-encoded
  // space and then immediately after, perform a linear premultiply
  // and linear blending.  Can we find a way to perform the
  // premultiplication and blending in a linear space?
  SkColorSpaceXform* xform = ColorTransform();
  if (xform) {
    const SkColorSpaceXform::ColorFormat kSrcFormat =
        SkColorSpaceXform::kBGRA_8888_ColorFormat;
    const SkColorSpaceXform::ColorFormat kDstFormat =
        SkColorSpaceXform::kRGBA_8888_ColorFormat;
    for (int y = decoded_height_; y < decoded_height; ++y) {
      const int canvas_y = top + y;
      uint8_t* row = reinterpret_cast<uint8_t*>(buffer.GetAddr(left, canvas_y));
      xform->apply(kDstFormat, row, kSrcFormat, row, width,
                   kUnpremul_SkAlphaType);

      uint8_t* pixel = row;
      for (int x = 0; x < width; ++x, pixel += 4) {
        const int canvas_x = left + x;
        buffer.SetRGBA(canvas_x, canvas_y, pixel[0], pixel[1], pixel[2],
                       pixel[3]);
      }
    }
  }

  // During the decoding of the current frame, we may have set some pixels to be
  // transparent (i.e. alpha < 255). If the alpha blend source was
  // 'BlendAtopPreviousFrame', the values of these pixels should be determined
  // by blending them against the pixels of the corresponding previous frame.
  // Compute the correct opaque values now.
  // FIXME: This could be avoided if libwebp decoder had an API that used the
  // previous required frame to do the alpha-blending by itself.
  if ((format_flags_ & ANIMATION_FLAG) && frame_index &&
      buffer.GetAlphaBlendSource() == ImageFrame::kBlendAtopPreviousFrame &&
      buffer.RequiredPreviousFrameIndex() != kNotFound) {
    ImageFrame& prev_buffer = frame_buffer_cache_[frame_index - 1];
    DCHECK_EQ(prev_buffer.GetStatus(), ImageFrame::kFrameComplete);
    ImageFrame::DisposalMethod prev_disposal_method =
        prev_buffer.GetDisposalMethod();
    if (prev_disposal_method == ImageFrame::kDisposeKeep) {
      // Blend transparent pixels with pixels in previous canvas.
      for (int y = decoded_height_; y < decoded_height; ++y) {
        blend_function_(buffer, prev_buffer, top + y, left, width);
      }
    } else if (prev_disposal_method == ImageFrame::kDisposeOverwriteBgcolor) {
      const IntRect& prev_rect = prev_buffer.OriginalFrameRect();
      // We need to blend a transparent pixel with the starting value (from just
      // after the InitFrame() call). If the pixel belongs to prev_rect, the
      // starting value was fully transparent, so this is a no-op. Otherwise, we
      // need to blend against the pixel from the previous canvas.
      for (int y = decoded_height_; y < decoded_height; ++y) {
        int canvas_y = top + y;
        int left1, width1, left2, width2;
        findBlendRangeAtRow(frame_rect, prev_rect, canvas_y, left1, width1,
                            left2, width2);
        if (width1 > 0)
          blend_function_(buffer, prev_buffer, canvas_y, left1, width1);
        if (width2 > 0)
          blend_function_(buffer, prev_buffer, canvas_y, left2, width2);
      }
    }
  }

  decoded_height_ = decoded_height;
  buffer.SetPixelsChanged(true);
}

size_t WEBPImageDecoder::DecodeFrameCount() {
  // If UpdateDemuxer() fails, return the existing number of frames.  This way
  // if we get halfway through the image before decoding fails, we won't
  // suddenly start reporting that the image has zero frames.
  return UpdateDemuxer() ? WebPDemuxGetI(demux_, WEBP_FF_FRAME_COUNT)
                         : frame_buffer_cache_.size();
}

void WEBPImageDecoder::InitializeNewFrame(size_t index) {
  if (!(format_flags_ & ANIMATION_FLAG)) {
    DCHECK(!index);
    return;
  }
  WebPIterator animated_frame;
  WebPDemuxGetFrame(demux_, index + 1, &animated_frame);
  DCHECK_EQ(animated_frame.complete, 1);
  ImageFrame* buffer = &frame_buffer_cache_[index];
  IntRect frame_rect(animated_frame.x_offset, animated_frame.y_offset,
                     animated_frame.width, animated_frame.height);
  buffer->SetOriginalFrameRect(
      Intersection(frame_rect, IntRect(IntPoint(), Size())));
  buffer->SetDuration(TimeDelta::FromMilliseconds(animated_frame.duration));
  buffer->SetDisposalMethod(animated_frame.dispose_method ==
                                    WEBP_MUX_DISPOSE_BACKGROUND
                                ? ImageFrame::kDisposeOverwriteBgcolor
                                : ImageFrame::kDisposeKeep);
  buffer->SetAlphaBlendSource(animated_frame.blend_method == WEBP_MUX_BLEND
                                  ? ImageFrame::kBlendAtopPreviousFrame
                                  : ImageFrame::kBlendAtopBgcolor);
  buffer->SetRequiredPreviousFrameIndex(
      FindRequiredPreviousFrame(index, !animated_frame.has_alpha));
  WebPDemuxReleaseIterator(&animated_frame);
}

void WEBPImageDecoder::Decode(size_t index) {
  if (Failed())
    return;

  Vector<size_t> frames_to_decode = FindFramesToDecode(index);

  DCHECK(demux_);
  for (auto i = frames_to_decode.rbegin(); i != frames_to_decode.rend(); ++i) {
    if ((format_flags_ & ANIMATION_FLAG) && !InitFrameBuffer(*i)) {
      SetFailed();
      return;
    }

    WebPIterator webp_frame;
    if (!WebPDemuxGetFrame(demux_, *i + 1, &webp_frame)) {
      SetFailed();
    } else {
      DecodeSingleFrame(webp_frame.fragment.bytes, webp_frame.fragment.size,
                        *i);
      WebPDemuxReleaseIterator(&webp_frame);
    }
    if (Failed())
      return;

    // If this returns false, we need more data to continue decoding.
    if (!PostDecodeProcessing(*i))
      break;
  }

  // It is also a fatal error if all data is received and we have decoded all
  // frames available but the file is truncated.
  if (index >= frame_buffer_cache_.size() - 1 && IsAllDataReceived() &&
      demux_ && demux_state_ != WEBP_DEMUX_DONE)
    SetFailed();
}

bool WEBPImageDecoder::DecodeSingleFrame(const uint8_t* data_bytes,
                                         size_t data_size,
                                         size_t frame_index) {
  if (Failed())
    return false;

  DCHECK(IsDecodedSizeAvailable());

  DCHECK_GT(frame_buffer_cache_.size(), frame_index);
  ImageFrame& buffer = frame_buffer_cache_[frame_index];
  DCHECK_NE(buffer.GetStatus(), ImageFrame::kFrameComplete);

  if (buffer.GetStatus() == ImageFrame::kFrameEmpty) {
    if (!buffer.AllocatePixelData(Size().Width(), Size().Height(),
                                  ColorSpaceForSkImages()))
      return SetFailed();
    buffer.ZeroFillPixelData();
    buffer.SetStatus(ImageFrame::kFramePartial);
    // The buffer is transparent outside the decoded area while the image is
    // loading. The correct alpha value for the frame will be set when it is
    // fully decoded.
    buffer.SetHasAlpha(true);
    buffer.SetOriginalFrameRect(IntRect(IntPoint(), Size()));
  }

  const IntRect& frame_rect = buffer.OriginalFrameRect();
  if (!decoder_) {
    WEBP_CSP_MODE mode = outputMode(format_flags_ & ALPHA_FLAG);
    if (!premultiply_alpha_)
      mode = outputMode(false);
    if (ColorTransform()) {
      // Swizzling between RGBA and BGRA is zero cost in a color transform.
      // So when we have a color transform, we should decode to whatever is
      // easiest for libwebp, and then let the color transform swizzle if
      // necessary.
      // Lossy webp is encoded as YUV (so RGBA and BGRA are the same cost).
      // Lossless webp is encoded as BGRA. This means decoding to BGRA is
      // either faster or the same cost as RGBA.
      mode = MODE_BGRA;
    }
    WebPInitDecBuffer(&decoder_buffer_);
    decoder_buffer_.colorspace = mode;
    decoder_buffer_.u.RGBA.stride =
        Size().Width() * sizeof(ImageFrame::PixelData);
    decoder_buffer_.u.RGBA.size =
        decoder_buffer_.u.RGBA.stride * frame_rect.Height();
    decoder_buffer_.is_external_memory = 1;
    decoder_ = WebPINewDecoder(&decoder_buffer_);
    if (!decoder_)
      return SetFailed();
  }

  decoder_buffer_.u.RGBA.rgba = reinterpret_cast<uint8_t*>(
      buffer.GetAddr(frame_rect.X(), frame_rect.Y()));

  switch (WebPIUpdate(decoder_, data_bytes, data_size)) {
    case VP8_STATUS_OK:
      ApplyPostProcessing(frame_index);
      buffer.SetHasAlpha((format_flags_ & ALPHA_FLAG) ||
                         frame_background_has_alpha_);
      buffer.SetStatus(ImageFrame::kFrameComplete);
      ClearDecoder();
      return true;
    case VP8_STATUS_SUSPENDED:
      if (!IsAllDataReceived() && !FrameIsReceivedAtIndex(frame_index)) {
        ApplyPostProcessing(frame_index);
        return false;
      }
    // FALLTHROUGH
    default:
      Clear();
      return SetFailed();
  }
}

}  // namespace blink
