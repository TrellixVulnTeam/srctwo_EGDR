/*
 * Copyright (C) 2013 Google, Inc.
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

#include "core/xml/DocumentXPathEvaluator.h"

#include "bindings/core/v8/ExceptionState.h"
#include "core/xml/XPathExpression.h"
#include "core/xml/XPathResult.h"

namespace blink {

DocumentXPathEvaluator::DocumentXPathEvaluator(Document& document)
    : Supplement<Document>(document) {}

DocumentXPathEvaluator& DocumentXPathEvaluator::From(Document& document) {
  DocumentXPathEvaluator* cache = static_cast<DocumentXPathEvaluator*>(
      Supplement<Document>::From(document, SupplementName()));
  if (!cache) {
    cache = new DocumentXPathEvaluator(document);
    Supplement<Document>::ProvideTo(document, SupplementName(), cache);
  }
  return *cache;
}

XPathExpression* DocumentXPathEvaluator::createExpression(
    Document& document,
    const String& expression,
    XPathNSResolver* resolver,
    ExceptionState& exception_state) {
  DocumentXPathEvaluator& suplement = From(document);
  if (!suplement.xpath_evaluator_)
    suplement.xpath_evaluator_ = XPathEvaluator::Create();
  return suplement.xpath_evaluator_->createExpression(expression, resolver,
                                                      exception_state);
}

XPathNSResolver* DocumentXPathEvaluator::createNSResolver(Document& document,
                                                          Node* node_resolver) {
  DocumentXPathEvaluator& suplement = From(document);
  if (!suplement.xpath_evaluator_)
    suplement.xpath_evaluator_ = XPathEvaluator::Create();
  return suplement.xpath_evaluator_->createNSResolver(node_resolver);
}

XPathResult* DocumentXPathEvaluator::evaluate(Document& document,
                                              const String& expression,
                                              Node* context_node,
                                              XPathNSResolver* resolver,
                                              unsigned short type,
                                              const ScriptValue&,
                                              ExceptionState& exception_state) {
  DocumentXPathEvaluator& suplement = From(document);
  if (!suplement.xpath_evaluator_)
    suplement.xpath_evaluator_ = XPathEvaluator::Create();
  return suplement.xpath_evaluator_->evaluate(
      expression, context_node, resolver, type, ScriptValue(), exception_state);
}

DEFINE_TRACE(DocumentXPathEvaluator) {
  visitor->Trace(xpath_evaluator_);
  Supplement<Document>::Trace(visitor);
}

}  // namespace blink
