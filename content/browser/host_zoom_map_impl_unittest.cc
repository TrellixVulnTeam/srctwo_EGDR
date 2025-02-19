// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/host_zoom_map_impl.h"

#include <stddef.h>

#include "base/macros.h"
#include "base/memory/ptr_util.h"
#include "base/memory/ref_counted.h"
#include "base/message_loop/message_loop.h"
#include "base/test/simple_test_clock.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/test/test_browser_thread.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace content {

class HostZoomMapTest : public testing::Test {
 public:
  HostZoomMapTest() : ui_thread_(BrowserThread::UI, &message_loop_) {
  }

 protected:
  base::MessageLoop message_loop_;
  TestBrowserThread ui_thread_;
};

TEST_F(HostZoomMapTest, GetSetZoomLevel) {
  HostZoomMapImpl host_zoom_map;

  double zoomed = 2.5;
  host_zoom_map.SetZoomLevelForHost("zoomed.com", zoomed);

  EXPECT_DOUBLE_EQ(0,
      host_zoom_map.GetZoomLevelForHostAndScheme("http", "normal.com"));
  EXPECT_DOUBLE_EQ(zoomed,
      host_zoom_map.GetZoomLevelForHostAndScheme("http", "zoomed.com"));
}

TEST_F(HostZoomMapTest, GetSetZoomLevelWithScheme) {
  HostZoomMapImpl host_zoom_map;

  double zoomed = 2.5;
  double default_zoom = 1.5;

  host_zoom_map.SetZoomLevelForHostAndScheme("chrome", "login", 0);

  host_zoom_map.SetDefaultZoomLevel(default_zoom);

  EXPECT_DOUBLE_EQ(0,
      host_zoom_map.GetZoomLevelForHostAndScheme("chrome", "login"));
  EXPECT_DOUBLE_EQ(default_zoom,
      host_zoom_map.GetZoomLevelForHostAndScheme("http", "login"));

  host_zoom_map.SetZoomLevelForHost("login", zoomed);

  EXPECT_DOUBLE_EQ(0,
      host_zoom_map.GetZoomLevelForHostAndScheme("chrome", "login"));
  EXPECT_DOUBLE_EQ(zoomed,
      host_zoom_map.GetZoomLevelForHostAndScheme("http", "login"));
}

TEST_F(HostZoomMapTest, GetAllZoomLevels) {
  HostZoomMapImpl host_zoom_map;

  double zoomed = 2.5;
  host_zoom_map.SetZoomLevelForHost("zoomed.com", zoomed);
  host_zoom_map.SetZoomLevelForHostAndScheme("https", "zoomed.com", zoomed);
  host_zoom_map.SetZoomLevelForHostAndScheme("chrome", "login", zoomed);

  HostZoomMap::ZoomLevelVector levels = host_zoom_map.GetAllZoomLevels();
  HostZoomMap::ZoomLevelChange expected[] = {
      {HostZoomMap::ZOOM_CHANGED_FOR_HOST, "zoomed.com", std::string(), zoomed},
      {HostZoomMap::ZOOM_CHANGED_FOR_SCHEME_AND_HOST, "login", "chrome",
       zoomed},
      {HostZoomMap::ZOOM_CHANGED_FOR_SCHEME_AND_HOST, "zoomed.com", "https",
       zoomed}, };
  ASSERT_EQ(arraysize(expected), levels.size());
  for (size_t i = 0; i < arraysize(expected); ++i) {
    SCOPED_TRACE(testing::Message() << "levels[" << i << "]");
    EXPECT_EQ(expected[i].mode, levels[i].mode);
    EXPECT_EQ(expected[i].scheme, levels[i].scheme);
    EXPECT_EQ(expected[i].host, levels[i].host);
    EXPECT_EQ(expected[i].zoom_level, levels[i].zoom_level);
    EXPECT_EQ(expected[i].last_modified, base::Time());
  }
}

TEST_F(HostZoomMapTest, LastModifiedTimestamp) {
  HostZoomMapImpl host_zoom_map;
  host_zoom_map.SetStoreLastModified(true);

  base::Time now = base::Time::Now();
  auto test_clock = base::MakeUnique<base::SimpleTestClock>();
  base::SimpleTestClock* clock = test_clock.get();
  host_zoom_map.SetClockForTesting(std::move(test_clock));

  clock->SetNow(now);
  host_zoom_map.SetZoomLevelForHost("zoomed.com", 1.5);
  host_zoom_map.SetZoomLevelForHost("zoomed2.com", 2.0);

  base::Time later = now + base::TimeDelta::FromSeconds(1);
  clock->SetNow(later);
  host_zoom_map.SetZoomLevelForHost("zoomed2.com", 2.5);
  host_zoom_map.SetZoomLevelForHost("zoomzoom.com", 3);
  host_zoom_map.SetZoomLevelForHostAndScheme("chrome", "login", 3);

  HostZoomMap::ZoomLevelVector levels = host_zoom_map.GetAllZoomLevels();
  std::string scheme;
  HostZoomMap::ZoomLevelChange expected[] = {
      {HostZoomMap::ZOOM_CHANGED_FOR_HOST, "zoomed.com", scheme, 1.5, now},
      {HostZoomMap::ZOOM_CHANGED_FOR_HOST, "zoomed2.com", scheme, 2.5, later},
      {HostZoomMap::ZOOM_CHANGED_FOR_HOST, "zoomzoom.com", scheme, 3.0, later},
      {HostZoomMap::ZOOM_CHANGED_FOR_SCHEME_AND_HOST, "login", "chrome", 3.0,
       base::Time()},
  };
  ASSERT_EQ(arraysize(expected), levels.size());
  for (size_t i = 0; i < arraysize(expected); ++i) {
    SCOPED_TRACE(testing::Message() << "levels[" << i << "]");
    EXPECT_EQ(expected[i].mode, levels[i].mode);
    EXPECT_EQ(expected[i].scheme, levels[i].scheme);
    EXPECT_EQ(expected[i].host, levels[i].host);
    EXPECT_EQ(expected[i].zoom_level, levels[i].zoom_level);
    EXPECT_EQ(expected[i].last_modified, levels[i].last_modified);
  }
}

TEST_F(HostZoomMapTest, ClearZoomLevels) {
  HostZoomMapImpl host_zoom_map;
  host_zoom_map.SetStoreLastModified(true);

  auto test_clock = base::MakeUnique<base::SimpleTestClock>();
  base::SimpleTestClock* clock = test_clock.get();
  host_zoom_map.SetClockForTesting(std::move(test_clock));

  base::Time now = base::Time::Now();
  clock->SetNow(now - base::TimeDelta::FromHours(3));
  host_zoom_map.SetZoomLevelForHost("zoomzoom.com", 3.5);
  clock->SetNow(now - base::TimeDelta::FromHours(1));
  host_zoom_map.SetZoomLevelForHost("zoom.com", 1.5);
  EXPECT_EQ(2u, host_zoom_map.GetAllZoomLevels().size());

  host_zoom_map.ClearZoomLevels(now - base::TimeDelta::FromHours(2),
                                base::Time::Max());
  EXPECT_EQ(1u, host_zoom_map.GetAllZoomLevels().size());
  EXPECT_EQ("zoomzoom.com", host_zoom_map.GetAllZoomLevels()[0].host);

  host_zoom_map.ClearZoomLevels(base::Time(), base::Time::Max());
  EXPECT_EQ(0u, host_zoom_map.GetAllZoomLevels().size());

  // Host and scheme settings should not be affected.
  host_zoom_map.SetZoomLevelForHostAndScheme("chrome", "login", 3);
  host_zoom_map.ClearZoomLevels(base::Time(), base::Time::Max());
  EXPECT_EQ(1u, host_zoom_map.GetAllZoomLevels().size());
}

}  // namespace content
