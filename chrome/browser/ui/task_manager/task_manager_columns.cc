// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/task_manager/task_manager_columns.h"

#include "base/logging.h"
#include "base/macros.h"
#include "build/build_config.h"
#include "chrome/grit/generated_resources.h"

namespace task_manager {

namespace {

// On Mac: Width of "a" and most other letters/digits in "small" table views.
const int kCharWidth = 6;

}  // namespace

// IMPORTANT: Do NOT change the below list without changing the COLUMN_LIST
// macro below.
const TableColumnData kColumns[] = {
    {IDS_TASK_MANAGER_TASK_COLUMN, ui::TableColumn::LEFT, -1, 1, 120, 600, true,
     true, true},
    {IDS_TASK_MANAGER_PROFILE_NAME_COLUMN, ui::TableColumn::LEFT, -1, 0, 60,
     200, true, true, false},
    {IDS_TASK_MANAGER_PHYSICAL_MEM_COLUMN, ui::TableColumn::RIGHT, -1, 0,
     arraysize("800 MiB") * kCharWidth, -1, true, false, true},
    {IDS_TASK_MANAGER_SHARED_MEM_COLUMN, ui::TableColumn::RIGHT, -1, 0,
     arraysize("800 MiB") * kCharWidth, -1, true, false, false},
    {IDS_TASK_MANAGER_PRIVATE_MEM_COLUMN, ui::TableColumn::RIGHT, -1, 0,
     arraysize("800 MiB") * kCharWidth, -1, true, false, false},

#if defined(OS_CHROMEOS)
    {IDS_TASK_MANAGER_SWAPPED_MEM_COLUMN, ui::TableColumn::RIGHT, -1, 0,
     arraysize("800 MiB") * kCharWidth, -1, true, false, false},
#endif

    {IDS_TASK_MANAGER_CPU_COLUMN, ui::TableColumn::RIGHT, -1, 0,
     arraysize("99.9") * kCharWidth, -1, true, false, true},
#if defined(OS_WIN)
    {IDS_TASK_MANAGER_CPU_TIME_COLUMN, ui::TableColumn::RIGHT, -1, 0,
     arraysize("1234h 42m 30s") * kCharWidth, -1, true, false, false},
    {IDS_TASK_MANAGER_START_TIME_COLUMN, ui::TableColumn::RIGHT, -1, 0,
     arraysize("12/13/14 11:44:30 PM") * kCharWidth, -1, true, true, false},
#endif
    {IDS_TASK_MANAGER_NET_COLUMN, ui::TableColumn::RIGHT, -1, 0,
     arraysize("150 kiB/s") * kCharWidth, -1, true, false, true},
    {IDS_TASK_MANAGER_PROCESS_ID_COLUMN, ui::TableColumn::RIGHT, -1, 0,
     arraysize("73099  ") * kCharWidth, -1, true, true, true},

#if defined(OS_WIN)
    {IDS_TASK_MANAGER_GDI_HANDLES_COLUMN, ui::TableColumn::RIGHT, -1, 0, 0, 0,
     true, false, false},
    {IDS_TASK_MANAGER_USER_HANDLES_COLUMN, ui::TableColumn::RIGHT, -1, 0, 0, 0,
     true, false, false},
#endif

    {IDS_TASK_MANAGER_WEBCORE_IMAGE_CACHE_COLUMN, ui::TableColumn::RIGHT, -1, 0,
     arraysize("2000.0K (2000.0 live)") * kCharWidth, -1, true, false, false},
    {IDS_TASK_MANAGER_WEBCORE_SCRIPTS_CACHE_COLUMN, ui::TableColumn::RIGHT, -1,
     0, arraysize("2000.0K (2000.0 live)") * kCharWidth, -1, true, false,
     false},
    {IDS_TASK_MANAGER_WEBCORE_CSS_CACHE_COLUMN, ui::TableColumn::RIGHT, -1, 0,
     arraysize("2000.0K (2000.0 live)") * kCharWidth, -1, true, false, false},
    {IDS_TASK_MANAGER_VIDEO_MEMORY_COLUMN, ui::TableColumn::RIGHT, -1, 0,
     arraysize("2000.0K") * kCharWidth, -1, true, false, false},
    {IDS_TASK_MANAGER_SQLITE_MEMORY_USED_COLUMN, ui::TableColumn::RIGHT, -1, 0,
     arraysize("800 kB") * kCharWidth, -1, true, false, false},

#if !defined(DISABLE_NACL)
    {IDS_TASK_MANAGER_NACL_DEBUG_STUB_PORT_COLUMN, ui::TableColumn::RIGHT, -1,
     0, arraysize("32767") * kCharWidth, -1, true, true, false},
#endif  // !defined(DISABLE_NACL)

    {IDS_TASK_MANAGER_JAVASCRIPT_MEMORY_ALLOCATED_COLUMN,
     ui::TableColumn::RIGHT, -1, 0,
     arraysize("2000.0K (2000.0 live)") * kCharWidth, -1, true, false, false},
    {IDS_TASK_MANAGER_IDLE_WAKEUPS_COLUMN, ui::TableColumn::RIGHT, -1, 0,
     arraysize("idlewakeups") * kCharWidth, -1, true, false, false},

#if defined(OS_LINUX)
    {IDS_TASK_MANAGER_OPEN_FD_COUNT_COLUMN, ui::TableColumn::RIGHT, -1, 0,
     arraysize("999") * kCharWidth, -1, true, false, false},
#endif  // defined(OS_LINUX)
    {IDS_TASK_MANAGER_PROCESS_PRIORITY_COLUMN, ui::TableColumn::LEFT, -1, 0,
     arraysize("background") * kCharWidth, -1, true, true, false},
    {IDS_TASK_MANAGER_MEMORY_STATE_COLUMN, ui::TableColumn::LEFT, -1, 0,
     arraysize("throttled") * kCharWidth, -1, true, false, false},
    {IDS_TASK_MANAGER_KEEPALIVE_COUNT_COLUMN, ui::TableColumn::RIGHT, -1, 0,
     arraysize("999") * kCharWidth, -1, false, false, false},
};

const size_t kColumnsSize = arraysize(kColumns);

const char kSortColumnIdKey[] = "sort_column_id";
const char kSortIsAscendingKey[] = "sort_is_ascending";

// We can't derive session restore keys from the integer IDs of the columns
// since the IDs are generated, and so may change from one build to another.
// Instead we stringify the column ID symbol (i.e. for the ID
// IDS_TASK_MANAGER_TASK_COLUMN, we use the literal string
// "IDS_TASK_MANAGER_TASK_COLUMN").

#define COLUMN_CASE(column_id) \
  case column_id:              \
    return std::string(#column_id);

std::string GetColumnIdAsString(int column_id) {
  switch (column_id) {
    COLUMN_CASE(IDS_TASK_MANAGER_TASK_COLUMN);
    COLUMN_CASE(IDS_TASK_MANAGER_PROFILE_NAME_COLUMN);
    COLUMN_CASE(IDS_TASK_MANAGER_PHYSICAL_MEM_COLUMN);
    COLUMN_CASE(IDS_TASK_MANAGER_SHARED_MEM_COLUMN);
    COLUMN_CASE(IDS_TASK_MANAGER_PRIVATE_MEM_COLUMN);
    COLUMN_CASE(IDS_TASK_MANAGER_SWAPPED_MEM_COLUMN);
    COLUMN_CASE(IDS_TASK_MANAGER_CPU_COLUMN);
    COLUMN_CASE(IDS_TASK_MANAGER_START_TIME_COLUMN);
    COLUMN_CASE(IDS_TASK_MANAGER_CPU_TIME_COLUMN);
    COLUMN_CASE(IDS_TASK_MANAGER_NET_COLUMN);
    COLUMN_CASE(IDS_TASK_MANAGER_PROCESS_ID_COLUMN);
    COLUMN_CASE(IDS_TASK_MANAGER_GDI_HANDLES_COLUMN);
    COLUMN_CASE(IDS_TASK_MANAGER_USER_HANDLES_COLUMN);
    COLUMN_CASE(IDS_TASK_MANAGER_WEBCORE_IMAGE_CACHE_COLUMN);
    COLUMN_CASE(IDS_TASK_MANAGER_WEBCORE_SCRIPTS_CACHE_COLUMN);
    COLUMN_CASE(IDS_TASK_MANAGER_WEBCORE_CSS_CACHE_COLUMN);
    COLUMN_CASE(IDS_TASK_MANAGER_VIDEO_MEMORY_COLUMN);
    COLUMN_CASE(IDS_TASK_MANAGER_SQLITE_MEMORY_USED_COLUMN);
    COLUMN_CASE(IDS_TASK_MANAGER_NACL_DEBUG_STUB_PORT_COLUMN);
    COLUMN_CASE(IDS_TASK_MANAGER_JAVASCRIPT_MEMORY_ALLOCATED_COLUMN);
    COLUMN_CASE(IDS_TASK_MANAGER_IDLE_WAKEUPS_COLUMN);
    COLUMN_CASE(IDS_TASK_MANAGER_OPEN_FD_COUNT_COLUMN);
    COLUMN_CASE(IDS_TASK_MANAGER_PROCESS_PRIORITY_COLUMN);
    COLUMN_CASE(IDS_TASK_MANAGER_MEMORY_STATE_COLUMN);
    COLUMN_CASE(IDS_TASK_MANAGER_KEEPALIVE_COUNT_COLUMN);
    default:
      NOTREACHED();
      return std::string();
  }
}

}  // namespace task_manager
