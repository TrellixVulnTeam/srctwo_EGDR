// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/printing/printer_manager_dialog.h"

#include <windows.h>
#include <shellapi.h>

#include "base/bind.h"
#include "base/files/file_path.h"
#include "base/path_service.h"
#include "base/task_scheduler/post_task.h"
#include "base/threading/thread.h"

namespace {

// A helper callback that opens the printer management dialog.
void OpenPrintersDialogCallback() {
  base::FilePath sys_dir;
  PathService::Get(base::DIR_SYSTEM, &sys_dir);
  base::FilePath rundll32 = sys_dir.Append(L"rundll32.exe");
  base::FilePath shell32dll = sys_dir.Append(L"shell32.dll");

  std::wstring args(shell32dll.value());
  args.append(L",SHHelpShortcuts_RunDLL PrintersFolder");
  ShellExecute(nullptr, L"open", rundll32.value().c_str(), args.c_str(),
               nullptr, SW_SHOWNORMAL);
}

}  // namespace

namespace printing {

void PrinterManagerDialog::ShowPrinterManagerDialog() {
  base::PostTaskWithTraits(
      FROM_HERE, {base::MayBlock(), base::TaskPriority::USER_BLOCKING},
      base::Bind(OpenPrintersDialogCallback));
}

}  // namespace printing
