/*
 * Copyright (C) 2013 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer
 *    in the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of Google Inc. nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
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

#include "core/html/custom/V0CustomElement.h"

#include "core/HTMLNames.h"
#include "core/MathMLNames.h"
#include "core/SVGNames.h"
#include "core/dom/Document.h"
#include "core/dom/Element.h"
#include "core/frame/UseCounter.h"
#include "core/html/custom/V0CustomElementMicrotaskRunQueue.h"
#include "core/html/custom/V0CustomElementObserver.h"
#include "core/html/custom/V0CustomElementScheduler.h"

namespace blink {

V0CustomElementMicrotaskImportStep* V0CustomElement::DidCreateImport(
    HTMLImportChild* import) {
  return V0CustomElementScheduler::ScheduleImport(import);
}

void V0CustomElement::DidFinishLoadingImport(Document& master) {
  master.CustomElementMicrotaskRunQueue()->RequestDispatchIfNeeded();
}

Vector<AtomicString>& V0CustomElement::EmbedderCustomElementNames() {
  DEFINE_STATIC_LOCAL(Vector<AtomicString>, names, ());
  return names;
}

void V0CustomElement::AddEmbedderCustomElementName(const AtomicString& name) {
  AtomicString lower = name.DeprecatedLower();
  if (IsValidName(lower, kEmbedderNames))
    return;
  EmbedderCustomElementNames().push_back(lower);
}

static inline bool IsValidNCName(const AtomicString& name) {
  if (kNotFound != name.find(':'))
    return false;

  if (!name.GetString().Is8Bit()) {
    const UChar32 c = name.Characters16()[0];
    // These characters comes under CombiningChar in NCName and according to
    // NCName only BaseChar and Ideodgraphic can come as first chars.
    // Also these characters come under Letter_Other in UnicodeData, thats
    // why they pass as valid document name.
    if (c == 0x0B83 || c == 0x0F88 || c == 0x0F89 || c == 0x0F8A || c == 0x0F8B)
      return false;
  }

  return Document::IsValidName(name.GetString());
}

bool V0CustomElement::IsValidName(const AtomicString& name,
                                  NameSet valid_names) {
  if ((valid_names & kEmbedderNames) &&
      kNotFound != EmbedderCustomElementNames().Find(name))
    return Document::IsValidName(name);

  if ((valid_names & kStandardNames) && kNotFound != name.find('-')) {
    DEFINE_STATIC_LOCAL(Vector<AtomicString>, reserved_names, ());
    if (reserved_names.IsEmpty()) {
      // FIXME(crbug.com/426605): We should be able to remove this.
      reserved_names.push_back(MathMLNames::annotation_xmlTag.LocalName());
    }

    if (kNotFound == reserved_names.Find(name))
      return IsValidNCName(name);
  }

  return false;
}

void V0CustomElement::Define(Element* element,
                             V0CustomElementDefinition* definition) {
  switch (element->GetV0CustomElementState()) {
    case Element::kV0NotCustomElement:
    case Element::kV0Upgraded:
      NOTREACHED();
      break;

    case Element::kV0WaitingForUpgrade:
      UseCounter::Count(
          element->GetDocument(),
          definition->Descriptor().IsTypeExtension()
              ? WebFeature::kV0CustomElementsCreateTypeExtensionElement
              : WebFeature::kV0CustomElementsCreateCustomTagElement);
      element->V0SetCustomElementDefinition(definition);
      V0CustomElementScheduler::ScheduleCallback(
          definition->Callbacks(), element,
          V0CustomElementLifecycleCallbacks::kCreatedCallback);
      break;
  }
}

void V0CustomElement::AttributeDidChange(Element* element,
                                         const AtomicString& name,
                                         const AtomicString& old_value,
                                         const AtomicString& new_value) {
  DCHECK_EQ(element->GetV0CustomElementState(), Element::kV0Upgraded);
  V0CustomElementScheduler::ScheduleAttributeChangedCallback(
      element->GetV0CustomElementDefinition()->Callbacks(), element, name,
      old_value, new_value);
}

void V0CustomElement::DidAttach(Element* element, const Document& document) {
  DCHECK_EQ(element->GetV0CustomElementState(), Element::kV0Upgraded);
  if (!document.domWindow())
    return;
  V0CustomElementScheduler::ScheduleCallback(
      element->GetV0CustomElementDefinition()->Callbacks(), element,
      V0CustomElementLifecycleCallbacks::kAttachedCallback);
}

void V0CustomElement::DidDetach(Element* element, const Document& document) {
  DCHECK_EQ(element->GetV0CustomElementState(), Element::kV0Upgraded);
  if (!document.domWindow())
    return;
  V0CustomElementScheduler::ScheduleCallback(
      element->GetV0CustomElementDefinition()->Callbacks(), element,
      V0CustomElementLifecycleCallbacks::kDetachedCallback);
}

void V0CustomElement::WasDestroyed(Element* element) {
  switch (element->GetV0CustomElementState()) {
    case Element::kV0NotCustomElement:
      NOTREACHED();
      break;

    case Element::kV0WaitingForUpgrade:
    case Element::kV0Upgraded:
      V0CustomElementObserver::NotifyElementWasDestroyed(element);
      break;
  }
}

}  // namespace blink
