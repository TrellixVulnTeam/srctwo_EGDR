// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/renderer/bindings/api_type_reference_map.h"

#include "extensions/renderer/bindings/api_signature.h"
#include "extensions/renderer/bindings/argument_spec.h"

namespace extensions {

APITypeReferenceMap::APITypeReferenceMap(
    const InitializeTypeCallback& initialize_type)
    : initialize_type_(initialize_type) {}
APITypeReferenceMap::~APITypeReferenceMap() = default;

void APITypeReferenceMap::AddSpec(const std::string& name,
                                  std::unique_ptr<ArgumentSpec> spec) {
  DCHECK(type_refs_.find(name) == type_refs_.end());
  type_refs_[name] = std::move(spec);
}

const ArgumentSpec* APITypeReferenceMap::GetSpec(
    const std::string& name) const {
  auto iter = type_refs_.find(name);
  if (iter == type_refs_.end()) {
    initialize_type_.Run(name);
    iter = type_refs_.find(name);
  }
  return iter == type_refs_.end() ? nullptr : iter->second.get();
}

void APITypeReferenceMap::AddAPIMethodSignature(
    const std::string& name,
    std::unique_ptr<APISignature> signature) {
  DCHECK(api_methods_.find(name) == api_methods_.end())
      << "Cannot re-register signature for: " << name;
  api_methods_[name] = std::move(signature);
}

const APISignature* APITypeReferenceMap::GetAPIMethodSignature(
    const std::string& name) const {
  auto iter = api_methods_.find(name);
  if (iter == api_methods_.end()) {
    initialize_type_.Run(name);
    iter = api_methods_.find(name);
  }
  return iter == api_methods_.end() ? nullptr : iter->second.get();
}

void APITypeReferenceMap::AddTypeMethodSignature(
    const std::string& name,
    std::unique_ptr<APISignature> signature) {
  DCHECK(type_methods_.find(name) == type_methods_.end())
      << "Cannot re-register signature for: " << name;
  type_methods_[name] = std::move(signature);
}

const APISignature* APITypeReferenceMap::GetTypeMethodSignature(
    const std::string& name) const {
  auto iter = type_methods_.find(name);
  if (iter == type_methods_.end()) {
    // Find the type name by stripping away the method suffix.
    std::string::size_type dot = name.rfind('.');
    DCHECK_NE(std::string::npos, dot);
    DCHECK_LT(dot, name.size() - 1);
    std::string type_name = name.substr(0, dot);
    initialize_type_.Run(type_name);
    iter = type_methods_.find(name);
  }
  return iter == type_methods_.end() ? nullptr : iter->second.get();
}

bool APITypeReferenceMap::HasTypeMethodSignature(
    const std::string& name) const {
  return type_methods_.find(name) != type_methods_.end();
}

}  // namespace extensions
