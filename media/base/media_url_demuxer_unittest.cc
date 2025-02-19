// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media/base/media_url_demuxer.h"

#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/macros.h"
#include "base/run_loop.h"
#include "base/test/scoped_task_environment.h"
#include "base/threading/thread_task_runner_handle.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace media {

class MediaUrlDemuxerTest : public testing::Test {
 public:
  MediaUrlDemuxerTest()
      : default_media_url_("http://example.com/file.mp4"),
        default_first_party_url_("http://example.com/") {}

  void InitializeTest(const GURL& media_url, const GURL& first_party) {
    demuxer_.reset(new MediaUrlDemuxer(base::ThreadTaskRunnerHandle::Get(),
                                       media_url, first_party));
  }

  void InitializeTest() {
    InitializeTest(default_media_url_, default_first_party_url_);
  }

  void VerifyCallbackOk(PipelineStatus status) {
    EXPECT_EQ(PIPELINE_OK, status);
  }

  GURL default_media_url_;
  GURL default_first_party_url_;
  std::unique_ptr<Demuxer> demuxer_;

  // Necessary, or else base::ThreadTaskRunnerHandle::Get() fails.
  base::test::ScopedTaskEnvironment scoped_task_environment_;

 private:
  DISALLOW_COPY_AND_ASSIGN(MediaUrlDemuxerTest);
};

TEST_F(MediaUrlDemuxerTest, BaseCase) {
  InitializeTest();

  EXPECT_EQ(MediaResource::Type::URL, demuxer_->GetType());

  MediaUrlParams params = demuxer_->GetMediaUrlParams();
  EXPECT_EQ(default_media_url_, params.media_url);
  EXPECT_EQ(default_first_party_url_, params.site_for_cookies);
}

TEST_F(MediaUrlDemuxerTest, AcceptsEmptyStrings) {
  InitializeTest(GURL(), GURL());

  MediaUrlParams params = demuxer_->GetMediaUrlParams();
  EXPECT_EQ(GURL::EmptyGURL(), params.media_url);
  EXPECT_EQ(GURL::EmptyGURL(), params.site_for_cookies);
}

TEST_F(MediaUrlDemuxerTest, InitializeReturnsPipelineOk) {
  InitializeTest();
  demuxer_->Initialize(nullptr,
                       base::Bind(&MediaUrlDemuxerTest::VerifyCallbackOk,
                                  base::Unretained(this)),
                       false);

  base::RunLoop().RunUntilIdle();
}

TEST_F(MediaUrlDemuxerTest, SeekReturnsPipelineOk) {
  InitializeTest();
  demuxer_->Seek(base::TimeDelta(),
                 base::Bind(&MediaUrlDemuxerTest::VerifyCallbackOk,
                            base::Unretained(this)));

  base::RunLoop().RunUntilIdle();
}

}  // namespace media
