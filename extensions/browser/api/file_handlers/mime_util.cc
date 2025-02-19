// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/browser/api/file_handlers/mime_util.h"

#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/task_scheduler/post_task.h"
#include "base/threading/thread_task_runner_handle.h"
#include "build/build_config.h"
#include "content/public/browser/browser_context.h"
#include "net/base/filename_util.h"
#include "net/base/mime_sniffer.h"
#include "net/base/mime_util.h"
#include "storage/browser/fileapi/file_system_url.h"

#if defined(OS_CHROMEOS)
#include "extensions/browser/api/extensions_api_client.h"
#include "extensions/browser/api/file_handlers/non_native_file_system_delegate.h"
#endif

namespace {

const char kMimeTypeApplicationOctetStream[] = "application/octet-stream";

}  // namespace

namespace extensions {
namespace app_file_handler_util {
namespace {

// Detects MIME type by reading initial bytes from the file. If found, then
// writes the MIME type to |result|.
void SniffMimeType(const base::FilePath& local_path, std::string* result) {
  std::vector<char> content(net::kMaxBytesToSniff);

  const int bytes_read =
      base::ReadFile(local_path, &content[0], static_cast<int>(content.size()));

  if (bytes_read >= 0) {
    net::SniffMimeType(&content[0], bytes_read,
                       net::FilePathToFileURL(local_path),
                       std::string(),  // type_hint (passes no hint)
                       result);
  }
}

#if defined(OS_CHROMEOS)
// Converts a result passed as a scoped pointer to a dereferenced value passed
// to |callback|.
void OnGetMimeTypeFromFileForNonNativeLocalPathCompleted(
    std::unique_ptr<std::string> mime_type,
    const base::Callback<void(const std::string&)>& callback) {
  callback.Run(*mime_type);
}

// Called when fetching MIME type for a non-native local path is completed.
// If |success| is false, then tries to guess the MIME type by looking at the
// file name.
void OnGetMimeTypeFromMetadataForNonNativeLocalPathCompleted(
    const base::FilePath& local_path,
    const base::Callback<void(const std::string&)>& callback,
    bool success,
    const std::string& mime_type) {
  if (success) {
    callback.Run(mime_type);
    return;
  }

  // MIME type not available with metadata, hence try to guess it from the
  // file's extension.
  std::unique_ptr<std::string> mime_type_from_extension(new std::string);
  std::string* const mime_type_from_extension_ptr =
      mime_type_from_extension.get();
  base::PostTaskWithTraitsAndReply(
      FROM_HERE, {base::MayBlock()},
      base::Bind(base::IgnoreResult(&net::GetMimeTypeFromFile), local_path,
                 mime_type_from_extension_ptr),
      base::Bind(&OnGetMimeTypeFromFileForNonNativeLocalPathCompleted,
                 base::Passed(&mime_type_from_extension), callback));
}
#endif

// Called when sniffing for MIME type in the native local file is completed.
void OnSniffMimeTypeForNativeLocalPathCompleted(
    std::unique_ptr<std::string> mime_type,
    const base::Callback<void(const std::string&)>& callback) {
  // Do not return application/zip as sniffed result. If the file has .zip
  // extension, it should be already returned as application/zip. If the file
  // does not have .zip extension and couldn't find mime type from the
  // extension, it might be unknown internally zipped file.
  if (*mime_type == "application/zip") {
    callback.Run(kMimeTypeApplicationOctetStream);
    return;
  }

  callback.Run(*mime_type);
}

}  // namespace

// Handles response of net::GetMimeTypeFromFile for native file systems. If
// MIME type is available, then forwards it to |callback|. Otherwise, fallbacks
// to sniffing.
void OnGetMimeTypeFromFileForNativeLocalPathCompleted(
    const base::FilePath& local_path,
    std::unique_ptr<std::string> mime_type,
    const base::Callback<void(const std::string&)>& callback) {
  if (!mime_type->empty()) {
    callback.Run(*mime_type);
    return;
  }

  std::unique_ptr<std::string> sniffed_mime_type(
      new std::string(kMimeTypeApplicationOctetStream));
  std::string* const sniffed_mime_type_ptr = sniffed_mime_type.get();
  base::PostTaskWithTraitsAndReply(
      FROM_HERE, {base::MayBlock()},
      base::Bind(&SniffMimeType, local_path, sniffed_mime_type_ptr),
      base::Bind(&OnSniffMimeTypeForNativeLocalPathCompleted,
                 base::Passed(&sniffed_mime_type), callback));
}

// Fetches MIME type for a local path and returns it with a |callback|.
void GetMimeTypeForLocalPath(
    content::BrowserContext* context,
    const base::FilePath& local_path,
    const base::Callback<void(const std::string&)>& callback) {
#if defined(OS_CHROMEOS)
  NonNativeFileSystemDelegate* delegate =
      ExtensionsAPIClient::Get()->GetNonNativeFileSystemDelegate();
  if (delegate && delegate->IsUnderNonNativeLocalPath(context, local_path)) {
    // For non-native files, try to get the MIME type from metadata. If not
    // available, then try to guess from the extension. Never sniff (because
    // it can be very slow).
    delegate->GetNonNativeLocalPathMimeType(
        context, local_path,
        base::Bind(&OnGetMimeTypeFromMetadataForNonNativeLocalPathCompleted,
                   local_path, callback));
    return;
  }
#endif

  // For native local files, try to guess the mime from the extension. If
  // not available, then try to sniff if.
  std::unique_ptr<std::string> mime_type_from_extension(new std::string);
  std::string* const mime_type_from_extension_ptr =
      mime_type_from_extension.get();
  base::PostTaskWithTraitsAndReply(
      FROM_HERE, {base::MayBlock()},
      base::Bind(base::IgnoreResult(&net::GetMimeTypeFromFile), local_path,
                 mime_type_from_extension_ptr),
      base::Bind(&OnGetMimeTypeFromFileForNativeLocalPathCompleted, local_path,
                 base::Passed(&mime_type_from_extension), callback));
}

MimeTypeCollector::MimeTypeCollector(content::BrowserContext* context)
    : context_(context), left_(0), weak_ptr_factory_(this) {}

MimeTypeCollector::~MimeTypeCollector() {}

void MimeTypeCollector::CollectForURLs(
    const std::vector<storage::FileSystemURL>& urls,
    const CompletionCallback& callback) {
  std::vector<base::FilePath> local_paths;
  for (size_t i = 0; i < urls.size(); ++i) {
    local_paths.push_back(urls[i].path());
  }

  CollectForLocalPaths(local_paths, callback);
}

void MimeTypeCollector::CollectForLocalPaths(
    const std::vector<base::FilePath>& local_paths,
    const CompletionCallback& callback) {
  DCHECK(!callback.is_null());
  callback_ = callback;

  DCHECK(!result_.get());
  result_.reset(new std::vector<std::string>(local_paths.size()));
  left_ = local_paths.size();

  if (!left_) {
    // Nothing to process.
    base::ThreadTaskRunnerHandle::Get()->PostTask(
        FROM_HERE, base::Bind(callback_, base::Passed(&result_)));
    callback_ = CompletionCallback();
    return;
  }

  for (size_t i = 0; i < local_paths.size(); ++i) {
    GetMimeTypeForLocalPath(context_, local_paths[i],
                            base::Bind(&MimeTypeCollector::OnMimeTypeCollected,
                                       weak_ptr_factory_.GetWeakPtr(), i));
  }
}

void MimeTypeCollector::OnMimeTypeCollected(size_t index,
                                            const std::string& mime_type) {
  (*result_)[index] = mime_type;
  if (!--left_) {
    base::ThreadTaskRunnerHandle::Get()->PostTask(
        FROM_HERE, base::Bind(callback_, base::Passed(&result_)));
    // Release the callback to avoid a circullar reference in case an instance
    // of this class is a member of a ref counted class, which instance is bound
    // to this callback.
    callback_ = CompletionCallback();
  }
}

}  // namespace app_file_handler_util
}  // namespace extensions
