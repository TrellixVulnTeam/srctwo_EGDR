// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/offline_pages/core/renovations/page_renovator.h"

#include <memory>
#include <utility>
#include <vector>

#include "base/files/file_path.h"
#include "base/path_service.h"
#include "base/run_loop.h"
#include "base/strings/utf_string_conversions.h"
#include "base/values.h"
#include "build/build_config.h"
#include "components/offline_pages/content/renovations/render_frame_script_injector.h"
#include "components/offline_pages/core/renovations/page_renovation_loader.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/isolated_world_ids.h"
#include "content/public/test/content_browser_test.h"
#include "content/public/test/content_browser_test_utils.h"
#include "content/public/test/test_utils.h"
#include "content/shell/browser/shell.h"
#include "net/test/embedded_test_server/embedded_test_server.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/base/resource/resource_bundle.h"
#include "url/gurl.h"

namespace offline_pages {

namespace {

const char kDocRoot[] = "components/test/data/offline_pages";

// For testing real renovations.
const char kWikipediaTestPagePath[] = "/wikipedia_renovation_test_page.html";

const char kCheckUnfoldBlockScript[] =
    "document.getElementById('block1').classList.contains('open-block') && "
    "document.getElementById('block2').classList.contains('open-block')";
const char kCheckUnfoldHeadingScript[] =
    "document.getElementById('heading1').classList.contains('open-block') && "
    "document.getElementById('heading2').classList.contains('open-block')";

// For running against the test renovations.
const char kTestPagePath[] = "/renovator_test_page.html";
const char kTestRenovationScript[] =
    R"*(function foo() {
      var node = document.getElementById('foo');
      node.innerHTML = 'Good';
    }
    function bar() {
      var node = document.getElementById('bar');
      node.parentNode.removeChild(node);
    }
    function always() {
      var node = document.getElementById('always');
      node.parentNode.removeChild(node);
    }
    var map_renovations = {'foo':foo, 'bar':bar, 'always':always};
    function run_renovations(idlist) {
      for (var id of idlist) {
        map_renovations[id]();
      }
    })*";

// Scripts to check whether each renovation ran in the test page.
const char kCheckFooScript[] =
    "document.getElementById('foo').innerHTML == 'Good'";
const char kCheckBarScript[] = "document.getElementById('bar') == null";
const char kCheckAlwaysScript[] = "document.getElementById('always') == null";

// Renovation that should only run on pages from URL foo.bar
class FooPageRenovation : public PageRenovation {
 public:
  bool ShouldRun(const GURL& url) const override {
    return url.host() == "foo.bar";
  }
  std::string GetID() const override { return "foo"; }
};

// Renovation that should only run on pages from URL bar.qux
class BarPageRenovation : public PageRenovation {
 public:
  bool ShouldRun(const GURL& url) const override {
    return url.host() == "bar.qux";
  }
  std::string GetID() const override { return "bar"; }
};

// Renovation that should run on every page.
class AlwaysRenovation : public PageRenovation {
 public:
  bool ShouldRun(const GURL& url) const override { return true; }
  std::string GetID() const override { return "always"; }
};

}  // namespace

class PageRenovatorBrowserTest : public content::ContentBrowserTest {
 public:
  void SetUpOnMainThread() override;

  // Navigates the content shell to the path indicated. This path is
  // relative to the Chromium source root.
  void Navigate(const std::string& test_page_path);

  // These functions initialize the PageRenovator and dependencies
  // with either testing renovations defined above, or with production
  // renovations. |fake_url| is the URL passed to the PageRenovator
  // and determines which renovations should be run.
  //
  // Only one of these should be called, and |Navigate| should be called
  // beforehand.
  void InitializeWithTestingRenovations(const GURL& fake_url);
  void InitializeWithRealRenovations(const GURL& fake_url);

  void QuitRunLoop();

 protected:
  net::EmbeddedTestServer test_server_;
  content::RenderFrameHost* render_frame_;
  std::unique_ptr<PageRenovationLoader> page_renovation_loader_;
  std::unique_ptr<PageRenovator> page_renovator_;

  std::unique_ptr<base::RunLoop> run_loop_;
};

void PageRenovatorBrowserTest::SetUpOnMainThread() {
  // Add resources pack to resource bundle so PageRenovationLoader can
  // load our renovation script.
  base::FilePath pak_dir;
#if defined(OS_ANDROID)
  PathService::Get(base::DIR_ANDROID_APP_DATA, &pak_dir);
  pak_dir = pak_dir.Append(FILE_PATH_LITERAL("paks"));
#else
  PathService::Get(base::DIR_MODULE, &pak_dir);
#endif  // OS_ANDROID
  base::FilePath pak_file =
      pak_dir.Append(FILE_PATH_LITERAL("components_tests_resources.pak"));
  ui::ResourceBundle::GetSharedInstance().AddDataPackFromPath(
      pak_file, ui::SCALE_FACTOR_NONE);

  // Serve our test HTML.
  test_server_.ServeFilesFromSourceDirectory(kDocRoot);
  ASSERT_TRUE(test_server_.Start());

  run_loop_.reset(new base::RunLoop);
}

void PageRenovatorBrowserTest::Navigate(const std::string& test_page_path) {
  GURL url = test_server_.GetURL(test_page_path);
  content::NavigateToURL(shell(), url);
  render_frame_ = shell()->web_contents()->GetMainFrame();
}

void PageRenovatorBrowserTest::InitializeWithTestingRenovations(
    const GURL& fake_url) {
  ASSERT_TRUE(render_frame_) << "Navigate should have been called.";

  std::vector<std::unique_ptr<PageRenovation>> renovations;
  renovations.push_back(base::MakeUnique<FooPageRenovation>());
  renovations.push_back(base::MakeUnique<BarPageRenovation>());
  renovations.push_back(base::MakeUnique<AlwaysRenovation>());

  page_renovation_loader_.reset(new PageRenovationLoader);
  page_renovation_loader_->SetSourceForTest(
      base::ASCIIToUTF16(kTestRenovationScript));
  page_renovation_loader_->SetRenovationsForTest(std::move(renovations));

  auto script_injector = base::MakeUnique<RenderFrameScriptInjector>(
      render_frame_, content::ISOLATED_WORLD_ID_CONTENT_END);
  page_renovator_.reset(new PageRenovator(
      page_renovation_loader_.get(), std::move(script_injector), fake_url));
}

void PageRenovatorBrowserTest::InitializeWithRealRenovations(
    const GURL& fake_url) {
  ASSERT_TRUE(render_frame_) << "Navigate should have been called.";

  page_renovation_loader_.reset(new PageRenovationLoader);

  auto script_injector = base::MakeUnique<RenderFrameScriptInjector>(
      render_frame_, content::ISOLATED_WORLD_ID_CONTENT_END);
  page_renovator_.reset(new PageRenovator(
      page_renovation_loader_.get(), std::move(script_injector), fake_url));
}

void PageRenovatorBrowserTest::QuitRunLoop() {
  base::Closure quit_task =
      content::GetDeferredQuitTaskForRunLoop(run_loop_.get());
  content::BrowserThread::PostTask(content::BrowserThread::UI, FROM_HERE,
                                   quit_task);
}

IN_PROC_BROWSER_TEST_F(PageRenovatorBrowserTest, CorrectRenovationsRun) {
  Navigate(kTestPagePath);
  InitializeWithTestingRenovations(GURL("http://foo.bar/"));
  // This should run FooPageRenovation and AlwaysRenovation, but not
  // BarPageRenovation.
  page_renovator_->RunRenovations(base::Bind(
      &PageRenovatorBrowserTest::QuitRunLoop, base::Unretained(this)));
  content::RunThisRunLoop(run_loop_.get());

  // Check that correct modifications were made to the page.
  std::unique_ptr<base::Value> fooResult =
      content::ExecuteScriptAndGetValue(render_frame_, kCheckFooScript);
  std::unique_ptr<base::Value> barResult =
      content::ExecuteScriptAndGetValue(render_frame_, kCheckBarScript);
  std::unique_ptr<base::Value> alwaysResult =
      content::ExecuteScriptAndGetValue(render_frame_, kCheckAlwaysScript);

  ASSERT_TRUE(fooResult.get() != nullptr);
  ASSERT_TRUE(barResult.get() != nullptr);
  ASSERT_TRUE(alwaysResult.get() != nullptr);
  EXPECT_TRUE(fooResult->GetBool());
  EXPECT_FALSE(barResult->GetBool());
  EXPECT_TRUE(alwaysResult->GetBool());
}

IN_PROC_BROWSER_TEST_F(PageRenovatorBrowserTest, WikipediaRenovationRuns) {
  Navigate(kWikipediaTestPagePath);
  InitializeWithRealRenovations(GURL("http://en.m.wikipedia.org/"));
  page_renovator_->RunRenovations(base::Bind(
      &PageRenovatorBrowserTest::QuitRunLoop, base::Unretained(this)));
  content::RunThisRunLoop(run_loop_.get());

  std::unique_ptr<base::Value> unfoldBlockResult =
      content::ExecuteScriptAndGetValue(render_frame_, kCheckUnfoldBlockScript);
  std::unique_ptr<base::Value> unfoldHeadingResult =
      content::ExecuteScriptAndGetValue(render_frame_,
                                        kCheckUnfoldHeadingScript);

  ASSERT_TRUE(unfoldBlockResult.get() != nullptr);
  ASSERT_TRUE(unfoldHeadingResult.get() != nullptr);
  EXPECT_TRUE(unfoldBlockResult->GetBool());
  EXPECT_TRUE(unfoldHeadingResult->GetBool());
}

}  // namespace offline_pages
