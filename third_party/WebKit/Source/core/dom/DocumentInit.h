/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
 *           (C) 2006 Alexey Proskuryakov (ap@webkit.org)
 * Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2012 Apple Inc. All
 * rights reserved.
 * Copyright (C) 2008, 2009 Torch Mobile Inc. All rights reserved.
 * (http://www.torchmobile.com/)
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2013 Google Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef DocumentInit_h
#define DocumentInit_h

#include "core/CoreExport.h"
#include "core/dom/SandboxFlags.h"
#include "core/dom/SecurityContext.h"
#include "core/html/custom/V0CustomElementRegistrationContext.h"
#include "platform/heap/Handle.h"
#include "platform/weborigin/KURL.h"
#include "public/platform/WebInsecureRequestPolicy.h"

namespace blink {

class Document;
class LocalFrame;
class HTMLImportsController;
class Settings;

class CORE_EXPORT DocumentInit final {
  STACK_ALLOCATED();

 public:
  // Use either of the following methods to create a DocumentInit instance, and
  // then add a chain of calls to the .WithFooBar() methods to add optional
  // parameters to it.
  //
  // Example:
  //
  //   DocumentInit init = DocumentInit::Create()
  //       .WithFrame(frame)
  //       .WithContextDocument(context_document)
  //       .WithURL(url);
  //   Document* document = Document::Create(init);
  static DocumentInit Create();
  static DocumentInit CreateWithImportsController(HTMLImportsController*);

  DocumentInit(const DocumentInit&);
  ~DocumentInit();

  HTMLImportsController* ImportsController() const {
    return imports_controller_;
  }

  bool HasSecurityContext() const { return FrameForSecurityContext(); }
  bool ShouldTreatURLAsSrcdocDocument() const;
  bool ShouldSetURL() const;
  bool IsSeamlessAllowedFor(Document* child) const;
  SandboxFlags GetSandboxFlags() const;
  bool IsHostedInReservedIPRange() const;
  WebInsecureRequestPolicy GetInsecureRequestPolicy() const;
  SecurityContext::InsecureNavigationsSet* InsecureNavigationsToUpgrade() const;

  KURL ParentBaseURL() const;
  LocalFrame* OwnerFrame() const;
  Settings* GetSettings() const;

  DocumentInit& WithFrame(LocalFrame*);
  LocalFrame* GetFrame() const { return frame_; }

  // Used by the DOMImplementation and DOMParser to pass their parent Document
  // so that the created Document will return the Document when the
  // ContextDocument() method is called.
  DocumentInit& WithContextDocument(Document*);
  Document* ContextDocument() const;

  DocumentInit& WithURL(const KURL&);
  const KURL& Url() const { return url_; }

  // Specifies the Document to inherit security configurations from.
  DocumentInit& WithOwnerDocument(Document*);
  Document* OwnerDocument() const { return owner_document_.Get(); }

  DocumentInit& WithRegistrationContext(V0CustomElementRegistrationContext*);
  V0CustomElementRegistrationContext* RegistrationContext(Document*) const;
  DocumentInit& WithNewRegistrationContext();

 private:
  DocumentInit(HTMLImportsController*);

  LocalFrame* FrameForSecurityContext() const;

  Member<LocalFrame> frame_;
  Member<Document> parent_document_;

  Member<HTMLImportsController> imports_controller_;

  Member<Document> context_document_;
  KURL url_;
  Member<Document> owner_document_;

  Member<V0CustomElementRegistrationContext> registration_context_;
  bool create_new_registration_context_;
};

}  // namespace blink

#endif  // DocumentInit_h
