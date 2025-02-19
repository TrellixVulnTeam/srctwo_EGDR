/* Copyright (c) 2012 The Chromium Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* From dev/pp_video_dev.idl modified Tue Apr 30 14:58:38 2013. */

#ifndef PPAPI_C_DEV_PP_VIDEO_DEV_H_
#define PPAPI_C_DEV_PP_VIDEO_DEV_H_

#include "ppapi/c/pp_macros.h"
#include "ppapi/c/pp_resource.h"
#include "ppapi/c/pp_size.h"
#include "ppapi/c/pp_stdint.h"

/**
 * @file
 * NOTE: these must be kept in sync with the versions in media/!
 */


/**
 * @addtogroup Enums
 * @{
 */
/**
 * Video format.
 *
 * Keep the values in this enum unique, as they imply format (h.264 vs. VP8,
 * for example), and keep the values for a particular format grouped together
 * for clarity.
 * Note: Keep these in sync with media::VideoCodecProfile.
 */
typedef enum {
  PP_VIDEODECODER_PROFILE_UNKNOWN = -1,
  PP_VIDEODECODER_H264PROFILE_NONE = 0,
  PP_VIDEODECODER_H264PROFILE_BASELINE = 1,
  PP_VIDEODECODER_H264PROFILE_MAIN = 2,
  PP_VIDEODECODER_H264PROFILE_EXTENDED = 3,
  PP_VIDEODECODER_H264PROFILE_HIGH = 4,
  PP_VIDEODECODER_H264PROFILE_HIGH10PROFILE = 5,
  PP_VIDEODECODER_H264PROFILE_HIGH422PROFILE = 6,
  PP_VIDEODECODER_H264PROFILE_HIGH444PREDICTIVEPROFILE = 7,
  PP_VIDEODECODER_H264PROFILE_SCALABLEBASELINE = 8,
  PP_VIDEODECODER_H264PROFILE_SCALABLEHIGH = 9,
  PP_VIDEODECODER_H264PROFILE_STEREOHIGH = 10,
  PP_VIDEODECODER_H264PROFILE_MULTIVIEWHIGH = 11,
  PP_VIDEODECODER_VP8PROFILE_ANY = 12
} PP_VideoDecoder_Profile;
PP_COMPILE_ASSERT_SIZE_IN_BYTES(PP_VideoDecoder_Profile, 4);
/**
 * @}
 */

/**
 * @addtogroup Structs
 * @{
 */
/**
 * The data structure for video bitstream buffer.
 */
struct PP_VideoBitstreamBuffer_Dev {
  /**
   * Client-specified identifier for the bitstream buffer. Valid values are
   * non-negative.
   */
  int32_t id;
  /**
   * Buffer to hold the bitstream data. Should be allocated using the
   * PPB_Buffer interface for consistent interprocess behaviour.
   */
  PP_Resource data;
  /**
   * Size of the bitstream contained in buffer (in bytes).
   */
  uint32_t size;
};
PP_COMPILE_ASSERT_STRUCT_SIZE_IN_BYTES(PP_VideoBitstreamBuffer_Dev, 12);

/**
 * Struct for specifying texture-backed picture data.
 */
struct PP_PictureBuffer_Dev {
  /**
   * Client-specified id for the picture buffer. By using this value client can
   * keep track of the buffers it has assigned to the video decoder and how they
   * are passed back to it. Valid values are non-negative.
   */
  int32_t id;
  /**
   * Dimensions of the buffer.
   */
  struct PP_Size size;
  /**
   * Texture ID in the given context where picture is stored.
   */
  uint32_t texture_id;
};
PP_COMPILE_ASSERT_STRUCT_SIZE_IN_BYTES(PP_PictureBuffer_Dev, 16);

/**
 * Structure to describe a decoded output frame.
 */
struct PP_Picture_Dev {
  /**
   * ID of the picture buffer where the picture is stored.
   */
  int32_t picture_buffer_id;
  /**
   * ID of the bitstream from which this data was decoded.
   */
  int32_t bitstream_buffer_id;
};
PP_COMPILE_ASSERT_STRUCT_SIZE_IN_BYTES(PP_Picture_Dev, 8);
/**
 * @}
 */

/**
 * @addtogroup Enums
 * @{
 */
/**
 * Decoder error codes reported to the plugin. A reasonable naive
 * error handling policy is for the plugin to Destroy() the decoder on error.
 */
typedef enum {
  /**
   * An operation was attempted during an incompatible decoder state.
   */
  PP_VIDEODECODERERROR_ILLEGAL_STATE = 1,
  /**
   * Invalid argument was passed to an API method.
   */
  PP_VIDEODECODERERROR_INVALID_ARGUMENT = 2,
  /**
   * Encoded input is unreadable.
   */
  PP_VIDEODECODERERROR_UNREADABLE_INPUT = 3,
  /**
   * A failure occurred at the browser layer or lower.  Examples of such
   * failures include GPU hardware failures, GPU driver failures, GPU library
   * failures, browser programming errors, and so on.
   */
  PP_VIDEODECODERERROR_PLATFORM_FAILURE = 4
} PP_VideoDecodeError_Dev;
PP_COMPILE_ASSERT_SIZE_IN_BYTES(PP_VideoDecodeError_Dev, 4);
/**
 * @}
 */

#endif  /* PPAPI_C_DEV_PP_VIDEO_DEV_H_ */

