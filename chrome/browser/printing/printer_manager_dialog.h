// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_PRINTING_PRINTER_MANAGER_DIALOG_H_
#define CHROME_BROWSER_PRINTING_PRINTER_MANAGER_DIALOG_H_

#include "base/macros.h"

namespace printing {

// An abstraction of a printer manager dialog. This is used for print preview.
// This includes the OS-dependent UI to manage the network and local printers.
class PrinterManagerDialog {
 public:
  // Displays the native printer manager dialog.
  static void ShowPrinterManagerDialog();

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(PrinterManagerDialog);
};

}  // namespace printing

#endif  // CHROME_BROWSER_PRINTING_PRINTER_MANAGER_DIALOG_H_
