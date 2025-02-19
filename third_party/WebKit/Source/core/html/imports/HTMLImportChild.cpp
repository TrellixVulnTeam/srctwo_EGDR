/*
 * Copyright (C) 2013 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
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

#include "core/html/imports/HTMLImportChild.h"

#include "core/css/StyleEngine.h"
#include "core/css/StyleSheetList.h"
#include "core/dom/Document.h"
#include "core/frame/Deprecation.h"
#include "core/frame/UseCounter.h"
#include "core/html/custom/V0CustomElement.h"
#include "core/html/custom/V0CustomElementMicrotaskImportStep.h"
#include "core/html/custom/V0CustomElementSyncMicrotaskQueue.h"
#include "core/html/imports/HTMLImportChildClient.h"
#include "core/html/imports/HTMLImportLoader.h"
#include "core/html/imports/HTMLImportTreeRoot.h"
#include "core/html/imports/HTMLImportsController.h"

namespace blink {

HTMLImportChild::HTMLImportChild(const KURL& url,
                                 HTMLImportLoader* loader,
                                 HTMLImportChildClient* client,
                                 SyncMode sync)
    : HTMLImport(sync), url_(url), loader_(loader), client_(client) {
  DCHECK(loader_);
  DCHECK(client_);
}

HTMLImportChild::~HTMLImportChild() {}

void HTMLImportChild::OwnerInserted() {
  if (!loader_->IsDone())
    return;

  DCHECK(Root());
  DCHECK(Root()->GetDocument());
  Root()->GetDocument()->GetStyleEngine().HtmlImportAddedOrRemoved();
}

void HTMLImportChild::DidShareLoader() {
  CreateCustomElementMicrotaskStepIfNeeded();
  StateWillChange();
}

void HTMLImportChild::DidStartLoading() {
  CreateCustomElementMicrotaskStepIfNeeded();
}

void HTMLImportChild::DidFinish() {
  if (client_)
    client_->DidFinish();
}

void HTMLImportChild::DidFinishLoading() {
  StateWillChange();
  if (GetDocument() && GetDocument()->GetStyleEngine().HasStyleSheets()) {
    Deprecation::CountDeprecation(Root()->GetDocument(),
                                  WebFeature::kHTMLImportsHasStyleSheets);
  }
  V0CustomElement::DidFinishLoadingImport(*(Root()->GetDocument()));
}

void HTMLImportChild::DidFinishUpgradingCustomElements() {
  StateWillChange();
  custom_element_microtask_step_.Clear();
}

void HTMLImportChild::Dispose() {
  InvalidateCustomElementMicrotaskStep();
  if (Parent())
    Parent()->RemoveChild(this);

  DCHECK(loader_);
  loader_->RemoveImport(this);
  loader_ = nullptr;

  if (client_) {
    client_->ImportChildWasDisposed(this);
    client_ = nullptr;
  }
}

Document* HTMLImportChild::GetDocument() const {
  DCHECK(loader_);
  return loader_->GetDocument();
}

void HTMLImportChild::StateWillChange() {
  ToHTMLImportTreeRoot(Root())->ScheduleRecalcState();
}

void HTMLImportChild::StateDidChange() {
  HTMLImport::StateDidChange();

  if (GetState().IsReady())
    DidFinish();
}

void HTMLImportChild::InvalidateCustomElementMicrotaskStep() {
  if (!custom_element_microtask_step_)
    return;
  custom_element_microtask_step_->Invalidate();
  custom_element_microtask_step_.Clear();
}

void HTMLImportChild::CreateCustomElementMicrotaskStepIfNeeded() {
  DCHECK(!custom_element_microtask_step_);

  if (!HasFinishedLoading() && !FormsCycle()) {
    custom_element_microtask_step_ = V0CustomElement::DidCreateImport(this);
  }
}

bool HTMLImportChild::HasFinishedLoading() const {
  DCHECK(loader_);

  return loader_->IsDone() && loader_->MicrotaskQueue()->IsEmpty() &&
         !custom_element_microtask_step_;
}

HTMLImportLoader* HTMLImportChild::Loader() const {
  // This should never be called after dispose().
  DCHECK(loader_);
  return loader_;
}

HTMLLinkElement* HTMLImportChild::Link() const {
  if (!client_)
    return nullptr;
  return client_->Link();
}

// Ensuring following invariants against the import tree:
// - HTMLImportLoader::FirstImport() is the "first import" of the DFS order of
//   the import tree loaded by the loader.
// - The "first import" manages all the children that are loaded by the
// document.
void HTMLImportChild::Normalize() {
  DCHECK(loader_);

  if (!loader_->IsFirstImport(this) && this->Precedes(loader_->FirstImport())) {
    HTMLImportChild* old_first = loader_->FirstImport();
    loader_->MoveToFirst(this);
    TakeChildrenFrom(old_first);
  }

  for (HTMLImportChild* child = ToHTMLImportChild(FirstChild()); child;
       child = ToHTMLImportChild(child->Next())) {
    if (child->FormsCycle())
      child->InvalidateCustomElementMicrotaskStep();
    child->Normalize();
  }
}

DEFINE_TRACE(HTMLImportChild) {
  visitor->Trace(custom_element_microtask_step_);
  visitor->Trace(loader_);
  visitor->Trace(client_);
  HTMLImport::Trace(visitor);
}

}  // namespace blink
