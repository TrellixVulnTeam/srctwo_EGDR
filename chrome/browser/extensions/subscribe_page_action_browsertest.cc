// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/extensions/extension_browsertest.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "chrome/common/url_constants.h"
#include "chrome/test/base/ui_test_utils.h"
#include "content/public/browser/web_contents.h"
#include "content/public/test/browser_test_utils.h"
#include "extensions/common/constants.h"
#include "extensions/common/extension.h"
#include "net/test/embedded_test_server/embedded_test_server.h"

using content::WebContents;
using extensions::Extension;

namespace {

const char kSubscribePage[] = "/subscribe.html";
const char kFeedPageMultiRel[] = "/feeds/feed_multi_rel.html";
const char kValidFeedNoLinks[] = "/feeds/feed_nolinks.xml";
const char kValidFeed0[] = "/feeds/feed_script.xml";
const char kValidFeed1[] = "/feeds/feed1.xml";
const char kValidFeed2[] = "/feeds/feed2.xml";
const char kValidFeed3[] = "/feeds/feed3.xml";
const char kValidFeed4[] = "/feeds/feed4.xml";
const char kValidFeed5[] = "/feeds/feed5.xml";
const char kValidFeed6[] = "/feeds/feed6.xml";
const char kInvalidFeed1[] = "/feeds/feed_invalid1.xml";
const char kInvalidFeed2[] = "/feeds/feed_invalid2.xml";
// We need a triple encoded string to prove that we are not decoding twice in
// subscribe.js because one layer is also stripped off when subscribe.js passes
// it to the XMLHttpRequest object.
const char kFeedTripleEncoded[] = "/feeds/url%25255Fdecoding.html";

static const char kScriptFeedTitle[] =
    "window.domAutomationController.send("
    "  document.getElementById('title') ? "
    "    document.getElementById('title').textContent : "
    "    \"element 'title' not found\""
    ");";
static const char kScriptAnchor[] =
    "window.domAutomationController.send("
    "  document.getElementById('anchor_0') ? "
    "    document.getElementById('anchor_0').textContent : "
    "    \"element 'anchor_0' not found\""
    ");";
static const char kScriptDesc[] =
    "window.domAutomationController.send("
    "  document.getElementById('desc_0') ? "
    "    document.getElementById('desc_0').textContent : "
    "    \"element 'desc_0' not found\""
    ");";
static const char kScriptError[] =
    "window.domAutomationController.send("
    "  document.getElementById('error') ? "
    "    document.getElementById('error').textContent : "
    "    \"No error\""
    ");";

GURL GetFeedUrl(net::EmbeddedTestServer* server,
                const std::string& feed_page,
                bool direct_url,
                std::string extension_id) {
  GURL feed_url = server->GetURL(feed_page);
  if (direct_url) {
    // We navigate directly to the subscribe page for feeds where the feed
    // sniffing won't work, in other words, as is the case for malformed feeds.
    return GURL(std::string(extensions::kExtensionScheme) +
        url::kStandardSchemeSeparator +
        extension_id + std::string(kSubscribePage) + std::string("?") +
        feed_url.spec() + std::string("&synchronous"));
  } else {
    // Navigate to the feed content (which will cause the extension to try to
    // sniff the type and display the subscribe page in another tab.
    return GURL(feed_url.spec());
  }
}

bool ValidatePageElement(content::RenderFrameHost* frame,
                         const std::string& javascript,
                         const std::string& expected_value) {
  std::string returned_value;

  if (!content::ExecuteScriptAndExtractString(frame,
                                              javascript,
                                              &returned_value))
    return false;

  EXPECT_STREQ(expected_value.c_str(), returned_value.c_str());
  return expected_value == returned_value;
}

// Navigates to a feed page and, if |sniff_xml_type| is set, wait for the
// extension to kick in, detect the feed and redirect to a feed preview page.
// |sniff_xml_type| is generally set to true if the feed is sniffable and false
// for invalid feeds.
void NavigateToFeedAndValidate(net::EmbeddedTestServer* server,
                               const std::string& url,
                               Browser* browser,
                               std::string extension_id,
                               bool sniff_xml_type,
                               const std::string& expected_feed_title,
                               const std::string& expected_item_title,
                               const std::string& expected_item_desc,
                               const std::string& expected_error) {
  if (sniff_xml_type) {
    // TODO(finnur): Implement this is a non-flaky way.
  }

  // Navigate to the subscribe page directly.
  ui_test_utils::NavigateToURL(browser,
                               GetFeedUrl(server, url, true, extension_id));

  WebContents* tab = browser->tab_strip_model()->GetActiveWebContents();
  content::RenderFrameHost* frame = content::FrameMatchingPredicate(
      tab, base::Bind(&content::FrameIsChildOfMainFrame));
  ASSERT_TRUE(ValidatePageElement(
      tab->GetMainFrame(), kScriptFeedTitle, expected_feed_title));
  ASSERT_TRUE(ValidatePageElement(frame, kScriptAnchor, expected_item_title));
  ASSERT_TRUE(ValidatePageElement(frame, kScriptDesc, expected_item_desc));
  ASSERT_TRUE(ValidatePageElement(frame, kScriptError, expected_error));
}

} // namespace

// Makes sure that the RSS detects RSS feed links, even when rel tag contains
// more than just "alternate".
IN_PROC_BROWSER_TEST_F(ExtensionBrowserTest, RSSMultiRelLink) {
  ASSERT_TRUE(embedded_test_server()->Start());

  ASSERT_TRUE(LoadExtension(
    test_data_dir_.AppendASCII("subscribe_page_action")));

  ASSERT_TRUE(WaitForPageActionVisibilityChangeTo(0));

  // Navigate to the feed page.
  GURL feed_url = embedded_test_server()->GetURL(kFeedPageMultiRel);
  ui_test_utils::NavigateToURL(browser(), feed_url);
  // We should now have one page action ready to go in the LocationBar.
  ASSERT_TRUE(WaitForPageActionVisibilityChangeTo(1));
}

// This test is flaky on all platforms; see http://crbug.com/340354
IN_PROC_BROWSER_TEST_F(ExtensionBrowserTest, DISABLED_RSSParseFeedValidFeed1) {
  ASSERT_TRUE(embedded_test_server()->Start());

  const Extension* extension = LoadExtension(
      test_data_dir_.AppendASCII("subscribe_page_action"));
  ASSERT_TRUE(extension);
  std::string id = extension->id();

  NavigateToFeedAndValidate(embedded_test_server(), kValidFeed1, browser(), id,
                            true, "Feed for MyFeedTitle", "Title 1", "Desc",
                            "No error");
}

// This test is flaky on all platforms; see http://crbug.com/340354
IN_PROC_BROWSER_TEST_F(ExtensionBrowserTest, DISABLED_RSSParseFeedValidFeed2) {
  ASSERT_TRUE(embedded_test_server()->Start());

  const Extension* extension = LoadExtension(
      test_data_dir_.AppendASCII("subscribe_page_action"));
  ASSERT_TRUE(extension);
  std::string id = extension->id();

  NavigateToFeedAndValidate(embedded_test_server(), kValidFeed2, browser(), id,
                            true, "Feed for MyFeed2", "My item title1",
                            "This is a summary.", "No error");
}

// This test is flaky on all platforms; see http://crbug.com/340354
IN_PROC_BROWSER_TEST_F(ExtensionBrowserTest, DISABLED_RSSParseFeedValidFeed3) {
  ASSERT_TRUE(embedded_test_server()->Start());

  const Extension* extension = LoadExtension(
      test_data_dir_.AppendASCII("subscribe_page_action"));
  ASSERT_TRUE(extension);
  std::string id = extension->id();

  NavigateToFeedAndValidate(embedded_test_server(), kValidFeed3, browser(), id,
                            true, "Feed for Google Code buglist rss feed",
                            "My dear title", "My dear content", "No error");
}

// This test is flaky on all platforms; see http://crbug.com/340354
IN_PROC_BROWSER_TEST_F(ExtensionBrowserTest, DISABLED_RSSParseFeedValidFeed4) {
  ASSERT_TRUE(embedded_test_server()->Start());

  const Extension* extension = LoadExtension(
      test_data_dir_.AppendASCII("subscribe_page_action"));
  ASSERT_TRUE(extension);
  std::string id = extension->id();

  NavigateToFeedAndValidate(embedded_test_server(), kValidFeed4, browser(), id,
                            true, "Feed for Title chars <script> %23 stop",
                            "Title chars  %23 stop", "My dear content %23 stop",
                            "No error");
}

// This test is flaky on all platforms; see http://crbug.com/340354
IN_PROC_BROWSER_TEST_F(ExtensionBrowserTest, DISABLED_RSSParseFeedValidFeed0) {
  ASSERT_TRUE(embedded_test_server()->Start());

  const Extension* extension = LoadExtension(
      test_data_dir_.AppendASCII("subscribe_page_action"));
  ASSERT_TRUE(extension);
  std::string id = extension->id();

  // Try a feed with a link with an onclick handler (before r27440 this would
  // trigger a NOTREACHED).
  NavigateToFeedAndValidate(embedded_test_server(), kValidFeed0, browser(), id,
                            true, "Feed for MyFeedTitle", "Title 1",
                            "Desc VIDEO", "No error");
}

// This test is flaky on all platforms; see http://crbug.com/340354
IN_PROC_BROWSER_TEST_F(ExtensionBrowserTest, DISABLED_RSSParseFeedValidFeed5) {
  ASSERT_TRUE(embedded_test_server()->Start());

  const Extension* extension = LoadExtension(
      test_data_dir_.AppendASCII("subscribe_page_action"));
  ASSERT_TRUE(extension);
  std::string id = extension->id();

  // Feed with valid but mostly empty xml.
  NavigateToFeedAndValidate(
      embedded_test_server(), kValidFeed5, browser(), id, true,
      "Feed for Unknown feed name", "element 'anchor_0' not found",
      "element 'desc_0' not found", "This feed contains no entries.");
}

// This test is flaky on all platforms; see http://crbug.com/340354
IN_PROC_BROWSER_TEST_F(ExtensionBrowserTest, DISABLED_RSSParseFeedValidFeed6) {
  ASSERT_TRUE(embedded_test_server()->Start());

  const Extension* extension = LoadExtension(
      test_data_dir_.AppendASCII("subscribe_page_action"));
  ASSERT_TRUE(extension);
  std::string id = extension->id();

  // Feed that is technically invalid but still parseable.
  NavigateToFeedAndValidate(embedded_test_server(), kValidFeed6, browser(), id,
                            true, "Feed for MyFeedTitle", "Title 1", "Desc",
                            "No error");
}

IN_PROC_BROWSER_TEST_F(ExtensionBrowserTest, RSSParseFeedInvalidFeed1) {
  ASSERT_TRUE(embedded_test_server()->Start());

  const Extension* extension = LoadExtension(
      test_data_dir_.AppendASCII("subscribe_page_action"));
  ASSERT_TRUE(extension);
  std::string id = extension->id();

  // Try an empty feed.
  NavigateToFeedAndValidate(
      embedded_test_server(), kInvalidFeed1, browser(), id, false,
      "Feed for Unknown feed name", "element 'anchor_0' not found",
      "element 'desc_0' not found", "This feed contains no entries.");
}

IN_PROC_BROWSER_TEST_F(ExtensionBrowserTest, RSSParseFeedInvalidFeed2) {
  ASSERT_TRUE(embedded_test_server()->Start());

  const Extension* extension = LoadExtension(
      test_data_dir_.AppendASCII("subscribe_page_action"));
  ASSERT_TRUE(extension);
  std::string id = extension->id();

  // Try a garbage feed.
  NavigateToFeedAndValidate(
      embedded_test_server(), kInvalidFeed2, browser(), id, false,
      "Feed for Unknown feed name", "element 'anchor_0' not found",
      "element 'desc_0' not found", "This feed contains no entries.");
}

IN_PROC_BROWSER_TEST_F(ExtensionBrowserTest, RSSParseFeedInvalidFeed3) {
  ASSERT_TRUE(embedded_test_server()->Start());

  const Extension* extension = LoadExtension(
      test_data_dir_.AppendASCII("subscribe_page_action"));
  ASSERT_TRUE(extension);
  std::string id = extension->id();

  // Try a feed that doesn't exist.
  NavigateToFeedAndValidate(
      embedded_test_server(), "/foo.xml", browser(), id, false,
      "Feed for Unknown feed name", "element 'anchor_0' not found",
      "element 'desc_0' not found", "This feed contains no entries.");
}

IN_PROC_BROWSER_TEST_F(ExtensionBrowserTest, RSSParseFeedInvalidFeed4) {
  ASSERT_TRUE(embedded_test_server()->Start());

  const Extension* extension = LoadExtension(
      test_data_dir_.AppendASCII("subscribe_page_action"));
  ASSERT_TRUE(extension);
  std::string id = extension->id();

  // subscribe.js shouldn't double-decode the URL passed in. Otherwise feed
  // links such as http://search.twitter.com/search.atom?lang=en&q=%23chrome
  // will result in no feed being downloaded because %23 gets decoded to # and
  // therefore #chrome is not treated as part of the Twitter query. This test
  // uses an underscore instead of a hash, but the principle is the same. If
  // we start erroneously double decoding again, the path (and the feed) will
  // become valid resulting in a failure for this test.
  NavigateToFeedAndValidate(
      embedded_test_server(), kFeedTripleEncoded, browser(), id, true,
      "Feed for Unknown feed name", "element 'anchor_0' not found",
      "element 'desc_0' not found", "This feed contains no entries.");
}

// This test is flaky on all platforms; see http://crbug.com/340354
IN_PROC_BROWSER_TEST_F(ExtensionBrowserTest,
                       DISABLED_RSSParseFeedValidFeedNoLinks) {
  ASSERT_TRUE(embedded_test_server()->Start());

  const Extension* extension = LoadExtension(
      test_data_dir_.AppendASCII("subscribe_page_action"));
  ASSERT_TRUE(extension);
  std::string id = extension->id();

  // Valid feed but containing no links.
  NavigateToFeedAndValidate(embedded_test_server(), kValidFeedNoLinks,
                            browser(), id, true, "Feed for MyFeedTitle",
                            "Title with no link", "Desc", "No error");
}
