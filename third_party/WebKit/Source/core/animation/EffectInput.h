// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EffectInput_h
#define EffectInput_h

#include "core/CoreExport.h"
#include "core/animation/EffectModel.h"
#include "platform/wtf/Allocator.h"
#include "platform/wtf/Vector.h"

namespace blink {

class EffectModel;
class DictionarySequenceOrDictionary;
class Dictionary;
class Element;
class ExceptionState;
class ExecutionContext;

class CORE_EXPORT EffectInput {
  STATIC_ONLY(EffectInput);

 public:
  // TODO(alancutter): Replace Element* parameter with Document&.
  static EffectModel* Convert(Element*,
                              const DictionarySequenceOrDictionary&,
                              ExecutionContext*,
                              ExceptionState&);

 private:
  static EffectModel* ConvertArrayForm(Element&,
                                       const Vector<Dictionary>& keyframes,
                                       ExecutionContext*,
                                       ExceptionState&);
  static EffectModel* ConvertObjectForm(Element&,
                                        const Dictionary& keyframe,
                                        ExecutionContext*,
                                        ExceptionState&);
};

}  // namespace blink

#endif
