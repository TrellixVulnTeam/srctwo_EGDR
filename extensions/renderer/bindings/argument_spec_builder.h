// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXTENSIONS_RENDERER_BINDINGS_ARGUMENT_SPEC_BUILDER_H_
#define EXTENSIONS_RENDERER_BINDINGS_ARGUMENT_SPEC_BUILDER_H_

#include <memory>
#include <set>
#include <vector>

#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "extensions/renderer/bindings/argument_spec.h"

namespace extensions {

// A utility class for helping construct ArgumentSpecs in tests.
// NOTE: This is designed to be test-only. It's not worth adding to production
// code because it's a) only a bit of syntactic sugar and b) rife with footguns.
class ArgumentSpecBuilder {
 public:
  explicit ArgumentSpecBuilder(ArgumentType type);
  ArgumentSpecBuilder(ArgumentType type, base::StringPiece name);

  ~ArgumentSpecBuilder();

  ArgumentSpecBuilder& MakeOptional();
  ArgumentSpecBuilder& AddProperty(base::StringPiece property_name,
                                   std::unique_ptr<ArgumentSpec> property_spec);
  ArgumentSpecBuilder& SetMinimum(int minimum);
  ArgumentSpecBuilder& SetListType(std::unique_ptr<ArgumentSpec> list_type);
  ArgumentSpecBuilder& SetRef(base::StringPiece ref);
  ArgumentSpecBuilder& SetChoices(
      std::vector<std::unique_ptr<ArgumentSpec>> choices);
  ArgumentSpecBuilder& SetEnums(std::set<std::string> enum_values);
  ArgumentSpecBuilder& SetAdditionalProperties(
      std::unique_ptr<ArgumentSpec> additional_properties);
  std::unique_ptr<ArgumentSpec> Build();

 private:
  std::unique_ptr<ArgumentSpec> spec_;
  ArgumentSpec::PropertiesMap properties_;

  DISALLOW_COPY_AND_ASSIGN(ArgumentSpecBuilder);
};

}  // namespace extensions

#endif  // EXTENSIONS_RENDERER_BINDINGS_ARGUMENT_SPEC_BUILDER_H_
