/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef WebGLSharedObject_h
#define WebGLSharedObject_h

#include "modules/webgl/WebGLObject.h"
#include "platform/bindings/TraceWrapperMember.h"

namespace blink {

class WebGLContextGroup;
class WebGLRenderingContextBase;

// WebGLSharedObject is the base class for objects that can be shared by
// multiple WebGLRenderingContexts.
class WebGLSharedObject : public WebGLObject {
 public:
  WebGLContextGroup* ContextGroup() const { return context_group_; }

  virtual bool IsBuffer() const { return false; }
  virtual bool IsProgram() const { return false; }
  virtual bool IsQuery() const { return false; }
  virtual bool IsRenderbuffer() const { return false; }
  virtual bool IsSampler() const { return false; }
  virtual bool IsShader() const { return false; }
  virtual bool IsSync() const { return false; }
  virtual bool IsTexture() const { return false; }

  bool Validate(const WebGLContextGroup* context_group,
                const WebGLRenderingContextBase*) const final;

  DECLARE_VIRTUAL_TRACE();

  DECLARE_VIRTUAL_TRACE_WRAPPERS();

 protected:
  explicit WebGLSharedObject(WebGLRenderingContextBase*);

  bool HasGroupOrContext() const final { return context_group_; }

  uint32_t CurrentNumberOfContextLosses() const final;

  gpu::gles2::GLES2Interface* GetAGLInterface() const final;

 private:
  TraceWrapperMember<WebGLContextGroup> context_group_;
};

}  // namespace blink

#endif  // WebGLSharedObject_h
