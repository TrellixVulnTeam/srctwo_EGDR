/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2003-2008, 2011, 2012, 2014 Apple Inc. All rights reserved.
 * Copyright (C) 2014 Samsung Electronics. All rights reserved.
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

#include "core/html/HTMLCollection.h"

#include "core/HTMLNames.h"
#include "core/dom/ClassCollection.h"
#include "core/dom/ElementTraversal.h"
#include "core/dom/NodeRareData.h"
#include "core/html/DocumentNameCollection.h"
#include "core/html/HTMLDataListOptionsCollection.h"
#include "core/html/HTMLElement.h"
#include "core/html/HTMLFormControlElement.h"
#include "core/html/HTMLObjectElement.h"
#include "core/html/HTMLOptionElement.h"
#include "core/html/HTMLOptionsCollection.h"
#include "core/html/HTMLTagCollection.h"
#include "core/html/WindowNameCollection.h"
#include "platform/wtf/HashSet.h"

namespace blink {

using namespace HTMLNames;

static bool ShouldTypeOnlyIncludeDirectChildren(CollectionType type) {
  switch (type) {
    case kClassCollectionType:
    case kTagCollectionType:
    case kTagCollectionNSType:
    case kHTMLTagCollectionType:
    case kDocAll:
    case kDocAnchors:
    case kDocApplets:
    case kDocEmbeds:
    case kDocForms:
    case kDocImages:
    case kDocLinks:
    case kDocScripts:
    case kDocumentNamedItems:
    case kMapAreas:
    case kTableRows:
    case kSelectOptions:
    case kSelectedOptions:
    case kDataListOptions:
    case kWindowNamedItems:
    case kFormControls:
      return false;
    case kNodeChildren:
    case kTRCells:
    case kTSectionRows:
    case kTableTBodies:
      return true;
    case kNameNodeListType:
    case kRadioNodeListType:
    case kRadioImgNodeListType:
    case kLabelsNodeListType:
      break;
  }
  NOTREACHED();
  return false;
}

static NodeListRootType RootTypeFromCollectionType(const ContainerNode& owner,
                                                   CollectionType type) {
  switch (type) {
    case kDocImages:
    case kDocApplets:
    case kDocEmbeds:
    case kDocForms:
    case kDocLinks:
    case kDocAnchors:
    case kDocScripts:
    case kDocAll:
    case kWindowNamedItems:
    case kDocumentNamedItems:
      return NodeListRootType::kTreeScope;
    case kClassCollectionType:
    case kTagCollectionType:
    case kTagCollectionNSType:
    case kHTMLTagCollectionType:
    case kNodeChildren:
    case kTableTBodies:
    case kTSectionRows:
    case kTableRows:
    case kTRCells:
    case kSelectOptions:
    case kSelectedOptions:
    case kDataListOptions:
    case kMapAreas:
      return NodeListRootType::kNode;
    case kFormControls:
      if (isHTMLFieldSetElement(owner))
        return NodeListRootType::kNode;
      DCHECK(isHTMLFormElement(owner));
      return NodeListRootType::kTreeScope;
    case kNameNodeListType:
    case kRadioNodeListType:
    case kRadioImgNodeListType:
    case kLabelsNodeListType:
      break;
  }
  NOTREACHED();
  return NodeListRootType::kNode;
}

static NodeListInvalidationType InvalidationTypeExcludingIdAndNameAttributes(
    CollectionType type) {
  switch (type) {
    case kTagCollectionType:
    case kTagCollectionNSType:
    case kHTMLTagCollectionType:
    case kDocImages:
    case kDocEmbeds:
    case kDocForms:
    case kDocScripts:
    case kDocAll:
    case kNodeChildren:
    case kTableTBodies:
    case kTSectionRows:
    case kTableRows:
    case kTRCells:
    case kSelectOptions:
    case kMapAreas:
      return kDoNotInvalidateOnAttributeChanges;
    case kDocApplets:
    case kSelectedOptions:
    case kDataListOptions:
      // FIXME: We can do better some day.
      return kInvalidateOnAnyAttrChange;
    case kDocAnchors:
      return kInvalidateOnNameAttrChange;
    case kDocLinks:
      return kInvalidateOnHRefAttrChange;
    case kWindowNamedItems:
      return kInvalidateOnIdNameAttrChange;
    case kDocumentNamedItems:
      return kInvalidateOnIdNameAttrChange;
    case kFormControls:
      return kInvalidateForFormControls;
    case kClassCollectionType:
      return kInvalidateOnClassAttrChange;
    case kNameNodeListType:
    case kRadioNodeListType:
    case kRadioImgNodeListType:
    case kLabelsNodeListType:
      break;
  }
  NOTREACHED();
  return kDoNotInvalidateOnAttributeChanges;
}

HTMLCollection::HTMLCollection(ContainerNode& owner_node,
                               CollectionType type,
                               ItemAfterOverrideType item_after_override_type)
    : LiveNodeListBase(owner_node,
                       RootTypeFromCollectionType(owner_node, type),
                       InvalidationTypeExcludingIdAndNameAttributes(type),
                       type),
      overrides_item_after_(item_after_override_type == kOverridesItemAfter),
      should_only_include_direct_children_(
          ShouldTypeOnlyIncludeDirectChildren(type)) {
  // Keep this in the child class because |registerNodeList| requires wrapper
  // tracing and potentially calls virtual methods which is not allowed in a
  // base class constructor.
  GetDocument().RegisterNodeList(this);
}

HTMLCollection* HTMLCollection::Create(ContainerNode& base,
                                       CollectionType type) {
  return new HTMLCollection(base, type, kDoesNotOverrideItemAfter);
}

HTMLCollection::~HTMLCollection() {}

void HTMLCollection::InvalidateCache(Document* old_document) const {
  collection_items_cache_.Invalidate();
  InvalidateIdNameCacheMaps(old_document);
}

unsigned HTMLCollection::length() const {
  return collection_items_cache_.NodeCount(*this);
}

Element* HTMLCollection::item(unsigned offset) const {
  return collection_items_cache_.NodeAt(*this, offset);
}

static inline bool IsMatchingHTMLElement(const HTMLCollection& html_collection,
                                         const HTMLElement& element) {
  switch (html_collection.GetType()) {
    case kDocImages:
      return element.HasTagName(imgTag);
    case kDocScripts:
      return element.HasTagName(scriptTag);
    case kDocForms:
      return element.HasTagName(formTag);
    case kDocumentNamedItems:
      return ToDocumentNameCollection(html_collection).ElementMatches(element);
    case kTableTBodies:
      return element.HasTagName(tbodyTag);
    case kTRCells:
      return element.HasTagName(tdTag) || element.HasTagName(thTag);
    case kTSectionRows:
      return element.HasTagName(trTag);
    case kSelectOptions:
      return ToHTMLOptionsCollection(html_collection).ElementMatches(element);
    case kSelectedOptions:
      return isHTMLOptionElement(element) &&
             toHTMLOptionElement(element).Selected();
    case kDataListOptions:
      return ToHTMLDataListOptionsCollection(html_collection)
          .ElementMatches(element);
    case kMapAreas:
      return element.HasTagName(areaTag);
    case kDocApplets:
      return isHTMLObjectElement(element) &&
             toHTMLObjectElement(element).ContainsJavaApplet();
    case kDocEmbeds:
      return element.HasTagName(embedTag);
    case kDocLinks:
      return (element.HasTagName(aTag) || element.HasTagName(areaTag)) &&
             element.FastHasAttribute(hrefAttr);
    case kDocAnchors:
      return element.HasTagName(aTag) && element.FastHasAttribute(nameAttr);
    case kFormControls:
      DCHECK(isHTMLFieldSetElement(html_collection.ownerNode()));
      return isHTMLObjectElement(element) || IsHTMLFormControlElement(element);
    case kClassCollectionType:
    case kTagCollectionType:
    case kTagCollectionNSType:
    case kHTMLTagCollectionType:
    case kDocAll:
    case kNodeChildren:
    case kTableRows:
    case kWindowNamedItems:
    case kNameNodeListType:
    case kRadioNodeListType:
    case kRadioImgNodeListType:
    case kLabelsNodeListType:
      NOTREACHED();
  }
  return false;
}

inline bool HTMLCollection::ElementMatches(const Element& element) const {
  // These collections apply to any kind of Elements, not just HTMLElements.
  switch (GetType()) {
    case kDocAll:
    case kNodeChildren:
      return true;
    case kClassCollectionType:
      return ToClassCollection(*this).ElementMatches(element);
    case kTagCollectionType:
      return ToTagCollection(*this).ElementMatches(element);
    case kHTMLTagCollectionType:
      return ToHTMLTagCollection(*this).ElementMatches(element);
    case kTagCollectionNSType:
      return ToTagCollectionNS(*this).ElementMatches(element);
    case kWindowNamedItems:
      return ToWindowNameCollection(*this).ElementMatches(element);
    default:
      break;
  }

  // The following only applies to HTMLElements.
  return element.IsHTMLElement() &&
         IsMatchingHTMLElement(*this, ToHTMLElement(element));
}

namespace {

template <class HTMLCollectionType>
class IsMatch {
  STACK_ALLOCATED();

 public:
  IsMatch(const HTMLCollectionType& list) : list_(&list) {}

  bool operator()(const Element& element) const {
    return list_->ElementMatches(element);
  }

 private:
  Member<const HTMLCollectionType> list_;
};

}  // namespace

template <class HTMLCollectionType>
static inline IsMatch<HTMLCollectionType> MakeIsMatch(
    const HTMLCollectionType& list) {
  return IsMatch<HTMLCollectionType>(list);
}

Element* HTMLCollection::VirtualItemAfter(Element*) const {
  NOTREACHED();
  return nullptr;
}

// https://html.spec.whatwg.org/multipage/infrastructure.html#all-named-elements
// The document.all collection returns only certain types of elements by name,
// although it returns any type of element by id.
static inline bool NameShouldBeVisibleInDocumentAll(
    const HTMLElement& element) {
  return element.HasTagName(aTag) || element.HasTagName(appletTag) ||
         element.HasTagName(buttonTag) || element.HasTagName(embedTag) ||
         element.HasTagName(formTag) || element.HasTagName(frameTag) ||
         element.HasTagName(framesetTag) || element.HasTagName(iframeTag) ||
         element.HasTagName(imgTag) || element.HasTagName(inputTag) ||
         element.HasTagName(mapTag) || element.HasTagName(metaTag) ||
         element.HasTagName(objectTag) || element.HasTagName(selectTag) ||
         element.HasTagName(textareaTag);
}

Element* HTMLCollection::TraverseToFirst() const {
  switch (GetType()) {
    case kHTMLTagCollectionType:
      return ElementTraversal::FirstWithin(
          RootNode(), MakeIsMatch(ToHTMLTagCollection(*this)));
    case kClassCollectionType:
      return ElementTraversal::FirstWithin(
          RootNode(), MakeIsMatch(ToClassCollection(*this)));
    default:
      if (OverridesItemAfter())
        return VirtualItemAfter(0);
      if (ShouldOnlyIncludeDirectChildren())
        return ElementTraversal::FirstChild(RootNode(), MakeIsMatch(*this));
      return ElementTraversal::FirstWithin(RootNode(), MakeIsMatch(*this));
  }
}

Element* HTMLCollection::TraverseToLast() const {
  DCHECK(CanTraverseBackward());
  if (ShouldOnlyIncludeDirectChildren())
    return ElementTraversal::LastChild(RootNode(), MakeIsMatch(*this));
  return ElementTraversal::LastWithin(RootNode(), MakeIsMatch(*this));
}

Element* HTMLCollection::TraverseForwardToOffset(
    unsigned offset,
    Element& current_element,
    unsigned& current_offset) const {
  DCHECK_LT(current_offset, offset);
  switch (GetType()) {
    case kHTMLTagCollectionType:
      return TraverseMatchingElementsForwardToOffset(
          current_element, &RootNode(), offset, current_offset,
          MakeIsMatch(ToHTMLTagCollection(*this)));
    case kClassCollectionType:
      return TraverseMatchingElementsForwardToOffset(
          current_element, &RootNode(), offset, current_offset,
          MakeIsMatch(ToClassCollection(*this)));
    default:
      if (OverridesItemAfter()) {
        for (Element* next = VirtualItemAfter(&current_element); next;
             next = VirtualItemAfter(next)) {
          if (++current_offset == offset)
            return next;
        }
        return nullptr;
      }
      if (ShouldOnlyIncludeDirectChildren()) {
        IsMatch<HTMLCollection> is_match(*this);
        for (Element* next =
                 ElementTraversal::NextSibling(current_element, is_match);
             next; next = ElementTraversal::NextSibling(*next, is_match)) {
          if (++current_offset == offset)
            return next;
        }
        return nullptr;
      }
      return TraverseMatchingElementsForwardToOffset(
          current_element, &RootNode(), offset, current_offset,
          MakeIsMatch(*this));
  }
}

Element* HTMLCollection::TraverseBackwardToOffset(
    unsigned offset,
    Element& current_element,
    unsigned& current_offset) const {
  DCHECK_GT(current_offset, offset);
  DCHECK(CanTraverseBackward());
  if (ShouldOnlyIncludeDirectChildren()) {
    IsMatch<HTMLCollection> is_match(*this);
    for (Element* previous =
             ElementTraversal::PreviousSibling(current_element, is_match);
         previous;
         previous = ElementTraversal::PreviousSibling(*previous, is_match)) {
      if (--current_offset == offset)
        return previous;
    }
    return nullptr;
  }
  return TraverseMatchingElementsBackwardToOffset(
      current_element, &RootNode(), offset, current_offset, MakeIsMatch(*this));
}

Element* HTMLCollection::namedItem(const AtomicString& name) const {
  // http://msdn.microsoft.com/workshop/author/dhtml/reference/methods/nameditem.asp
  // This method first searches for an object with a matching id
  // attribute. If a match is not found, the method then searches for an
  // object with a matching name attribute, but only on those elements
  // that are allowed a name attribute.
  UpdateIdNameCache();

  const NamedItemCache& cache = GetNamedItemCache();
  const auto* id_results = cache.GetElementsById(name);
  if (id_results && !id_results->IsEmpty())
    return id_results->front();

  const auto* name_results = cache.GetElementsByName(name);
  if (name_results && !name_results->IsEmpty())
    return name_results->front();

  return nullptr;
}

bool HTMLCollection::NamedPropertyQuery(const AtomicString& name,
                                        ExceptionState&) {
  return namedItem(name);
}

void HTMLCollection::SupportedPropertyNames(Vector<String>& names) {
  // As per the specification (https://dom.spec.whatwg.org/#htmlcollection):
  // The supported property names are the values from the list returned by these
  // steps:
  // 1. Let result be an empty list.
  // 2. For each element represented by the collection, in tree order, run these
  //    substeps:
  //   1. If element has an ID which is neither the empty string nor is in
  //      result, append element's ID to result.
  //   2. If element is in the HTML namespace and has a name attribute whose
  //      value is neither the empty string nor is in result, append element's
  //      name attribute value to result.
  // 3. Return result.
  HashSet<AtomicString> existing_names;
  unsigned length = this->length();
  for (unsigned i = 0; i < length; ++i) {
    Element* element = item(i);
    const AtomicString& id_attribute = element->GetIdAttribute();
    if (!id_attribute.IsEmpty()) {
      HashSet<AtomicString>::AddResult add_result =
          existing_names.insert(id_attribute);
      if (add_result.is_new_entry)
        names.push_back(id_attribute);
    }
    if (!element->IsHTMLElement())
      continue;
    const AtomicString& name_attribute = element->GetNameAttribute();
    if (!name_attribute.IsEmpty() &&
        (GetType() != kDocAll ||
         NameShouldBeVisibleInDocumentAll(ToHTMLElement(*element)))) {
      HashSet<AtomicString>::AddResult add_result =
          existing_names.insert(name_attribute);
      if (add_result.is_new_entry)
        names.push_back(name_attribute);
    }
  }
}

void HTMLCollection::NamedPropertyEnumerator(Vector<String>& names,
                                             ExceptionState&) {
  SupportedPropertyNames(names);
}

void HTMLCollection::UpdateIdNameCache() const {
  if (HasValidIdNameCache())
    return;

  NamedItemCache* cache = NamedItemCache::Create();
  unsigned length = this->length();
  for (unsigned i = 0; i < length; ++i) {
    Element* element = item(i);
    const AtomicString& id_attr_val = element->GetIdAttribute();
    if (!id_attr_val.IsEmpty())
      cache->AddElementWithId(id_attr_val, element);
    if (!element->IsHTMLElement())
      continue;
    const AtomicString& name_attr_val = element->GetNameAttribute();
    if (!name_attr_val.IsEmpty() && id_attr_val != name_attr_val &&
        (GetType() != kDocAll ||
         NameShouldBeVisibleInDocumentAll(ToHTMLElement(*element))))
      cache->AddElementWithName(name_attr_val, element);
  }
  // Set the named item cache last as traversing the tree may cause cache
  // invalidation.
  SetNamedItemCache(cache);
}

void HTMLCollection::NamedItems(const AtomicString& name,
                                HeapVector<Member<Element>>& result) const {
  DCHECK(result.IsEmpty());
  if (name.IsEmpty())
    return;

  UpdateIdNameCache();

  const NamedItemCache& cache = GetNamedItemCache();
  if (const auto* id_results = cache.GetElementsById(name))
    result.AppendVector(*id_results);
  if (const auto* name_results = cache.GetElementsByName(name))
    result.AppendVector(*name_results);
}

HTMLCollection::NamedItemCache::NamedItemCache() {}

DEFINE_TRACE(HTMLCollection) {
  visitor->Trace(named_item_cache_);
  visitor->Trace(collection_items_cache_);
  LiveNodeListBase::Trace(visitor);
}

}  // namespace blink
