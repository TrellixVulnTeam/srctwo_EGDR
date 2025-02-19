/*
 * Copyright (C) 2013 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
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

#ifndef StyleSheetCandidate_h
#define StyleSheetCandidate_h

#include "platform/heap/Handle.h"
#include "platform/wtf/text/AtomicString.h"
#include "platform/wtf/text/WTFString.h"

namespace blink {

class Document;
class Node;
class StyleSheet;

class StyleSheetCandidate {
  STACK_ALLOCATED();

 public:
  enum Type { kHTMLLink, kHTMLStyle, kSVGStyle, kPi, kInvalid };

  StyleSheetCandidate(Node& node) : node_(node), type_(TypeOf(node)) {}

  bool IsXSL() const;
  bool IsImport() const;
  bool IsAlternate() const;
  bool IsEnabledViaScript() const;
  bool IsEnabledAndLoading() const;
  bool CanBeActivated(const String& current_preferrable_name) const;
  bool IsCSSStyle() const;

  StyleSheet* Sheet() const;
  AtomicString Title() const;
  Document* ImportedDocument() const;

 private:
  bool IsElement() const { return type_ != kPi; }
  bool IsHTMLLink() const { return type_ == kHTMLLink; }
  Node& GetNode() const { return *node_; }

  static Type TypeOf(Node&);

  Member<Node> node_;
  Type type_;
};

}  // namespace blink

#endif
