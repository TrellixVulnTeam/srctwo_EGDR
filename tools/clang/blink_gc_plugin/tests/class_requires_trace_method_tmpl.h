// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLASS_REQUIRES_TRACE_METHOD_TMPL_H_
#define CLASS_REQUIRES_TRACE_METHOD_TMPL_H_

#include "heap/stubs.h"

namespace blink {

class HeapObject : public GarbageCollected<HeapObject> { };

class PartObjectA {
    DISALLOW_NEW();
};

class PartObjectB {
    DISALLOW_NEW();
public:
    void Trace(Visitor* visitor) { visitor->Trace(m_obj); }
private:
    Member<HeapObject> m_obj;
};

template<typename T>
class TemplatedObject {
private:
    T m_part;
};

}

#endif
