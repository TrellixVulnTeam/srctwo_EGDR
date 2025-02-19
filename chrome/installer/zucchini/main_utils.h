// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_INSTALLER_ZUCCHINI_MAIN_UTILS_H_
#define CHROME_INSTALLER_ZUCCHINI_MAIN_UTILS_H_

#include <iosfwd>

#include "base/files/file_path.h"
#include "chrome/installer/zucchini/zucchini.h"

// Utilities to run Zucchini command based on command-line input, and to print
// help messages.

namespace base {

class CommandLine;

}  // namespace base

// To add a new Zucchini command:
// 1. Declare the command's main function in zucchini_command.h. Its signature
//    must match CommandFunction.
// 2. Define the command's main function in zucchini_command.cc.
// 3. Add a new entry into |kCommands| in main_utils.cc.

// Searches |command_line| for Zucchini commands. If a unique command is found,
// runs it (passes |out| and |err|), and logs resource usage. Otherwise prints
// help message to |err|. Returns Zucchini status code for error handling.
zucchini::status::Code RunZucchiniCommand(const base::CommandLine& command_line,
                                          std::ostream& out,
                                          std::ostream& err);

#endif  // CHROME_INSTALLER_ZUCCHINI_MAIN_UTILS_H_
