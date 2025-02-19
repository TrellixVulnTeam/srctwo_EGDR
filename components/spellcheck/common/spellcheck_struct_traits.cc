// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/spellcheck/common/spellcheck_struct_traits.h"

namespace mojo {

spellcheck::mojom::Decoration
EnumTraits<spellcheck::mojom::Decoration, SpellCheckResult::Decoration>::
    ToMojom(SpellCheckResult::Decoration decoration) {
  switch (decoration) {
    case SpellCheckResult::SPELLING:
      return spellcheck::mojom::Decoration::kSpelling;
    case SpellCheckResult::GRAMMAR:
      return spellcheck::mojom::Decoration::kGrammar;
  }
  NOTREACHED();
  return spellcheck::mojom::Decoration::kSpelling;
}

bool EnumTraits<spellcheck::mojom::Decoration, SpellCheckResult::Decoration>::
    FromMojom(spellcheck::mojom::Decoration input,
              SpellCheckResult::Decoration* output) {
  switch (input) {
    case spellcheck::mojom::Decoration::kSpelling:
      *output = SpellCheckResult::SPELLING;
      return true;
    case spellcheck::mojom::Decoration::kGrammar:
      *output = SpellCheckResult::GRAMMAR;
      return true;
  }
  NOTREACHED();
  return false;
}

bool StructTraits<
    spellcheck::mojom::SpellCheckResultDataView,
    SpellCheckResult>::Read(spellcheck::mojom::SpellCheckResultDataView input,
                            SpellCheckResult* output) {
  if (!input.ReadDecoration(&output->decoration))
    return false;
  output->location = input.location();
  output->length = input.length();
  if (!input.ReadReplacements(&output->replacements))
    return false;
  return true;
}

}  // namespace mojo
