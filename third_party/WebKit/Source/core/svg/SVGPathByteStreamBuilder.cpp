/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "core/svg/SVGPathByteStreamBuilder.h"

#include "core/svg/SVGPathByteStream.h"
#include "core/svg/SVGPathData.h"
#include "platform/geometry/FloatPoint.h"

namespace blink {

// Helper class that coalesces writes to a SVGPathByteStream to a local buffer.
class CoalescingBuffer {
 public:
  CoalescingBuffer(SVGPathByteStream& byte_stream)
      : current_offset_(0), byte_stream_(byte_stream) {}
  ~CoalescingBuffer() { byte_stream_.Append(bytes_, current_offset_); }

  template <typename DataType>
  void WriteType(DataType value) {
    ByteType<DataType> data;
    data.value = value;
    size_t type_size = sizeof(ByteType<DataType>);
    DCHECK_LE(current_offset_ + type_size, sizeof(bytes_));
    memcpy(bytes_ + current_offset_, data.bytes, type_size);
    current_offset_ += type_size;
  }

  void WriteFlag(bool value) { WriteType<bool>(value); }
  void WriteFloat(float value) { WriteType<float>(value); }
  void WriteFloatPoint(const FloatPoint& point) {
    WriteType<float>(point.X());
    WriteType<float>(point.Y());
  }
  void WriteSegmentType(unsigned short value) {
    WriteType<unsigned short>(value);
  }

 private:
  // Adjust size to fit the largest command (in serialized/byte-stream format).
  // Currently a cubic segment.
  size_t current_offset_;
  unsigned char bytes_[sizeof(unsigned short) + sizeof(FloatPoint) * 3];
  SVGPathByteStream& byte_stream_;
};

SVGPathByteStreamBuilder::SVGPathByteStreamBuilder(
    SVGPathByteStream& byte_stream)
    : byte_stream_(byte_stream) {}

void SVGPathByteStreamBuilder::EmitSegment(const PathSegmentData& segment) {
  CoalescingBuffer buffer(byte_stream_);
  buffer.WriteSegmentType(segment.command);

  switch (segment.command) {
    case kPathSegMoveToRel:
    case kPathSegMoveToAbs:
    case kPathSegLineToRel:
    case kPathSegLineToAbs:
    case kPathSegCurveToQuadraticSmoothRel:
    case kPathSegCurveToQuadraticSmoothAbs:
      buffer.WriteFloatPoint(segment.target_point);
      break;
    case kPathSegLineToHorizontalRel:
    case kPathSegLineToHorizontalAbs:
      buffer.WriteFloat(segment.target_point.X());
      break;
    case kPathSegLineToVerticalRel:
    case kPathSegLineToVerticalAbs:
      buffer.WriteFloat(segment.target_point.Y());
      break;
    case kPathSegClosePath:
      break;
    case kPathSegCurveToCubicRel:
    case kPathSegCurveToCubicAbs:
      buffer.WriteFloatPoint(segment.point1);
      buffer.WriteFloatPoint(segment.point2);
      buffer.WriteFloatPoint(segment.target_point);
      break;
    case kPathSegCurveToCubicSmoothRel:
    case kPathSegCurveToCubicSmoothAbs:
      buffer.WriteFloatPoint(segment.point2);
      buffer.WriteFloatPoint(segment.target_point);
      break;
    case kPathSegCurveToQuadraticRel:
    case kPathSegCurveToQuadraticAbs:
      buffer.WriteFloatPoint(segment.point1);
      buffer.WriteFloatPoint(segment.target_point);
      break;
    case kPathSegArcRel:
    case kPathSegArcAbs:
      buffer.WriteFloatPoint(segment.point1);
      buffer.WriteFloat(segment.point2.X());
      buffer.WriteFlag(segment.arc_large);
      buffer.WriteFlag(segment.arc_sweep);
      buffer.WriteFloatPoint(segment.target_point);
      break;
    default:
      NOTREACHED();
  }
}

}  // namespace blink
