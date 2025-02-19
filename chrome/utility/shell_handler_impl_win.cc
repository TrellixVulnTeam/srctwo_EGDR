// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/utility/shell_handler_impl_win.h"

#include <objbase.h>
#include <shldisp.h>

#include "base/files/file_enumerator.h"
#include "base/files/file_path.h"
#include "base/memory/ptr_util.h"
#include "base/path_service.h"
#include "base/scoped_native_library.h"
#include "base/strings/string16.h"
#include "base/win/scoped_bstr.h"
#include "base/win/scoped_com_initializer.h"
#include "base/win/scoped_comptr.h"
#include "base/win/scoped_variant.h"
#include "base/win/shortcut.h"
#include "base/win/win_util.h"
#include "chrome/installer/util/install_util.h"
#include "content/public/utility/utility_thread.h"
#include "mojo/public/cpp/bindings/strong_binding.h"
#include "ui/base/win/open_file_name_win.h"

namespace {

// This class checks if the current executable is pinned to the taskbar. It also
// keeps track of the errors that occurs that prevents it from getting a result.
class IsPinnedToTaskbarHelper {
 public:
  IsPinnedToTaskbarHelper() = default;

  // Returns true if the current executable is pinned to the taskbar.
  bool GetResult();

  bool error_occured() { return error_occured_; }

 private:
  // Returns the shell resource string identified by |resource_id|, or an empty
  // string on error.
  base::string16 LoadShellResourceString(uint32_t resource_id);

  // Returns true if the "Unpin from taskbar" verb is available for |shortcut|,
  // which means that the shortcut is pinned to the taskbar.
  bool ShortcutHasUnpinToTaskbarVerb(const base::FilePath& shortcut);

  // Returns true if the target parameter of the |shortcut| evaluates to
  // |program_compare|.
  bool IsShortcutForProgram(const base::FilePath& shortcut,
                            const InstallUtil::ProgramCompare& program_compare);

  // Returns true if one of the shortcut inside the given |directory| evaluates
  // to |program_compare| and is pinned to the taskbar.
  bool DirectoryContainsPinnedShortcutForProgram(
      const base::FilePath& directory,
      const InstallUtil::ProgramCompare& program_compare);

  bool error_occured_ = false;
  base::win::ScopedCOMInitializer scoped_com_initializer_;

  DISALLOW_COPY_AND_ASSIGN(IsPinnedToTaskbarHelper);
};

base::string16 IsPinnedToTaskbarHelper::LoadShellResourceString(
    uint32_t resource_id) {
  base::ScopedNativeLibrary scoped_native_library(::LoadLibraryEx(
      FILE_PATH_LITERAL("shell32.dll"), nullptr,
      LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE));
  if (!scoped_native_library.is_valid())
    return base::string16();

  const wchar_t* resource_ptr = nullptr;
  int length = ::LoadStringW(scoped_native_library.get(), resource_id,
                             reinterpret_cast<wchar_t*>(&resource_ptr), 0);
  if (!length || !resource_ptr)
    return base::string16();
  return base::string16(resource_ptr, length);
}

bool IsPinnedToTaskbarHelper::ShortcutHasUnpinToTaskbarVerb(
    const base::FilePath& shortcut) {
  // Found inside shell32.dll's resources.
  constexpr uint32_t kUnpinFromTaskbarID = 5387;

  base::string16 verb_name(LoadShellResourceString(kUnpinFromTaskbarID));
  if (verb_name.empty()) {
    error_occured_ = true;
    return false;
  }

  base::win::ScopedComPtr<IShellDispatch> shell_dispatch;
  HRESULT hresult =
      ::CoCreateInstance(CLSID_Shell, nullptr, CLSCTX_INPROC_SERVER,
                         IID_PPV_ARGS(&shell_dispatch));
  if (FAILED(hresult) || !shell_dispatch) {
    error_occured_ = true;
    return false;
  }

  base::win::ScopedComPtr<Folder> folder;
  hresult = shell_dispatch->NameSpace(
      base::win::ScopedVariant(shortcut.DirName().value().c_str()),
      folder.GetAddressOf());
  if (FAILED(hresult) || !folder) {
    error_occured_ = true;
    return false;
  }

  base::win::ScopedComPtr<FolderItem> item;
  hresult = folder->ParseName(
      base::win::ScopedBstr(shortcut.BaseName().value().c_str()),
      item.GetAddressOf());
  if (FAILED(hresult) || !item) {
    error_occured_ = true;
    return false;
  }

  base::win::ScopedComPtr<FolderItemVerbs> verbs;
  hresult = item->Verbs(verbs.GetAddressOf());
  if (FAILED(hresult) || !verbs) {
    error_occured_ = true;
    return false;
  }

  long verb_count = 0;
  hresult = verbs->get_Count(&verb_count);
  if (FAILED(hresult)) {
    error_occured_ = true;
    return false;
  }

  long error_count = 0;
  for (long i = 0; i < verb_count; ++i) {
    base::win::ScopedComPtr<FolderItemVerb> verb;
    hresult =
        verbs->Item(base::win::ScopedVariant(i, VT_I4), verb.GetAddressOf());
    if (FAILED(hresult) || !verb) {
      error_count++;
      continue;
    }
    base::win::ScopedBstr name;
    hresult = verb->get_Name(name.Receive());
    if (FAILED(hresult)) {
      error_count++;
      continue;
    }
    if (base::StringPiece16(name, name.Length()) == verb_name)
      return true;
  }

  if (error_count == verb_count)
    error_occured_ = true;

  return false;
}

bool IsPinnedToTaskbarHelper::IsShortcutForProgram(
    const base::FilePath& shortcut,
    const InstallUtil::ProgramCompare& program_compare) {
  base::win::ShortcutProperties shortcut_properties;
  if (!ResolveShortcutProperties(
          shortcut, base::win::ShortcutProperties::PROPERTIES_TARGET,
          &shortcut_properties)) {
    return false;
  }

  return program_compare.EvaluatePath(shortcut_properties.target);
}

bool IsPinnedToTaskbarHelper::DirectoryContainsPinnedShortcutForProgram(
    const base::FilePath& directory,
    const InstallUtil::ProgramCompare& program_compare) {
  base::FileEnumerator shortcut_enum(directory, false,
                                     base::FileEnumerator::FILES);
  for (base::FilePath shortcut = shortcut_enum.Next(); !shortcut.empty();
       shortcut = shortcut_enum.Next()) {
    if (IsShortcutForProgram(shortcut, program_compare) &&
        ShortcutHasUnpinToTaskbarVerb(shortcut)) {
      return true;
    }
  }
  return false;
}

bool IsPinnedToTaskbarHelper::GetResult() {
  base::FilePath current_exe;
  PathService::Get(base::FILE_EXE, &current_exe);

  InstallUtil::ProgramCompare current_exe_compare(current_exe);
  // Look into the "Quick Launch\User Pinned\TaskBar" folder.
  base::FilePath taskbar_pins_dir;
  PathService::Get(base::DIR_TASKBAR_PINS, &taskbar_pins_dir);
  if (DirectoryContainsPinnedShortcutForProgram(taskbar_pins_dir,
                                                current_exe_compare)) {
    return true;
  }

  // Check all folders in ImplicitAppShortcuts.
  base::FilePath implicit_app_shortcuts_dir;
  PathService::Get(base::DIR_IMPLICIT_APP_SHORTCUTS,
                   &implicit_app_shortcuts_dir);
  base::FileEnumerator directory_enum(implicit_app_shortcuts_dir, false,
                                      base::FileEnumerator::DIRECTORIES);
  for (base::FilePath directory = directory_enum.Next(); !directory.empty();
       directory = directory_enum.Next()) {
    if (DirectoryContainsPinnedShortcutForProgram(directory,
                                                  current_exe_compare)) {
      return true;
    }
  }
  return false;
}

}  // namespace

ShellHandlerImpl::ShellHandlerImpl() = default;

ShellHandlerImpl::~ShellHandlerImpl() = default;

// static
void ShellHandlerImpl::Create(
    chrome::mojom::ShellHandlerRequest request) {
  mojo::MakeStrongBinding(base::MakeUnique<ShellHandlerImpl>(),
                          std::move(request));
}

void ShellHandlerImpl::IsPinnedToTaskbar(
    const IsPinnedToTaskbarCallback& callback) {
  IsPinnedToTaskbarHelper helper;
  bool is_pinned_to_taskbar = helper.GetResult();
  callback.Run(!helper.error_occured(), is_pinned_to_taskbar);
}

void ShellHandlerImpl::CallGetOpenFileName(
    uint32_t owner,
    uint32_t flags,
    const std::vector<std::tuple<base::string16, base::string16>>& filters,
    const base::FilePath& initial_directory,
    const base::FilePath& initial_filename,
    const CallGetOpenFileNameCallback& callback) {
  ui::win::OpenFileName open_file_name(
      reinterpret_cast<HWND>(base::win::Uint32ToHandle(owner)), flags);

  open_file_name.SetInitialSelection(initial_directory, initial_filename);
  open_file_name.SetFilters(filters);

  base::FilePath directory;
  std::vector<base::FilePath> files;
  if (::GetOpenFileName(open_file_name.GetOPENFILENAME()))
    open_file_name.GetResult(&directory, &files);

  if (!files.empty()) {
    callback.Run(directory, files);
  } else {
    callback.Run(base::FilePath(), std::vector<base::FilePath>());
  }
}

void ShellHandlerImpl::CallGetSaveFileName(
    uint32_t owner,
    uint32_t flags,
    const std::vector<std::tuple<base::string16, base::string16>>& filters,
    uint32_t one_based_filter_index,
    const base::FilePath& initial_directory,
    const base::FilePath& suggested_filename,
    const base::string16& default_extension,
    const CallGetSaveFileNameCallback& callback) {
  ui::win::OpenFileName open_file_name(
      reinterpret_cast<HWND>(base::win::Uint32ToHandle(owner)), flags);

  open_file_name.SetInitialSelection(initial_directory, suggested_filename);
  open_file_name.SetFilters(filters);
  open_file_name.GetOPENFILENAME()->nFilterIndex = one_based_filter_index;
  open_file_name.GetOPENFILENAME()->lpstrDefExt = default_extension.c_str();

  if (::GetSaveFileName(open_file_name.GetOPENFILENAME())) {
    callback.Run(base::FilePath(open_file_name.GetOPENFILENAME()->lpstrFile),
                 open_file_name.GetOPENFILENAME()->nFilterIndex);
    return;
  }

  // Error code 0 means the dialog was closed, otherwise there was an error.
  if (DWORD error_code = ::CommDlgExtendedError())
    NOTREACHED() << "::GetSaveFileName() failed: error code " << error_code;

  callback.Run(base::FilePath(), 0);
}
