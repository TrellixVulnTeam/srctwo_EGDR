// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CONFLICTS_INSTALLED_PROGRAMS_WIN_H_
#define CHROME_BROWSER_CONFLICTS_INSTALLED_PROGRAMS_WIN_H_

#include <memory>
#include <utility>
#include <vector>

#include "base/callback_forward.h"
#include "base/files/file_path.h"
#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "base/sequence_checker.h"
#include "base/strings/string16.h"

class MsiUtil;

// This class inspects the user's installed programs and builds a mapping of
// files to its associated program name.
//
// Installed programs are found by searching the
// "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall" registry key and
// its variants. There are 2 cases that are covered:
//
// 1 - If the program's installer did its due dilligence, it populated the
//     "InstallLocation" registry key with the directory where it was installed,
//     and all the files under that directory are assumed to be owned by this
//     program.
//
//     In the event of 2 conflicting "InstallLocation", both are ignored as this
//     method doesn't let us know for sure who is the owner of any enclosing
//     files.
//
// 2 - If the program's entry is a valid MSI Product GUID, the complete list of
//     associated file is used to exactly match a given file to a program.
//
//     If multiple products installed the same file as the same component,
//     Windows keeps a reference count of that component so that the file
//     doesn't get removed if one of them is uninstalled. So both programs are
//     returned by GetInstalledProgramNames().
//
//  Note: Programs may be skipped and so would not be returned by
//        GetInstalledProgramNames() for the following reasons:
//        - The program is owned by Microsoft.
//        - The uninstall entry is marked as a system component.
//        - The uninstall entry has no display name.
//        - The uninstall entry has no UninstallString.
//
// This class is not thread safe and should be only accessed on the same
// sequence.
class InstalledPrograms {
 public:
  // The guts of the class, this structure is populated on a background sequence
  // and moved back to this instance after initialization.
  struct ProgramsData {
    ProgramsData();
    ~ProgramsData();

    // Program names are stored in this vector because multiple entries in
    // |installed_files_map_| could point to the same one. This is to avoid
    // duplicating them.
    std::vector<base::string16> program_names;

    // Contains all the files from programs installed via Microsoft Installer.
    // The second part of the pair is the index into |program_names|.
    std::vector<std::pair<base::FilePath, size_t>> installed_files;

    // For some programs, the best information available is the directory of the
    // installation. The compare functor treats file paths where one is the
    // parent of the other as equal.
    // The second part of the pair is the index into |program_names|.
    std::vector<std::pair<base::FilePath, size_t>> install_directories;
  };

  InstalledPrograms();
  ~InstalledPrograms();

  // Initializes this class on a background sequence.
  void Initialize(const base::Closure& on_initialized_callback);

  // Given a |file|, checks if it matches with an installed program on the
  // user's machine and returns all the matching program names. Do not call this
  // before the initialization is done.
  bool GetInstalledProgramNames(const base::FilePath& file,
                                std::vector<base::string16>* program_names);

 protected:
  // Protected so that tests can subclass InstalledPrograms and access it.
  void Initialize(const base::Closure& on_initialized_callback,
                  std::unique_ptr<MsiUtil> msi_util);

 private:
  bool GetNamesFromInstalledFiles(const base::FilePath& file,
                                  std::vector<base::string16>* program_names);
  bool GetNamesFromInstallDirectories(
      const base::FilePath& file,
      std::vector<base::string16>* program_names);

  // Takes the result from the initialization and moves it to this instance,
  // then calls |on_initialized_callback| to indicate that the initialization
  // is done.
  void OnInitializationDone(const base::Closure& on_initialized_callback,
                            std::unique_ptr<ProgramsData> programs_data);

  // Used to assert that initialization was completed before calling
  // GetInstalledProgramNames().
  bool initialized_;

  std::unique_ptr<ProgramsData> programs_data_;

  // Make sure this class isn't accessed via multiple sequences.
  SEQUENCE_CHECKER(sequence_checker_);

  base::WeakPtrFactory<InstalledPrograms> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(InstalledPrograms);
};

#endif  // CHROME_BROWSER_CONFLICTS_INSTALLED_PROGRAMS_WIN_H_
