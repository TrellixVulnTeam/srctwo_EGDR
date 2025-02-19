// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/dom_distiller/core/task_tracker.h"

#include <utility>

#include "base/message_loop/message_loop.h"
#include "base/run_loop.h"
#include "components/dom_distiller/core/article_distillation_update.h"
#include "components/dom_distiller/core/article_entry.h"
#include "components/dom_distiller/core/distilled_content_store.h"
#include "components/dom_distiller/core/fake_distiller.h"
#include "testing/gtest/include/gtest/gtest.h"

using testing::Return;
using testing::_;

namespace dom_distiller {
namespace test {

class FakeViewRequestDelegate : public ViewRequestDelegate {
 public:
  virtual ~FakeViewRequestDelegate() {}
  MOCK_METHOD1(OnArticleReady,
               void(const DistilledArticleProto* article_proto));
  MOCK_METHOD1(OnArticleUpdated,
               void(ArticleDistillationUpdate article_update));
};

class MockContentStore : public DistilledContentStore {
 public:
  MOCK_METHOD2(LoadContent,
               void(const ArticleEntry& entry, LoadCallback callback));
  MOCK_METHOD3(SaveContent,
               void(const ArticleEntry& entry,
                    const DistilledArticleProto& proto,
                    SaveCallback callback));
};

class TestCancelCallback {
 public:
  TestCancelCallback() : cancelled_(false) {}
  TaskTracker::CancelCallback GetCallback() {
    return base::Bind(&TestCancelCallback::Cancel, base::Unretained(this));
  }
  void Cancel(TaskTracker*) { cancelled_ = true; }
  bool Cancelled() { return cancelled_; }

 private:
  bool cancelled_;
};

class MockSaveCallback {
 public:
  MOCK_METHOD3(Save,
               void(const ArticleEntry&, const DistilledArticleProto*, bool));
};

class DomDistillerTaskTrackerTest : public testing::Test {
 public:
  void SetUp() override {
    message_loop_.reset(new base::MessageLoop());
    entry_id_ = "id0";
    page_0_url_ = GURL("http://www.example.com/1");
    page_1_url_ = GURL("http://www.example.com/2");
  }

  ArticleEntry GetDefaultEntry() {
    ArticleEntry entry;
    entry.set_entry_id(entry_id_);
    ArticleEntryPage* page0 = entry.add_pages();
    ArticleEntryPage* page1 = entry.add_pages();
    page0->set_url(page_0_url_.spec());
    page1->set_url(page_1_url_.spec());
    return entry;
  }

 protected:
  std::unique_ptr<base::MessageLoop> message_loop_;
  std::string entry_id_;
  GURL page_0_url_;
  GURL page_1_url_;
};

TEST_F(DomDistillerTaskTrackerTest, TestHasEntryId) {
  MockDistillerFactory distiller_factory;
  TestCancelCallback cancel_callback;
  TaskTracker task_tracker(
      GetDefaultEntry(), cancel_callback.GetCallback(), NULL);
  EXPECT_TRUE(task_tracker.HasEntryId(entry_id_));
  EXPECT_FALSE(task_tracker.HasEntryId("other_id"));
}

TEST_F(DomDistillerTaskTrackerTest, TestHasUrl) {
  MockDistillerFactory distiller_factory;
  TestCancelCallback cancel_callback;
  TaskTracker task_tracker(
      GetDefaultEntry(), cancel_callback.GetCallback(), NULL);
  EXPECT_TRUE(task_tracker.HasUrl(page_0_url_));
  EXPECT_TRUE(task_tracker.HasUrl(page_1_url_));
  EXPECT_FALSE(task_tracker.HasUrl(GURL("http://other.url/")));
}

TEST_F(DomDistillerTaskTrackerTest, TestViewerCancelled) {
  MockDistillerFactory distiller_factory;
  TestCancelCallback cancel_callback;
  TaskTracker task_tracker(
      GetDefaultEntry(), cancel_callback.GetCallback(), NULL);

  FakeViewRequestDelegate viewer_delegate;
  FakeViewRequestDelegate viewer_delegate2;
  std::unique_ptr<ViewerHandle> handle(
      task_tracker.AddViewer(&viewer_delegate));
  std::unique_ptr<ViewerHandle> handle2(
      task_tracker.AddViewer(&viewer_delegate2));

  EXPECT_FALSE(cancel_callback.Cancelled());
  handle.reset();
  EXPECT_FALSE(cancel_callback.Cancelled());
  handle2.reset();
  EXPECT_TRUE(cancel_callback.Cancelled());
}

TEST_F(DomDistillerTaskTrackerTest, TestViewerCancelledWithSaveRequest) {
  MockDistillerFactory distiller_factory;
  TestCancelCallback cancel_callback;
  TaskTracker task_tracker(
      GetDefaultEntry(), cancel_callback.GetCallback(), NULL);

  FakeViewRequestDelegate viewer_delegate;
  std::unique_ptr<ViewerHandle> handle(
      task_tracker.AddViewer(&viewer_delegate));
  EXPECT_FALSE(cancel_callback.Cancelled());

  MockSaveCallback save_callback;
  task_tracker.AddSaveCallback(
      base::Bind(&MockSaveCallback::Save, base::Unretained(&save_callback)));
  handle.reset();

  // Since there is a pending save request, the task shouldn't be cancelled.
  EXPECT_FALSE(cancel_callback.Cancelled());
}

TEST_F(DomDistillerTaskTrackerTest, TestViewerNotifiedOnDistillationComplete) {
  MockDistillerFactory distiller_factory;
  FakeDistiller* distiller = new FakeDistiller(true);
  EXPECT_CALL(distiller_factory, CreateDistillerImpl())
      .WillOnce(Return(distiller));
  TestCancelCallback cancel_callback;
  TaskTracker task_tracker(
      GetDefaultEntry(), cancel_callback.GetCallback(), NULL);

  FakeViewRequestDelegate viewer_delegate;
  std::unique_ptr<ViewerHandle> handle(
      task_tracker.AddViewer(&viewer_delegate));
  base::RunLoop().RunUntilIdle();

  EXPECT_CALL(viewer_delegate, OnArticleReady(_));

  task_tracker.StartDistiller(&distiller_factory,
                              std::unique_ptr<DistillerPage>());
  base::RunLoop().RunUntilIdle();

  EXPECT_FALSE(cancel_callback.Cancelled());
}

TEST_F(DomDistillerTaskTrackerTest, TestDistillerFails) {
  MockDistillerFactory distiller_factory;
  FakeDistiller* distiller = new FakeDistiller(false);
  EXPECT_CALL(distiller_factory, CreateDistillerImpl())
      .WillOnce(Return(distiller));

  TestCancelCallback cancel_callback;
  TaskTracker task_tracker(
      GetDefaultEntry(), cancel_callback.GetCallback(), NULL);

  FakeViewRequestDelegate viewer_delegate;
  std::unique_ptr<ViewerHandle> handle(
      task_tracker.AddViewer(&viewer_delegate));
  base::RunLoop().RunUntilIdle();

  EXPECT_CALL(viewer_delegate, OnArticleReady(_));

  task_tracker.StartDistiller(&distiller_factory,
                              std::unique_ptr<DistillerPage>());
  distiller->RunDistillerCallback(
      std::unique_ptr<DistilledArticleProto>(new DistilledArticleProto));
  base::RunLoop().RunUntilIdle();

  EXPECT_FALSE(cancel_callback.Cancelled());
}

TEST_F(DomDistillerTaskTrackerTest,
       TestSaveCallbackCalledOnDistillationComplete) {
  MockDistillerFactory distiller_factory;
  FakeDistiller* distiller = new FakeDistiller(true);
  EXPECT_CALL(distiller_factory, CreateDistillerImpl())
      .WillOnce(Return(distiller));
  TestCancelCallback cancel_callback;
  TaskTracker task_tracker(
      GetDefaultEntry(), cancel_callback.GetCallback(), NULL);

  MockSaveCallback save_callback;
  task_tracker.AddSaveCallback(
      base::Bind(&MockSaveCallback::Save, base::Unretained(&save_callback)));
  base::RunLoop().RunUntilIdle();

  EXPECT_CALL(save_callback, Save(_, _, _));

  task_tracker.StartDistiller(&distiller_factory,
                              std::unique_ptr<DistillerPage>());
  base::RunLoop().RunUntilIdle();

  EXPECT_TRUE(cancel_callback.Cancelled());
}

DistilledArticleProto CreateDistilledArticleForEntry(
    const ArticleEntry& entry) {
  DistilledArticleProto article;
  for (int i = 0; i < entry.pages_size(); ++i) {
    DistilledPageProto* page = article.add_pages();
    page->set_url(entry.pages(i).url());
    page->set_html("<div>" + entry.pages(i).url() + "</div>");
  }
  return article;
}

TEST_F(DomDistillerTaskTrackerTest, TestBlobFetcher) {
  ArticleEntry entry_with_blob = GetDefaultEntry();
  DistilledArticleProto stored_distilled_article =
      CreateDistilledArticleForEntry(entry_with_blob);
  InMemoryContentStore content_store(kDefaultMaxNumCachedEntries);
  content_store.InjectContent(entry_with_blob, stored_distilled_article);
  TestCancelCallback cancel_callback;

  TaskTracker task_tracker(
      entry_with_blob, cancel_callback.GetCallback(), &content_store);

  FakeViewRequestDelegate viewer_delegate;
  std::unique_ptr<ViewerHandle> handle(
      task_tracker.AddViewer(&viewer_delegate));
  base::RunLoop().RunUntilIdle();

  const DistilledArticleProto* distilled_article;

  EXPECT_CALL(viewer_delegate, OnArticleReady(_))
      .WillOnce(testing::SaveArg<0>(&distilled_article));

  task_tracker.StartBlobFetcher();
  base::RunLoop().RunUntilIdle();

  EXPECT_EQ(stored_distilled_article.SerializeAsString(),
            distilled_article->SerializeAsString());

  EXPECT_FALSE(cancel_callback.Cancelled());
}

TEST_F(DomDistillerTaskTrackerTest, TestBlobFetcherFinishesFirst) {
  MockDistillerFactory distiller_factory;
  FakeDistiller* distiller = new FakeDistiller(false);
  EXPECT_CALL(distiller_factory, CreateDistillerImpl())
      .WillOnce(Return(distiller));

  ArticleEntry entry_with_blob = GetDefaultEntry();
  DistilledArticleProto stored_distilled_article =
      CreateDistilledArticleForEntry(entry_with_blob);
  InMemoryContentStore content_store(kDefaultMaxNumCachedEntries);
  content_store.InjectContent(entry_with_blob, stored_distilled_article);
  TestCancelCallback cancel_callback;
  TaskTracker task_tracker(
      entry_with_blob, cancel_callback.GetCallback(), &content_store);

  FakeViewRequestDelegate viewer_delegate;
  std::unique_ptr<ViewerHandle> handle(
      task_tracker.AddViewer(&viewer_delegate));
  base::RunLoop().RunUntilIdle();

  DistilledArticleProto distilled_article;

  EXPECT_CALL(viewer_delegate, OnArticleReady(_))
      .WillOnce(testing::SaveArgPointee<0>(&distilled_article));
  bool distiller_destroyed = false;
  EXPECT_CALL(*distiller, Die())
      .WillOnce(testing::Assign(&distiller_destroyed, true));

  task_tracker.StartDistiller(&distiller_factory,
                              std::unique_ptr<DistillerPage>());
  task_tracker.StartBlobFetcher();
  base::RunLoop().RunUntilIdle();

  testing::Mock::VerifyAndClearExpectations(&viewer_delegate);
  EXPECT_EQ(stored_distilled_article.SerializeAsString(),
            distilled_article.SerializeAsString());

  EXPECT_TRUE(distiller_destroyed);
  EXPECT_FALSE(cancel_callback.Cancelled());
  base::RunLoop().RunUntilIdle();
}

TEST_F(DomDistillerTaskTrackerTest, TestBlobFetcherWithoutBlob) {
  MockDistillerFactory distiller_factory;
  FakeDistiller* distiller = new FakeDistiller(false);
  EXPECT_CALL(distiller_factory, CreateDistillerImpl())
      .WillOnce(Return(distiller));

  ArticleEntry entry(GetDefaultEntry());
  InMemoryContentStore content_store(kDefaultMaxNumCachedEntries);
  std::unique_ptr<DistilledArticleProto> distilled_article(
      new DistilledArticleProto(CreateDistilledArticleForEntry(entry)));

  TestCancelCallback cancel_callback;
  TaskTracker task_tracker(
      GetDefaultEntry(), cancel_callback.GetCallback(), &content_store);

  FakeViewRequestDelegate viewer_delegate;
  std::unique_ptr<ViewerHandle> handle(
      task_tracker.AddViewer(&viewer_delegate));
  base::RunLoop().RunUntilIdle();

  task_tracker.StartBlobFetcher();
  task_tracker.StartDistiller(&distiller_factory,
                              std::unique_ptr<DistillerPage>());

  // OnArticleReady shouldn't be called until distillation finishes (i.e. the
  // blob fetcher shouldn't return distilled content).
  EXPECT_CALL(viewer_delegate, OnArticleReady(_)).Times(0);
  base::RunLoop().RunUntilIdle();

  EXPECT_CALL(viewer_delegate, OnArticleReady(_));
  distiller->RunDistillerCallback(std::move(distilled_article));
  base::RunLoop().RunUntilIdle();

  EXPECT_FALSE(cancel_callback.Cancelled());
}

TEST_F(DomDistillerTaskTrackerTest, TestDistillerFailsFirst) {
  MockDistillerFactory distiller_factory;
  FakeDistiller* distiller = new FakeDistiller(false);
  EXPECT_CALL(distiller_factory, CreateDistillerImpl())
      .WillOnce(Return(distiller));

  ArticleEntry entry(GetDefaultEntry());
  MockContentStore content_store;

  TestCancelCallback cancel_callback;
  TaskTracker task_tracker(
      GetDefaultEntry(), cancel_callback.GetCallback(), &content_store);

  FakeViewRequestDelegate viewer_delegate;
  std::unique_ptr<ViewerHandle> handle(
      task_tracker.AddViewer(&viewer_delegate));

  DistilledContentStore::LoadCallback content_store_load_callback;
  EXPECT_CALL(content_store, LoadContent(_, _))
      .WillOnce(testing::SaveArg<1>(&content_store_load_callback));

  task_tracker.StartDistiller(&distiller_factory,
                              std::unique_ptr<DistillerPage>());
  task_tracker.StartBlobFetcher();

  EXPECT_CALL(viewer_delegate, OnArticleReady(_)).Times(0);
  distiller->RunDistillerCallback(
      std::unique_ptr<DistilledArticleProto>(new DistilledArticleProto));
  base::RunLoop().RunUntilIdle();

  EXPECT_CALL(viewer_delegate, OnArticleReady(_));
  content_store_load_callback.Run(
      true, std::unique_ptr<DistilledArticleProto>(new DistilledArticleProto(
                CreateDistilledArticleForEntry(entry))));
  base::RunLoop().RunUntilIdle();

  EXPECT_FALSE(cancel_callback.Cancelled());
}

TEST_F(DomDistillerTaskTrackerTest, ContentIsSaved) {
  MockDistillerFactory distiller_factory;
  FakeDistiller* distiller = new FakeDistiller(false);
  EXPECT_CALL(distiller_factory, CreateDistillerImpl())
      .WillOnce(Return(distiller));

  ArticleEntry entry(GetDefaultEntry());
  DistilledArticleProto distilled_article =
      CreateDistilledArticleForEntry(entry);

  MockContentStore content_store;
  TestCancelCallback cancel_callback;
  TaskTracker task_tracker(
      GetDefaultEntry(), cancel_callback.GetCallback(), &content_store);

  FakeViewRequestDelegate viewer_delegate;
  std::unique_ptr<ViewerHandle> handle(
      task_tracker.AddViewer(&viewer_delegate));

  DistilledArticleProto stored_distilled_article;
  DistilledContentStore::LoadCallback content_store_load_callback;
  EXPECT_CALL(content_store, SaveContent(_, _, _))
      .WillOnce(testing::SaveArg<1>(&stored_distilled_article));

  task_tracker.StartDistiller(&distiller_factory,
                              std::unique_ptr<DistillerPage>());

  EXPECT_CALL(viewer_delegate, OnArticleReady(_));
  distiller->RunDistillerCallback(std::unique_ptr<DistilledArticleProto>(
      new DistilledArticleProto(distilled_article)));
  base::RunLoop().RunUntilIdle();

  ASSERT_EQ(stored_distilled_article.SerializeAsString(),
            distilled_article.SerializeAsString());
  EXPECT_FALSE(cancel_callback.Cancelled());
}

}  // namespace test
}  // namespace dom_distiller
