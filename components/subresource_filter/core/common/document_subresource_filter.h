// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_SUBRESOURCE_FILTER_CORE_COMMON_DOCUMENT_SUBRESOURCE_FILTER_H_
#define COMPONENTS_SUBRESOURCE_FILTER_CORE_COMMON_DOCUMENT_SUBRESOURCE_FILTER_H_

#include <stddef.h>

#include <memory>
#include <vector>

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "components/subresource_filter/core/common/activation_level.h"
#include "components/subresource_filter/core/common/activation_state.h"
#include "components/subresource_filter/core/common/document_load_statistics.h"
#include "components/subresource_filter/core/common/indexed_ruleset.h"
#include "components/subresource_filter/core/common/load_policy.h"
#include "components/url_pattern_index/proto/rules.pb.h"

class GURL;

namespace url {
class Origin;
}  // namespace url

namespace subresource_filter {

class FirstPartyOrigin;
class MemoryMappedRuleset;

// Performs filtering of subresource loads in the scope of a given document.
class DocumentSubresourceFilter {
 public:
  // Constructs a new filter that will:
  //  -- Operate in a manner prescribed in |activation_state|.
  //  -- Filter subresource loads in the scope of a document loaded from
  //     |document_origin|.
  //  -- Hold a reference to and use |ruleset| for its entire lifetime.
  DocumentSubresourceFilter(url::Origin document_origin,
                            ActivationState activation_state,
                            scoped_refptr<const MemoryMappedRuleset> ruleset);

  ~DocumentSubresourceFilter();

  const ActivationState& activation_state() const { return activation_state_; }
  const DocumentLoadStatistics& statistics() const { return statistics_; }

  // WARNING: This is only to allow DocumentSubresourceFilter's wrappers to
  // modify the |statistics|.
  // TODO(pkalinnikov): Find a better way to achieve this.
  DocumentLoadStatistics& statistics() { return statistics_; }

  LoadPolicy GetLoadPolicy(
      const GURL& subresource_url,
      url_pattern_index::proto::ElementType subresource_type);

 private:
  const ActivationState activation_state_;
  const scoped_refptr<const MemoryMappedRuleset> ruleset_;
  const IndexedRulesetMatcher ruleset_matcher_;

  // Equals nullptr iff |activation_state_.filtering_disabled_for_document|.
  std::unique_ptr<FirstPartyOrigin> document_origin_;

  DocumentLoadStatistics statistics_;

  DISALLOW_COPY_AND_ASSIGN(DocumentSubresourceFilter);
};

}  // namespace subresource_filter

#endif  // COMPONENTS_SUBRESOURCE_FILTER_CORE_COMMON_DOCUMENT_SUBRESOURCE_FILTER_H_
