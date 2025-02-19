// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_URL_PATTERN_INDEX_URL_PATTERN_INDEX_H_
#define COMPONENTS_URL_PATTERN_INDEX_URL_PATTERN_INDEX_H_

#include <stddef.h>
#include <stdint.h>

#include <vector>

#include "base/macros.h"
#include "base/strings/string_piece_forward.h"
#include "components/url_pattern_index/closed_hash_map.h"
#include "components/url_pattern_index/flat/url_pattern_index_generated.h"
#include "components/url_pattern_index/proto/rules.pb.h"
#include "components/url_pattern_index/uint64_hasher.h"
#include "third_party/flatbuffers/src/include/flatbuffers/flatbuffers.h"

class GURL;

namespace url {
class Origin;
}

namespace url_pattern_index {

// The integer type used to represent N-grams.
using NGram = uint64_t;
// The hasher used for hashing N-grams.
using NGramHasher = Uint64Hasher;
// The hash table probe sequence used both by UrlPatternIndex and its builder.
using NGramHashTableProber = DefaultProber<NGram, NGramHasher>;

// FlatBuffer offset aliases.
using UrlRuleOffset = flatbuffers::Offset<flat::UrlRule>;
using UrlPatternIndexOffset = flatbuffers::Offset<flat::UrlPatternIndex>;

constexpr size_t kNGramSize = 5;
static_assert(kNGramSize <= sizeof(NGram), "NGram type is too narrow.");

// Serializes the |rule| to the FlatBuffer |builder|, and returns an offset to
// it in the resulting buffer. Returns null offset iff the |rule| could not be
// serialized because of unsupported options or it is otherwise invalid.
UrlRuleOffset SerializeUrlRule(const proto::UrlRule& rule,
                               flatbuffers::FlatBufferBuilder* builder);

// Performs three-way comparison between two domains. In the total order defined
// by this predicate, the lengths of domains will be monotonically decreasing.
// Domains of same length are ordered in lexicographic order.
// Returns a negative value if |lhs_domain| should be ordered before
// |rhs_domain|, zero if |lhs_domain| is equal to |rhs_domain| and a positive
// value if |lhs_domain| should be ordered after |rhs_domain|.
int CompareDomains(base::StringPiece lhs_domain, base::StringPiece rhs_domain);

// The class used to construct an index over the URL patterns of a set of URL
// rules. The rules themselves need to be converted to FlatBuffers format by the
// client of this class, as well as persisted into the |flat_builder| that is
// supplied in the constructor.
class UrlPatternIndexBuilder {
 public:
  explicit UrlPatternIndexBuilder(flatbuffers::FlatBufferBuilder* flat_builder);
  ~UrlPatternIndexBuilder();

  // Adds a UrlRule to the index. The caller should have already persisted the
  // rule into the same |flat_builder| by a call to SerializeUrlRule returning a
  // non-null |offset|, and should pass in the resulting |offset| here.
  void IndexUrlRule(UrlRuleOffset offset);

  // Finalizes construction of the index, serializes it using |flat_builder|,
  // and returns an offset to it in the resulting FlatBuffer.
  UrlPatternIndexOffset Finish();

 private:
  using MutableUrlRuleList = std::vector<UrlRuleOffset>;
  using MutableNGramIndex =
      ClosedHashMap<NGram, MutableUrlRuleList, NGramHashTableProber>;

  // Returns an N-gram of the |pattern| encoded into the NGram integer type. The
  // N-gram is picked using a greedy heuristic, i.e. the one is chosen which
  // corresponds to the shortest list of rules within the index. If there are no
  // valid N-grams in the |pattern|, the return value is 0.
  NGram GetMostDistinctiveNGram(base::StringPiece pattern);

  // This index contains all non-REGEXP rules that have at least one acceptable
  // N-gram. For each given rule, the N-gram used as an index key is picked
  // greedily (see GetMostDistinctiveNGram).
  MutableNGramIndex ngram_index_;

  // A fallback list that contains all the rules with no acceptable N-gram.
  MutableUrlRuleList fallback_rules_;

  // Must outlive this instance.
  flatbuffers::FlatBufferBuilder* flat_builder_;

  DISALLOW_COPY_AND_ASSIGN(UrlPatternIndexBuilder);
};

// Encapsulates a read-only index built over the URL patterns of a set of URL
// rules, and provides fast matching of network requests against these rules.
class UrlPatternIndexMatcher {
 public:
  // Creates an instance to access the given |flat_index|. If |flat_index| is
  // nullptr, then all requests return no match.
  explicit UrlPatternIndexMatcher(const flat::UrlPatternIndex* flat_index);
  ~UrlPatternIndexMatcher();

  // If the index contains one or more UrlRules that match the request, returns
  // one of them (it is undefined which one). Otherwise, returns nullptr.
  //
  // Notes on parameters:
  //  - |url| should be valid, otherwise the return value is nullptr.
  //  - Exactly one of |element_type| and |activation_type| should be specified,
  //    i.e., not equal to *_UNSPECIFIED, otherwise the return value is nullptr.
  //  - |is_third_party| should be pre-computed by the caller, e.g. using the
  //    registry_controlled_domains library, to reflect the relation between
  //    |url| and |first_party_origin|.
  //
  // A rule is deemed to match the request iff all of the following applies:
  //  - The |url| matches the rule's UrlPattern (see url_pattern.h).
  //  - The |first_party_origin| matches the rule's targeted domains list.
  //  - |element_type| or |activation_type| is among the rule's targeted types.
  //  - The |is_third_party| bit matches the rule's requirement on the requested
  //    |url| being first-/third-party w.r.t. its |first_party_origin|.
  //  - The rule is not generic if |disable_generic_rules| is true.
  const flat::UrlRule* FindMatch(const GURL& url,
                                 const url::Origin& first_party_origin,
                                 proto::ElementType element_type,
                                 proto::ActivationType activation_type,
                                 bool is_third_party,
                                 bool disable_generic_rules) const;

 private:
  // Helper function to work with flat::*Type(s). If the index contains one or
  // more UrlRules that match the request, returns one of them (it is undefined
  // which one). Otherwise, returns nullptr.
  const flat::UrlRule* FindMatch(const GURL& url,
                                 const url::Origin& first_party_origin,
                                 flat::ElementType element_type,
                                 flat::ActivationType activation_type,
                                 bool is_third_party,
                                 bool disable_generic_rules) const;

  // Must outlive this instance.
  const flat::UrlPatternIndex* flat_index_;

  DISALLOW_COPY_AND_ASSIGN(UrlPatternIndexMatcher);
};

}  // namespace url_pattern_index

#endif  // COMPONENTS_URL_PATTERN_INDEX_URL_PATTERN_INDEX_H_
