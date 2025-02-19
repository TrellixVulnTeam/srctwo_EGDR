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

#ifndef SVGPathByteStreamSource_h
#define SVGPathByteStreamSource_h

#include "build/build_config.h"
#include "core/svg/SVGPathByteStream.h"
#include "core/svg/SVGPathData.h"
#include "platform/geometry/FloatPoint.h"

namespace blink {

class SVGPathByteStreamSource {
  WTF_MAKE_NONCOPYABLE(SVGPathByteStreamSource);
  STACK_ALLOCATED();

 public:
  explicit SVGPathByteStreamSource(const SVGPathByteStream& stream)
      : stream_current_(stream.begin()), stream_end_(stream.end()) {}

  bool HasMoreData() const { return stream_current_ < stream_end_; }
  PathSegmentData ParseSegment();

 private:
#if defined(COMPILER_MSVC)
#pragma warning(disable : 4701)
#endif
  template <typename DataType>
  DataType ReadType() {
    ByteType<DataType> data;
    size_t type_size = sizeof(ByteType<DataType>);
    DCHECK_LE(stream_current_ + type_size, stream_end_);
    memcpy(data.bytes, stream_current_, type_size);
    stream_current_ += type_size;
    return data.value;
  }

  bool ReadFlag() { return ReadType<bool>(); }
  float ReadFloat() { return ReadType<float>(); }
  unsigned short ReadSVGSegmentType() { return ReadType<unsigned short>(); }
  FloatPoint ReadFloatPoint() {
    float x = ReadType<float>();
    float y = ReadType<float>();
    return FloatPoint(x, y);
  }

  SVGPathByteStream::DataIterator stream_current_;
  SVGPathByteStream::DataIterator stream_end_;
};

}  // namespace blink

#endif  // SVGPathByteStreamSource_h
