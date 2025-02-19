/*
 * Copyright (C) 2005, 2006, 2009 Apple Inc. All rights reserved.
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
 */

#include "core/dom/QualifiedName.h"

#include "core/HTMLNames.h"
#include "core/MathMLNames.h"
#include "core/SVGNames.h"
#include "core/XLinkNames.h"
#include "core/XMLNSNames.h"
#include "core/XMLNames.h"
#include "platform/wtf/Assertions.h"
#include "platform/wtf/HashSet.h"
#include "platform/wtf/StaticConstructors.h"

namespace blink {

struct SameSizeAsQualifiedNameImpl
    : public RefCounted<SameSizeAsQualifiedNameImpl> {
  unsigned bitfield;
  void* pointers[4];
};

static_assert(sizeof(QualifiedName::QualifiedNameImpl) ==
                  sizeof(SameSizeAsQualifiedNameImpl),
              "QualifiedNameImpl should stay small");

using QualifiedNameCache =
    HashSet<QualifiedName::QualifiedNameImpl*, QualifiedNameHash>;

static QualifiedNameCache& GetQualifiedNameCache() {
  // This code is lockless and thus assumes it all runs on one thread!
  DCHECK(IsMainThread());
  static QualifiedNameCache* g_name_cache = new QualifiedNameCache;
  return *g_name_cache;
}

struct QNameComponentsTranslator {
  static unsigned GetHash(const QualifiedNameData& data) {
    return HashComponents(data.components_);
  }
  static bool Equal(QualifiedName::QualifiedNameImpl* name,
                    const QualifiedNameData& data) {
    return data.components_.prefix_ == name->prefix_.Impl() &&
           data.components_.local_name_ == name->local_name_.Impl() &&
           data.components_.namespace_ == name->namespace_.Impl();
  }
  static void Translate(QualifiedName::QualifiedNameImpl*& location,
                        const QualifiedNameData& data,
                        unsigned) {
    const QualifiedNameComponents& components = data.components_;
    location = QualifiedName::QualifiedNameImpl::Create(
                   AtomicString(components.prefix_),
                   AtomicString(components.local_name_),
                   AtomicString(components.namespace_), data.is_static_)
                   .LeakRef();
  }
};

QualifiedName::QualifiedName(const AtomicString& p,
                             const AtomicString& l,
                             const AtomicString& n) {
  QualifiedNameData data = {
      {p.Impl(), l.Impl(), n.IsEmpty() ? g_null_atom.Impl() : n.Impl()}, false};
  QualifiedNameCache::AddResult add_result =
      GetQualifiedNameCache().AddWithTranslator<QNameComponentsTranslator>(
          data);
  impl_ = add_result.is_new_entry ? AdoptRef(*add_result.stored_value)
                                  : *add_result.stored_value;
}

QualifiedName::QualifiedName(const AtomicString& p,
                             const AtomicString& l,
                             const AtomicString& n,
                             bool is_static) {
  QualifiedNameData data = {{p.Impl(), l.Impl(), n.Impl()}, is_static};
  QualifiedNameCache::AddResult add_result =
      GetQualifiedNameCache().AddWithTranslator<QNameComponentsTranslator>(
          data);
  impl_ = add_result.is_new_entry ? AdoptRef(*add_result.stored_value)
                                  : *add_result.stored_value;
}

QualifiedName::~QualifiedName() {}

QualifiedName::QualifiedNameImpl::~QualifiedNameImpl() {
  GetQualifiedNameCache().erase(this);
}

String QualifiedName::ToString() const {
  String local = LocalName();
  if (HasPrefix())
    return Prefix().GetString() + ":" + local;
  return local;
}

// Global init routines
DEFINE_GLOBAL(QualifiedName, g_any_name);
DEFINE_GLOBAL(QualifiedName, g_null_name);

void QualifiedName::InitAndReserveCapacityForSize(unsigned size) {
  DCHECK(g_star_atom.Impl());
  GetQualifiedNameCache().ReserveCapacityForSize(size +
                                                 2 /*starAtom and nullAtom */);
  new ((void*)&g_any_name)
      QualifiedName(g_null_atom, g_star_atom, g_star_atom, true);
  new ((void*)&g_null_name)
      QualifiedName(g_null_atom, g_null_atom, g_null_atom, true);
}

const AtomicString& QualifiedName::LocalNameUpper() const {
  if (!impl_->local_name_upper_)
    impl_->local_name_upper_ = impl_->local_name_.UpperASCII();
  return impl_->local_name_upper_;
}

unsigned QualifiedName::QualifiedNameImpl::ComputeHash() const {
  QualifiedNameComponents components = {prefix_.Impl(), local_name_.Impl(),
                                        namespace_.Impl()};
  return HashComponents(components);
}

void QualifiedName::CreateStatic(void* target_address,
                                 StringImpl* name,
                                 const AtomicString& name_namespace) {
  new (target_address)
      QualifiedName(g_null_atom, AtomicString(name), name_namespace, true);
}

void QualifiedName::CreateStatic(void* target_address, StringImpl* name) {
  new (target_address)
      QualifiedName(g_null_atom, AtomicString(name), g_null_atom, true);
}

}  // namespace blink
