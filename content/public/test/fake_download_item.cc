// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/test/fake_download_item.h"

#include "base/bind.h"
#include "net/http/http_response_headers.h"

namespace content {

FakeDownloadItem::FakeDownloadItem() = default;

FakeDownloadItem::~FakeDownloadItem() {
  NotifyDownloadRemoved();
  NotifyDownloadDestroyed();
}

void FakeDownloadItem::AddObserver(Observer* observer) {
  observers_.AddObserver(observer);
}

void FakeDownloadItem::RemoveObserver(Observer* observer) {
  observers_.RemoveObserver(observer);
}

void FakeDownloadItem::NotifyDownloadDestroyed() {
  for (auto& observer : observers_) {
    observer.OnDownloadDestroyed(this);
  }
}

void FakeDownloadItem::NotifyDownloadRemoved() {
  for (auto& observer : observers_) {
    observer.OnDownloadRemoved(this);
  }
}

void FakeDownloadItem::NotifyDownloadUpdated() {
  UpdateObservers();
}

void FakeDownloadItem::UpdateObservers() {
  for (auto& observer : observers_) {
    observer.OnDownloadUpdated(this);
  }
}

void FakeDownloadItem::SetId(uint32_t id) {
  id_ = id;
}

uint32_t FakeDownloadItem::GetId() const {
  return id_;
}

void FakeDownloadItem::SetGuid(const std::string& guid) {
  guid_ = guid;
}

const std::string& FakeDownloadItem::GetGuid() const {
  return guid_;
}

void FakeDownloadItem::SetURL(const GURL& url) {
  url_ = url;
}

const GURL& FakeDownloadItem::GetURL() const {
  return url_;
}

void FakeDownloadItem::SetUrlChain(const std::vector<GURL>& url_chain) {
  url_chain_ = url_chain;
}

const std::vector<GURL>& FakeDownloadItem::GetUrlChain() const {
  return url_chain_;
}

void FakeDownloadItem::SetLastReason(DownloadInterruptReason last_reason) {
  last_reason_ = last_reason;
}

DownloadInterruptReason FakeDownloadItem::GetLastReason() const {
  return last_reason_;
}

void FakeDownloadItem::SetTargetFilePath(const base::FilePath& file_path) {
  file_path_ = file_path;
}

const base::FilePath& FakeDownloadItem::GetTargetFilePath() const {
  return file_path_;
}

void FakeDownloadItem::SetFileExternallyRemoved(
    bool is_file_externally_removed) {
  is_file_externally_removed_ = is_file_externally_removed;
}

bool FakeDownloadItem::GetFileExternallyRemoved() const {
  return is_file_externally_removed_;
}

void FakeDownloadItem::SetStartTime(base::Time start_time) {
  start_time_ = start_time;
}

base::Time FakeDownloadItem::GetStartTime() const {
  return start_time_;
}

void FakeDownloadItem::SetEndTime(base::Time end_time) {
  end_time_ = end_time;
}

base::Time FakeDownloadItem::GetEndTime() const {
  return end_time_;
}

void FakeDownloadItem::SetState(const DownloadState& state) {
  download_state_ = state;
}

DownloadItem::DownloadState FakeDownloadItem::GetState() const {
  return download_state_;
}

void FakeDownloadItem::SetResponseHeaders(
    scoped_refptr<const net::HttpResponseHeaders> response_headers) {
  response_headers_ = response_headers;
}

const scoped_refptr<const net::HttpResponseHeaders>&
FakeDownloadItem::GetResponseHeaders() const {
  return response_headers_;
}

void FakeDownloadItem::SetMimeType(const std::string& mime_type) {
  mime_type_ = mime_type;
}

std::string FakeDownloadItem::GetMimeType() const {
  return mime_type_;
}

void FakeDownloadItem::SetOriginalUrl(const GURL& url) {
  original_url_ = url;
}

const GURL& FakeDownloadItem::GetOriginalUrl() const {
  return original_url_;
}

void FakeDownloadItem::SetReceivedBytes(int64_t received_bytes) {
  received_bytes_ = received_bytes;
}

void FakeDownloadItem::SetTotalBytes(int64_t total_bytes) {
  total_bytes_ = total_bytes;
}

int64_t FakeDownloadItem::GetReceivedBytes() const {
  return received_bytes_;
}

void FakeDownloadItem::SetLastAccessTime(base::Time time) {
  last_access_time_ = time;
}

base::Time FakeDownloadItem::GetLastAccessTime() const {
  return last_access_time_;
}

void FakeDownloadItem::SetIsTransient(bool is_transient) {
  is_transient_ = is_transient;
}

bool FakeDownloadItem::IsTransient() const {
  return is_transient_;
}

// The methods below are not supported and are not expected to be called.
void FakeDownloadItem::ValidateDangerousDownload() {
  NOTREACHED();
}

void FakeDownloadItem::StealDangerousDownload(
    bool delete_file_afterward,
    const AcquireFileCallback& callback) {
  NOTREACHED();
  callback.Run(base::FilePath());
}

void FakeDownloadItem::Pause() {
  NOTREACHED();
}

void FakeDownloadItem::Resume() {
  NOTREACHED();
}

void FakeDownloadItem::Cancel(bool user_cancel) {
  NOTREACHED();
}

void FakeDownloadItem::Remove() {
  NOTREACHED();
}

void FakeDownloadItem::OpenDownload() {
  NOTREACHED();
}

void FakeDownloadItem::ShowDownloadInShell() {
  NOTREACHED();
}

bool FakeDownloadItem::IsPaused() const {
  return false;
}

bool FakeDownloadItem::IsTemporary() const {
  NOTREACHED();
  return false;
}

bool FakeDownloadItem::CanResume() const {
  NOTREACHED();
  return false;
}

bool FakeDownloadItem::IsDone() const {
  NOTREACHED();
  return true;
}

const GURL& FakeDownloadItem::GetReferrerUrl() const {
  NOTREACHED();
  return dummy_url;
}

const GURL& FakeDownloadItem::GetSiteUrl() const {
  NOTREACHED();
  return dummy_url;
}

const GURL& FakeDownloadItem::GetTabUrl() const {
  NOTREACHED();
  return dummy_url;
}

const GURL& FakeDownloadItem::GetTabReferrerUrl() const {
  NOTREACHED();
  return dummy_url;
}

std::string FakeDownloadItem::GetSuggestedFilename() const {
  NOTREACHED();
  return std::string();
}

std::string FakeDownloadItem::GetContentDisposition() const {
  NOTREACHED();
  return std::string();
}

std::string FakeDownloadItem::GetOriginalMimeType() const {
  NOTREACHED();
  return std::string();
}

std::string FakeDownloadItem::GetRemoteAddress() const {
  NOTREACHED();
  return std::string();
}

bool FakeDownloadItem::HasUserGesture() const {
  NOTREACHED();
  return false;
}

ui::PageTransition FakeDownloadItem::GetTransitionType() const {
  NOTREACHED();
  return ui::PageTransition();
}

const std::string& FakeDownloadItem::GetLastModifiedTime() const {
  NOTREACHED();
  return dummy_string;
}

const std::string& FakeDownloadItem::GetETag() const {
  NOTREACHED();
  return dummy_string;
}

bool FakeDownloadItem::IsSavePackageDownload() const {
  NOTREACHED();
  return false;
}

const base::FilePath& FakeDownloadItem::GetFullPath() const {
  return dummy_file_path;
}

const base::FilePath& FakeDownloadItem::GetForcedFilePath() const {
  NOTREACHED();
  return dummy_file_path;
}

base::FilePath FakeDownloadItem::GetFileNameToReportUser() const {
  NOTREACHED();
  return base::FilePath();
}

DownloadItem::TargetDisposition FakeDownloadItem::GetTargetDisposition() const {
  NOTREACHED();
  return TargetDisposition();
}

const std::string& FakeDownloadItem::GetHash() const {
  NOTREACHED();
  return dummy_string;
}

void FakeDownloadItem::DeleteFile(const base::Callback<void(bool)>& callback) {
  NOTREACHED();
  callback.Run(false);
}

bool FakeDownloadItem::IsDangerous() const {
  NOTREACHED();
  return false;
}

DownloadDangerType FakeDownloadItem::GetDangerType() const {
  NOTREACHED();
  return DownloadDangerType();
}

bool FakeDownloadItem::TimeRemaining(base::TimeDelta* remaining) const {
  NOTREACHED();
  return false;
}

int64_t FakeDownloadItem::CurrentSpeed() const {
  NOTREACHED();
  return 1;
}

int FakeDownloadItem::PercentComplete() const {
  NOTREACHED();
  return 1;
}

bool FakeDownloadItem::AllDataSaved() const {
  NOTREACHED();
  return true;
}

int64_t FakeDownloadItem::GetTotalBytes() const {
  return total_bytes_;
}

const std::vector<DownloadItem::ReceivedSlice>&
FakeDownloadItem::GetReceivedSlices() const {
  NOTREACHED();
  static const std::vector<DownloadItem::ReceivedSlice> slices;
  return slices;
}

bool FakeDownloadItem::CanShowInFolder() {
  NOTREACHED();
  return true;
}

bool FakeDownloadItem::CanOpenDownload() {
  NOTREACHED();
  return true;
}

bool FakeDownloadItem::ShouldOpenFileBasedOnExtension() {
  NOTREACHED();
  return true;
}

bool FakeDownloadItem::GetOpenWhenComplete() const {
  NOTREACHED();
  return false;
}

bool FakeDownloadItem::GetAutoOpened() {
  NOTREACHED();
  return false;
}

bool FakeDownloadItem::GetOpened() const {
  NOTREACHED();
  return false;
}

BrowserContext* FakeDownloadItem::GetBrowserContext() const {
  NOTREACHED();
  return nullptr;
}

WebContents* FakeDownloadItem::GetWebContents() const {
  NOTREACHED();
  return nullptr;
}

void FakeDownloadItem::OnContentCheckCompleted(DownloadDangerType danger_type,
                                               DownloadInterruptReason reason) {
  NOTREACHED();
}

void FakeDownloadItem::SetOpenWhenComplete(bool open) {
  NOTREACHED();
}

void FakeDownloadItem::SetOpened(bool opened) {
  NOTREACHED();
}

void FakeDownloadItem::SetDisplayName(const base::FilePath& name) {
  NOTREACHED();
}

std::string FakeDownloadItem::DebugString(bool verbose) const {
  NOTREACHED();
  return std::string();
}

}  // namespace content
