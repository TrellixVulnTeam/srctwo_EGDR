// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/common/url_pattern_set.h"

#include <stddef.h>

#include <sstream>

#include "base/stl_util.h"
#include "base/values.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "url/gurl.h"

namespace extensions {

namespace {

void AddPattern(URLPatternSet* set, const std::string& pattern) {
  int schemes = URLPattern::SCHEME_ALL;
  set->AddPattern(URLPattern(schemes, pattern));
}

URLPatternSet Patterns(const std::string& pattern) {
  URLPatternSet set;
  AddPattern(&set, pattern);
  return set;
}

URLPatternSet Patterns(const std::string& pattern1,
                       const std::string& pattern2) {
  URLPatternSet set;
  AddPattern(&set, pattern1);
  AddPattern(&set, pattern2);
  return set;
}

}  // namespace

TEST(URLPatternSetTest, Empty) {
  URLPatternSet set;
  EXPECT_FALSE(set.MatchesURL(GURL("http://www.foo.com/bar")));
  EXPECT_FALSE(set.MatchesURL(GURL()));
  EXPECT_FALSE(set.MatchesURL(GURL("invalid")));
}

TEST(URLPatternSetTest, One) {
  URLPatternSet set;
  AddPattern(&set, "http://www.google.com/*");

  EXPECT_TRUE(set.MatchesURL(GURL("http://www.google.com/")));
  EXPECT_TRUE(set.MatchesURL(GURL("http://www.google.com/monkey")));
  EXPECT_FALSE(set.MatchesURL(GURL("https://www.google.com/")));
  EXPECT_FALSE(set.MatchesURL(GURL("https://www.microsoft.com/")));
}

TEST(URLPatternSetTest, Two) {
  URLPatternSet set;
  AddPattern(&set, "http://www.google.com/*");
  AddPattern(&set, "http://www.yahoo.com/*");

  EXPECT_TRUE(set.MatchesURL(GURL("http://www.google.com/monkey")));
  EXPECT_TRUE(set.MatchesURL(GURL("http://www.yahoo.com/monkey")));
  EXPECT_FALSE(set.MatchesURL(GURL("https://www.apple.com/monkey")));
}

TEST(URLPatternSetTest, StreamOperatorEmpty) {
  URLPatternSet set;

  std::ostringstream stream;
  stream << set;
  EXPECT_EQ("{ }", stream.str());
}

TEST(URLPatternSetTest, StreamOperatorOne) {
  URLPatternSet set;
  AddPattern(&set, "http://www.google.com/*");

  std::ostringstream stream;
  stream << set;
  EXPECT_EQ("{ \"http://www.google.com/*\" }", stream.str());
}

TEST(URLPatternSetTest, StreamOperatorTwo) {
  URLPatternSet set;
  AddPattern(&set, "http://www.google.com/*");
  AddPattern(&set, "http://www.yahoo.com/*");

  std::ostringstream stream;
  stream << set;
  EXPECT_EQ("{ \"http://www.google.com/*\", \"http://www.yahoo.com/*\" }",
            stream.str());
}

TEST(URLPatternSetTest, OverlapsWith) {
  URLPatternSet set1;
  AddPattern(&set1, "http://www.google.com/f*");
  AddPattern(&set1, "http://www.yahoo.com/b*");

  URLPatternSet set2;
  AddPattern(&set2, "http://www.reddit.com/f*");
  AddPattern(&set2, "http://www.yahoo.com/z*");

  URLPatternSet set3;
  AddPattern(&set3, "http://www.google.com/q/*");
  AddPattern(&set3, "http://www.yahoo.com/b/*");

  EXPECT_FALSE(set1.OverlapsWith(set2));
  EXPECT_FALSE(set2.OverlapsWith(set1));

  EXPECT_TRUE(set1.OverlapsWith(set3));
  EXPECT_TRUE(set3.OverlapsWith(set1));
}

TEST(URLPatternSetTest, CreateDifference) {
  URLPatternSet expected;
  URLPatternSet set1;
  URLPatternSet set2;
  AddPattern(&set1, "http://www.google.com/f*");
  AddPattern(&set1, "http://www.yahoo.com/b*");

  // Subtract an empty set.
  URLPatternSet result = URLPatternSet::CreateDifference(set1, set2);
  EXPECT_EQ(set1, result);

  // Subtract a real set.
  AddPattern(&set2, "http://www.reddit.com/f*");
  AddPattern(&set2, "http://www.yahoo.com/z*");
  AddPattern(&set2, "http://www.google.com/f*");

  AddPattern(&expected, "http://www.yahoo.com/b*");

  result = URLPatternSet::CreateDifference(set1, set2);
  EXPECT_EQ(expected, result);
  EXPECT_FALSE(result.is_empty());
  EXPECT_TRUE(set1.Contains(result));
  EXPECT_FALSE(result.Contains(set2));
  EXPECT_FALSE(set2.Contains(result));

  URLPatternSet intersection = URLPatternSet::CreateIntersection(result, set2);
  EXPECT_TRUE(intersection.is_empty());
}

TEST(URLPatternSetTest, CreateIntersection) {
  URLPatternSet empty_set;
  URLPatternSet expected;
  URLPatternSet set1;
  AddPattern(&set1, "http://www.google.com/f*");
  AddPattern(&set1, "http://www.yahoo.com/b*");

  // Intersection with an empty set.
  URLPatternSet result = URLPatternSet::CreateIntersection(set1, empty_set);
  EXPECT_EQ(expected, result);
  EXPECT_TRUE(result.is_empty());
  EXPECT_TRUE(empty_set.Contains(result));
  EXPECT_TRUE(result.Contains(empty_set));
  EXPECT_TRUE(set1.Contains(result));

  // Intersection with a real set.
  URLPatternSet set2;
  AddPattern(&set2, "http://www.reddit.com/f*");
  AddPattern(&set2, "http://www.yahoo.com/z*");
  AddPattern(&set2, "http://www.google.com/f*");

  AddPattern(&expected, "http://www.google.com/f*");

  result = URLPatternSet::CreateIntersection(set1, set2);
  EXPECT_EQ(expected, result);
  EXPECT_FALSE(result.is_empty());
  EXPECT_TRUE(set1.Contains(result));
  EXPECT_TRUE(set2.Contains(result));
}

TEST(URLPatternSetTest, CreateSemanticIntersection) {
  {
    URLPatternSet set1;
    AddPattern(&set1, "http://*.google.com/*");
    AddPattern(&set1, "http://*.yahoo.com/*");

    URLPatternSet set2;
    AddPattern(&set2, "http://google.com/*");

    // The semantic intersection should contain only those patterns that are in
    // both set 1 and set 2, or "http://google.com/*".
    URLPatternSet intersection =
        URLPatternSet::CreateSemanticIntersection(set1, set2);
    ASSERT_EQ(1u, intersection.size());
    EXPECT_EQ("http://google.com/*", intersection.begin()->GetAsString());
  }

  {
    // We don't handle funny intersections, where the resultant pattern is
    // neither of the two compared patterns. So the intersection of these two
    // is not http://www.google.com/*, but rather nothing.
    URLPatternSet set1;
    AddPattern(&set1, "http://*/*");
    URLPatternSet set2;
    AddPattern(&set2, "*://www.google.com/*");
    EXPECT_TRUE(
        URLPatternSet::CreateSemanticIntersection(set1, set2).is_empty());
  }
}

TEST(URLPatternSetTest, CreateUnion) {
  URLPatternSet empty_set;

  URLPatternSet set1;
  AddPattern(&set1, "http://www.google.com/f*");
  AddPattern(&set1, "http://www.yahoo.com/b*");

  URLPatternSet expected;
  AddPattern(&expected, "http://www.google.com/f*");
  AddPattern(&expected, "http://www.yahoo.com/b*");

  // Union with an empty set.
  URLPatternSet result = URLPatternSet::CreateUnion(set1, empty_set);
  EXPECT_EQ(expected, result);

  // Union with a real set.
  URLPatternSet set2;
  AddPattern(&set2, "http://www.reddit.com/f*");
  AddPattern(&set2, "http://www.yahoo.com/z*");
  AddPattern(&set2, "http://www.google.com/f*");

  AddPattern(&expected, "http://www.reddit.com/f*");
  AddPattern(&expected, "http://www.yahoo.com/z*");

  result = URLPatternSet::CreateUnion(set1, set2);
  EXPECT_EQ(expected, result);
}

TEST(URLPatternSetTest, Contains) {
  URLPatternSet set1;
  URLPatternSet set2;
  URLPatternSet empty_set;

  AddPattern(&set1, "http://www.google.com/*");
  AddPattern(&set1, "http://www.yahoo.com/*");

  AddPattern(&set2, "http://www.reddit.com/*");

  EXPECT_FALSE(set1.Contains(set2));
  EXPECT_TRUE(set1.Contains(empty_set));
  EXPECT_FALSE(empty_set.Contains(set1));

  AddPattern(&set2, "http://www.yahoo.com/*");

  EXPECT_FALSE(set1.Contains(set2));
  EXPECT_FALSE(set2.Contains(set1));

  AddPattern(&set2, "http://www.google.com/*");

  EXPECT_FALSE(set1.Contains(set2));
  EXPECT_TRUE(set2.Contains(set1));

  // Note that this checks if individual patterns contain other patterns, not
  // just equality. For example:
  AddPattern(&set1, "http://*.reddit.com/*");
  EXPECT_TRUE(set1.Contains(set2));
  EXPECT_FALSE(set2.Contains(set1));
}

TEST(URLPatternSetTest, Duplicates) {
  URLPatternSet set1;
  URLPatternSet set2;

  AddPattern(&set1, "http://www.google.com/*");
  AddPattern(&set2, "http://www.google.com/*");

  AddPattern(&set1, "http://www.google.com/*");

  // The sets should still be equal after adding a duplicate.
  EXPECT_EQ(set2, set1);
}

TEST(URLPatternSetTest, ToValueAndPopulate) {
  URLPatternSet set1;
  URLPatternSet set2;

  std::vector<std::string> patterns;
  patterns.push_back("http://www.google.com/*");
  patterns.push_back("http://www.yahoo.com/*");

  for (size_t i = 0; i < patterns.size(); ++i)
    AddPattern(&set1, patterns[i]);

  std::string error;
  bool allow_file_access = false;
  std::unique_ptr<base::ListValue> value(set1.ToValue());
  set2.Populate(*value, URLPattern::SCHEME_ALL, allow_file_access, &error);
  EXPECT_EQ(set1, set2);

  set2.ClearPatterns();
  set2.Populate(patterns, URLPattern::SCHEME_ALL, allow_file_access, &error);
  EXPECT_EQ(set1, set2);
}

TEST(URLPatternSetTest, NwayUnion) {
  std::string google_a = "http://www.google.com/a*";
  std::string google_b = "http://www.google.com/b*";
  std::string google_c = "http://www.google.com/c*";
  std::string yahoo_a = "http://www.yahoo.com/a*";
  std::string yahoo_b = "http://www.yahoo.com/b*";
  std::string yahoo_c = "http://www.yahoo.com/c*";
  std::string reddit_a = "http://www.reddit.com/a*";
  std::string reddit_b = "http://www.reddit.com/b*";
  std::string reddit_c = "http://www.reddit.com/c*";

  // Empty list.
  {
    std::vector<URLPatternSet> empty;

    URLPatternSet result = URLPatternSet::CreateUnion(empty);

    URLPatternSet expected;
    EXPECT_EQ(expected, result);
  }

  // Singleton list.
  {
    std::vector<URLPatternSet> test;
    test.push_back(Patterns(google_a));

    URLPatternSet result = URLPatternSet::CreateUnion(test);

    URLPatternSet expected = Patterns(google_a);
    EXPECT_EQ(expected, result);
  }

  // List with 2 elements.
  {
    std::vector<URLPatternSet> test;
    test.push_back(Patterns(google_a, google_b));
    test.push_back(Patterns(google_b, google_c));

    URLPatternSet result = URLPatternSet::CreateUnion(test);

    URLPatternSet expected;
    AddPattern(&expected, google_a);
    AddPattern(&expected, google_b);
    AddPattern(&expected, google_c);
    EXPECT_EQ(expected, result);
  }

  // List with 3 elements.
  {
    std::vector<URLPatternSet> test;
    test.push_back(Patterns(google_a, google_b));
    test.push_back(Patterns(google_b, google_c));
    test.push_back(Patterns(yahoo_a, yahoo_b));

    URLPatternSet result = URLPatternSet::CreateUnion(test);

    URLPatternSet expected;
    AddPattern(&expected, google_a);
    AddPattern(&expected, google_b);
    AddPattern(&expected, google_c);
    AddPattern(&expected, yahoo_a);
    AddPattern(&expected, yahoo_b);
    EXPECT_EQ(expected, result);
  }

  // List with 7 elements.
  {
    std::vector<URLPatternSet> test;
    test.push_back(Patterns(google_a));
    test.push_back(Patterns(google_b));
    test.push_back(Patterns(google_c));
    test.push_back(Patterns(yahoo_a));
    test.push_back(Patterns(yahoo_b));
    test.push_back(Patterns(yahoo_c));
    test.push_back(Patterns(reddit_a));

    URLPatternSet result = URLPatternSet::CreateUnion(test);

    URLPatternSet expected;
    AddPattern(&expected, google_a);
    AddPattern(&expected, google_b);
    AddPattern(&expected, google_c);
    AddPattern(&expected, yahoo_a);
    AddPattern(&expected, yahoo_b);
    AddPattern(&expected, yahoo_c);
    AddPattern(&expected, reddit_a);
    EXPECT_EQ(expected, result);
  }

  // List with 8 elements.
  {
    std::vector<URLPatternSet> test;
    test.push_back(Patterns(google_a));
    test.push_back(Patterns(google_b));
    test.push_back(Patterns(google_c));
    test.push_back(Patterns(yahoo_a));
    test.push_back(Patterns(yahoo_b));
    test.push_back(Patterns(yahoo_c));
    test.push_back(Patterns(reddit_a));
    test.push_back(Patterns(reddit_b));

    URLPatternSet result = URLPatternSet::CreateUnion(test);

    URLPatternSet expected;
    AddPattern(&expected, google_a);
    AddPattern(&expected, google_b);
    AddPattern(&expected, google_c);
    AddPattern(&expected, yahoo_a);
    AddPattern(&expected, yahoo_b);
    AddPattern(&expected, yahoo_c);
    AddPattern(&expected, reddit_a);
    AddPattern(&expected, reddit_b);
    EXPECT_EQ(expected, result);
  }

  // List with 9 elements.
  {
    std::vector<URLPatternSet> test;
    test.push_back(Patterns(google_a));
    test.push_back(Patterns(google_b));
    test.push_back(Patterns(google_c));
    test.push_back(Patterns(yahoo_a));
    test.push_back(Patterns(yahoo_b));
    test.push_back(Patterns(yahoo_c));
    test.push_back(Patterns(reddit_a));
    test.push_back(Patterns(reddit_b));
    test.push_back(Patterns(reddit_c));

    URLPatternSet result = URLPatternSet::CreateUnion(test);

    URLPatternSet expected;
    AddPattern(&expected, google_a);
    AddPattern(&expected, google_b);
    AddPattern(&expected, google_c);
    AddPattern(&expected, yahoo_a);
    AddPattern(&expected, yahoo_b);
    AddPattern(&expected, yahoo_c);
    AddPattern(&expected, reddit_a);
    AddPattern(&expected, reddit_b);
    AddPattern(&expected, reddit_c);
    EXPECT_EQ(expected, result);
  }
}

TEST(URLPatternSetTest, AddOrigin) {
  URLPatternSet set;
  EXPECT_TRUE(set.AddOrigin(
      URLPattern::SCHEME_ALL, GURL("https://www.google.com/")));
  EXPECT_TRUE(set.MatchesURL(GURL("https://www.google.com/foo/bar")));
  EXPECT_FALSE(set.MatchesURL(GURL("http://www.google.com/foo/bar")));
  EXPECT_FALSE(set.MatchesURL(GURL("https://en.google.com/foo/bar")));
  set.ClearPatterns();

  EXPECT_TRUE(set.AddOrigin(
      URLPattern::SCHEME_ALL, GURL("https://google.com/")));
  EXPECT_FALSE(set.MatchesURL(GURL("https://www.google.com/foo/bar")));
  EXPECT_TRUE(set.MatchesURL(GURL("https://google.com/foo/bar")));

  EXPECT_FALSE(set.AddOrigin(
      URLPattern::SCHEME_HTTP, GURL("https://google.com/")));
}

TEST(URLPatternSetTest, ToStringVector) {
  URLPatternSet set;
  AddPattern(&set, "https://google.com/");
  AddPattern(&set, "https://google.com/");
  AddPattern(&set, "https://yahoo.com/");

  std::unique_ptr<std::vector<std::string>> string_vector(set.ToStringVector());

  EXPECT_EQ(2UL, string_vector->size());

  EXPECT_TRUE(base::ContainsValue(*string_vector, "https://google.com/"));
  EXPECT_TRUE(base::ContainsValue(*string_vector, "https://yahoo.com/"));
}

}  // namespace extensions
