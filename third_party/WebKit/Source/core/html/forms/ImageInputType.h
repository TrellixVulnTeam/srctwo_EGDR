/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 * Copyright (C) 2011 Apple Inc. All rights reserved.
 * Copyright (C) 2012 Samsung Electronics. All rights reserved.
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

#ifndef ImageInputType_h
#define ImageInputType_h

#include "core/html/forms/BaseButtonInputType.h"
#include "platform/geometry/IntPoint.h"

namespace blink {

class ImageInputType final : public BaseButtonInputType {
 public:
  static InputType* Create(HTMLInputElement&);
  virtual RefPtr<ComputedStyle> CustomStyleForLayoutObject(
      RefPtr<ComputedStyle>);

 private:
  ImageInputType(HTMLInputElement&);
  const AtomicString& FormControlType() const override;
  bool IsFormDataAppendable() const override;
  void AppendToFormData(FormData&) const override;
  String ResultForDialogSubmit() const override;
  bool SupportsValidation() const override;
  LayoutObject* CreateLayoutObject(const ComputedStyle&) const override;
  void HandleDOMActivateEvent(Event*) override;
  void AltAttributeChanged() override;
  void SrcAttributeChanged() override;
  void ValueAttributeChanged() override;
  void StartResourceLoading() override;
  bool ShouldRespectAlignAttribute() override;
  bool CanBeSuccessfulSubmitButton() override;
  bool IsEnumeratable() override;
  bool ShouldRespectHeightAndWidthAttributes() override;
  unsigned Height() const override;
  unsigned Width() const override;
  bool HasLegalLinkAttribute(const QualifiedName&) const override;
  const QualifiedName& SubResourceAttributeName() const override;
  void EnsureFallbackContent() override;
  void EnsurePrimaryContent() override;
  void CreateShadowSubtree() override;

  void ReattachFallbackContent();
  void SetUseFallbackContent();
  bool HasFallbackContent() const { return use_fallback_content_; }

  // Valid only during HTMLFormElement::prepareForSubmission().
  IntPoint click_location_;

  bool use_fallback_content_;
};

}  // namespace blink

#endif  // ImageInputType_h
