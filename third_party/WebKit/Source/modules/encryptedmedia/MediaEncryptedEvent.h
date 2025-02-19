/*
 * Copyright (C) 2012 Google Inc.  All rights reserved.
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

#ifndef MediaEncryptedEvent_h
#define MediaEncryptedEvent_h

#include "modules/EventModules.h"
#include "modules/encryptedmedia/MediaEncryptedEventInit.h"

namespace blink {

class MediaEncryptedEvent final : public Event {
  DEFINE_WRAPPERTYPEINFO();

 public:
  ~MediaEncryptedEvent() override;

  static MediaEncryptedEvent* Create(
      const AtomicString& type,
      const MediaEncryptedEventInit& initializer) {
    return new MediaEncryptedEvent(type, initializer);
  }

  const AtomicString& InterfaceName() const override;

  String initDataType() const { return init_data_type_; }
  DOMArrayBuffer* initData() const { return init_data_.Get(); }

  DECLARE_VIRTUAL_TRACE();

 private:
  MediaEncryptedEvent(const AtomicString& type,
                      const MediaEncryptedEventInit& initializer);

  String init_data_type_;
  Member<DOMArrayBuffer> init_data_;
};

}  // namespace blink

#endif  // MediaEncryptedEvent_h
