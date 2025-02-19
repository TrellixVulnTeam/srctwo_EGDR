/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Simon Hausmann <hausmann@kde.org>
 * Copyright (C) 2007, 2008, 2009, 2010 Apple Inc. All rights reserved.
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

#ifndef HTMLAnchorElement_h
#define HTMLAnchorElement_h

#include "core/CoreExport.h"
#include "core/HTMLNames.h"
#include "core/dom/Document.h"
#include "core/html/HTMLElement.h"
#include "core/url/DOMURLUtils.h"
#include "platform/LinkHash.h"

namespace blink {

// Link relation bitmask values.
// FIXME: Uncomment as the various link relations are implemented.
enum {
  //     RelationAlternate   = 0x00000001,
  //     RelationArchives    = 0x00000002,
  //     RelationAuthor      = 0x00000004,
  //     RelationBoomark     = 0x00000008,
  //     RelationExternal    = 0x00000010,
  //     RelationFirst       = 0x00000020,
  //     RelationHelp        = 0x00000040,
  //     RelationIndex       = 0x00000080,
  //     RelationLast        = 0x00000100,
  //     RelationLicense     = 0x00000200,
  //     RelationNext        = 0x00000400,
  //     RelationNoFolow    = 0x00000800,
  kRelationNoReferrer = 0x00001000,
  //     RelationPrev        = 0x00002000,
  //     RelationSearch      = 0x00004000,
  //     RelationSidebar     = 0x00008000,
  //     RelationTag         = 0x00010000,
  //     RelationUp          = 0x00020000,
  kRelationNoOpener = 0x00040000,
};

class CORE_EXPORT HTMLAnchorElement : public HTMLElement, public DOMURLUtils {
  DEFINE_WRAPPERTYPEINFO();

 public:
  static HTMLAnchorElement* Create(Document&);

  ~HTMLAnchorElement() override;

  KURL Href() const;
  void SetHref(const AtomicString&);

  const AtomicString& GetName() const;

  KURL Url() const final;
  void SetURL(const KURL&) final;

  String Input() const final;
  void SetInput(const String&) final;

  bool IsLiveLink() const final;

  bool WillRespondToMouseClickEvents() final;

  bool HasRel(uint32_t relation) const;
  void SetRel(const AtomicString&);

  LinkHash VisitedLinkHash() const;
  void InvalidateCachedVisitedLinkHash() { cached_visited_link_hash_ = 0; }

  void SendPings(const KURL& destination_url) const;

 protected:
  HTMLAnchorElement(const QualifiedName&, Document&);

  void ParseAttribute(const AttributeModificationParams&) override;
  bool SupportsFocus() const override;
  bool MatchesEnabledPseudoClass() const override;

 private:
  void AttributeChanged(const AttributeModificationParams&) override;
  bool ShouldHaveFocusAppearance() const final;
  void DispatchFocusEvent(
      Element* old_focused_element,
      WebFocusType,
      InputDeviceCapabilities* source_capabilities) override;
  void DispatchBlurEvent(Element* new_focused_element,
                         WebFocusType,
                         InputDeviceCapabilities* source_capabilities) override;
  bool IsMouseFocusable() const override;
  bool IsKeyboardFocusable() const override;
  void DefaultEventHandler(Event*) final;
  bool HasActivationBehavior() const override;
  void SetActive(bool = true) final;
  void AccessKeyAction(bool send_mouse_events) final;
  bool IsURLAttribute(const Attribute&) const final;
  bool HasLegalLinkAttribute(const QualifiedName&) const final;
  bool CanStartSelection() const final;
  int tabIndex() const final;
  bool draggable() const final;
  bool IsInteractiveContent() const final;
  InsertionNotificationRequest InsertedInto(ContainerNode*) override;
  void HandleClick(Event*);

  uint32_t link_relations_;
  mutable LinkHash cached_visited_link_hash_;
  bool was_focused_by_mouse_;
};

inline LinkHash HTMLAnchorElement::VisitedLinkHash() const {
  if (!cached_visited_link_hash_)
    cached_visited_link_hash_ = blink::VisitedLinkHash(
        GetDocument().BaseURL(), FastGetAttribute(HTMLNames::hrefAttr));
  return cached_visited_link_hash_;
}

// Functions shared with the other anchor elements (i.e., SVG).

bool IsEnterKeyKeydownEvent(Event*);
bool IsLinkClick(Event*);

}  // namespace blink

#endif  // HTMLAnchorElement_h
