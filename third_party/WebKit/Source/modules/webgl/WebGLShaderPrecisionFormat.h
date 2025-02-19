/*
 * Copyright (c) 2012, Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef WebGLShaderPrecisionFormat_h
#define WebGLShaderPrecisionFormat_h

#include "platform/bindings/ScriptWrappable.h"
#include "platform/heap/Handle.h"
#include "third_party/khronos/GLES2/gl2.h"

namespace blink {

class WebGLShaderPrecisionFormat final
    : public GarbageCollected<WebGLShaderPrecisionFormat>,
      public ScriptWrappable {
  DEFINE_WRAPPERTYPEINFO();

 public:
  static WebGLShaderPrecisionFormat* Create(GLint range_min,
                                            GLint range_max,
                                            GLint precision);

  GLint rangeMin() const;
  GLint rangeMax() const;
  GLint precision() const;

  DEFINE_INLINE_TRACE() {}

 private:
  WebGLShaderPrecisionFormat(GLint range_min, GLint range_max, GLint precision);

  GLint range_min_;
  GLint range_max_;
  GLint precision_;
};

}  // namespace blink

#endif  // WebGLShaderPrecisionFormat_h
