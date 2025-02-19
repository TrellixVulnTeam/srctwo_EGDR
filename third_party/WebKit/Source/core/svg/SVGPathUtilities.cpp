/*
 * Copyright (C) Research In Motion Limited 2010, 2012. All rights reserved.
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

#include "core/svg/SVGPathUtilities.h"

#include "core/svg/SVGPathBuilder.h"
#include "core/svg/SVGPathByteStreamBuilder.h"
#include "core/svg/SVGPathByteStreamSource.h"
#include "core/svg/SVGPathParser.h"
#include "core/svg/SVGPathStringBuilder.h"
#include "core/svg/SVGPathStringSource.h"

namespace blink {

bool BuildPathFromString(const String& d, Path& result) {
  if (d.IsEmpty())
    return true;

  SVGPathBuilder builder(result);
  SVGPathStringSource source(d);
  return SVGPathParser::ParsePath(source, builder);
}

bool BuildPathFromByteStream(const SVGPathByteStream& stream, Path& result) {
  if (stream.IsEmpty())
    return true;

  SVGPathBuilder builder(result);
  SVGPathByteStreamSource source(stream);
  return SVGPathParser::ParsePath(source, builder);
}

String BuildStringFromByteStream(const SVGPathByteStream& stream) {
  if (stream.IsEmpty())
    return String();

  SVGPathStringBuilder builder;
  SVGPathByteStreamSource source(stream);
  SVGPathParser::ParsePath(source, builder);
  return builder.Result();
}

SVGParsingError BuildByteStreamFromString(const String& d,
                                          SVGPathByteStream& result) {
  result.clear();
  if (d.IsEmpty())
    return SVGParseStatus::kNoError;

  // The string length is typically a minor overestimate of eventual byte stream
  // size, so it avoids us a lot of reallocs.
  result.ReserveInitialCapacity(d.length());

  SVGPathByteStreamBuilder builder(result);
  SVGPathStringSource source(d);
  SVGPathParser::ParsePath(source, builder);
  result.ShrinkToFit();
  return source.ParseError();
}

}  // namespace blink
