// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/devtools/devtools_file_system_indexer.h"

#include <stddef.h>

#include <iterator>

#include "base/bind.h"
#include "base/callback.h"
#include "base/files/file_enumerator.h"
#include "base/files/file_util.h"
#include "base/files/file_util_proxy.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/macros.h"
#include "base/sequence_checker.h"
#include "base/stl_util.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/task_scheduler/lazy_task_runner.h"
#include "base/task_scheduler/post_task.h"

#include "content/public/browser/browser_thread.h"

using base::Bind;
using base::Callback;
using base::FileEnumerator;
using base::FilePath;
using base::Time;
using base::TimeDelta;
using base::TimeTicks;
using content::BrowserThread;
using std::map;
using std::string;
using std::vector;

namespace {

using std::set;

base::SequencedTaskRunner* impl_task_runner() {
  constexpr base::TaskTraits kBlockingTraits = {base::MayBlock(),
                                                base::TaskPriority::BACKGROUND};
  static base::LazySequencedTaskRunner s_sequenced_task_task_runner =
      LAZY_SEQUENCED_TASK_RUNNER_INITIALIZER(kBlockingTraits);
  return s_sequenced_task_task_runner.Get().get();
}

typedef int32_t Trigram;
typedef char TrigramChar;
typedef uint16_t FileId;

const int kMinTimeoutBetweenWorkedNitification = 200;
// Trigram characters include all ASCII printable characters (32-126) except for
// the capital letters, because the index is case insensitive.
const size_t kTrigramCharacterCount = 126 - 'Z' - 1 + 'A' - ' ' + 1;
const size_t kTrigramCount =
    kTrigramCharacterCount * kTrigramCharacterCount * kTrigramCharacterCount;
const int kMaxReadLength = 10 * 1024;
const TrigramChar kUndefinedTrigramChar = -1;
const TrigramChar kBinaryTrigramChar = -2;
const Trigram kUndefinedTrigram = -1;

class Index {
 public:
  Index();
  // Index is only instantiated as a leak LazyInstance, so the destructor is
  // never called.
  ~Index() = delete;

  Time LastModifiedTimeForFile(const FilePath& file_path);
  void SetTrigramsForFile(const FilePath& file_path,
                          const vector<Trigram>& index,
                          const Time& time);
  vector<FilePath> Search(const string& query);
  void NormalizeVectors();

 private:
  FileId GetFileId(const FilePath& file_path);

  typedef map<FilePath, FileId> FileIdsMap;
  FileIdsMap file_ids_;
  FileId last_file_id_;
  // The index in this vector is the trigram id.
  vector<vector<FileId> > index_;
  typedef map<FilePath, Time> IndexedFilesMap;
  IndexedFilesMap index_times_;
  vector<bool> is_normalized_;
  SEQUENCE_CHECKER(sequence_checker_);

  DISALLOW_COPY_AND_ASSIGN(Index);
};

base::LazyInstance<Index>::Leaky g_trigram_index = LAZY_INSTANCE_INITIALIZER;

TrigramChar TrigramCharForChar(char c) {
  static TrigramChar* trigram_chars = nullptr;
  if (!trigram_chars) {
    trigram_chars = new TrigramChar[256];
    for (size_t i = 0; i < 256; ++i) {
      if (i > 127) {
        trigram_chars[i] = kUndefinedTrigramChar;
        continue;
      }
      char ch = static_cast<char>(i);
      if (ch == '\t')
        ch = ' ';
      if (base::IsAsciiUpper(ch))
        ch = ch - 'A' + 'a';

      bool is_binary_char = ch < 9 || (ch >= 14 && ch < 32) || ch == 127;
      if (is_binary_char) {
        trigram_chars[i] = kBinaryTrigramChar;
        continue;
      }

      if (ch < ' ') {
        trigram_chars[i] = kUndefinedTrigramChar;
        continue;
      }

      if (ch >= 'Z')
        ch = ch - 'Z' - 1 + 'A';
      ch -= ' ';
      char signed_trigram_count = static_cast<char>(kTrigramCharacterCount);
      CHECK(ch >= 0 && ch < signed_trigram_count);
      trigram_chars[i] = ch;
    }
  }
  unsigned char uc = static_cast<unsigned char>(c);
  return trigram_chars[uc];
}

Trigram TrigramAtIndex(const vector<TrigramChar>& trigram_chars, size_t index) {
  static int kTrigramCharacterCountSquared =
      kTrigramCharacterCount * kTrigramCharacterCount;
  if (trigram_chars[index] == kUndefinedTrigramChar ||
      trigram_chars[index + 1] == kUndefinedTrigramChar ||
      trigram_chars[index + 2] == kUndefinedTrigramChar)
    return kUndefinedTrigram;
  Trigram trigram = kTrigramCharacterCountSquared * trigram_chars[index] +
                    kTrigramCharacterCount * trigram_chars[index + 1] +
                    trigram_chars[index + 2];
  return trigram;
}

Index::Index() : last_file_id_(0) {
  index_.resize(kTrigramCount);
  is_normalized_.resize(kTrigramCount);
  std::fill(is_normalized_.begin(), is_normalized_.end(), true);
}

Time Index::LastModifiedTimeForFile(const FilePath& file_path) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  Time last_modified_time;
  if (index_times_.find(file_path) != index_times_.end())
    last_modified_time = index_times_[file_path];
  return last_modified_time;
}

void Index::SetTrigramsForFile(const FilePath& file_path,
                               const vector<Trigram>& index,
                               const Time& time) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  FileId file_id = GetFileId(file_path);
  vector<Trigram>::const_iterator it = index.begin();
  for (; it != index.end(); ++it) {
    Trigram trigram = *it;
    index_[trigram].push_back(file_id);
    is_normalized_[trigram] = false;
  }
  index_times_[file_path] = time;
}

vector<FilePath> Index::Search(const string& query) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  const char* data = query.c_str();
  vector<TrigramChar> trigram_chars;
  trigram_chars.reserve(query.size());
  for (size_t i = 0; i < query.size(); ++i) {
      TrigramChar trigram_char = TrigramCharForChar(data[i]);
      if (trigram_char == kBinaryTrigramChar)
        trigram_char = kUndefinedTrigramChar;
      trigram_chars.push_back(trigram_char);
  }
  vector<Trigram> trigrams;
  for (size_t i = 0; i + 2 < query.size(); ++i) {
    Trigram trigram = TrigramAtIndex(trigram_chars, i);
    if (trigram != kUndefinedTrigram)
      trigrams.push_back(trigram);
  }
  set<FileId> file_ids;
  bool first = true;
  vector<Trigram>::const_iterator it = trigrams.begin();
  for (; it != trigrams.end(); ++it) {
    Trigram trigram = *it;
    if (first) {
      std::copy(index_[trigram].begin(),
                index_[trigram].end(),
                std::inserter(file_ids, file_ids.begin()));
      first = false;
      continue;
    }
    set<FileId> intersection = base::STLSetIntersection<set<FileId> >(
        file_ids, index_[trigram]);
    file_ids.swap(intersection);
  }
  vector<FilePath> result;
  FileIdsMap::const_iterator ids_it = file_ids_.begin();
  for (; ids_it != file_ids_.end(); ++ids_it) {
    if (trigrams.size() == 0 ||
        file_ids.find(ids_it->second) != file_ids.end()) {
      result.push_back(ids_it->first);
    }
  }
  return result;
}

FileId Index::GetFileId(const FilePath& file_path) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  string file_path_str = file_path.AsUTF8Unsafe();
  if (file_ids_.find(file_path) != file_ids_.end())
    return file_ids_[file_path];
  file_ids_[file_path] = ++last_file_id_;
  return last_file_id_;
}

void Index::NormalizeVectors() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  for (size_t i = 0; i < kTrigramCount; ++i) {
    if (!is_normalized_[i]) {
      std::sort(index_[i].begin(), index_[i].end());
      if (index_[i].capacity() > index_[i].size())
        vector<FileId>(index_[i]).swap(index_[i]);
      is_normalized_[i] = true;
    }
  }
}

typedef Callback<void(bool, const vector<bool>&)> IndexerCallback;

}  // namespace

DevToolsFileSystemIndexer::FileSystemIndexingJob::FileSystemIndexingJob(
    const FilePath& file_system_path,
    const TotalWorkCallback& total_work_callback,
    const WorkedCallback& worked_callback,
    const DoneCallback& done_callback)
    : file_system_path_(file_system_path),
      total_work_callback_(total_work_callback),
      worked_callback_(worked_callback),
      done_callback_(done_callback),
      files_indexed_(0),
      stopped_(false) {
  current_trigrams_set_.resize(kTrigramCount);
  current_trigrams_.reserve(kTrigramCount);
}

DevToolsFileSystemIndexer::FileSystemIndexingJob::~FileSystemIndexingJob() {}

void DevToolsFileSystemIndexer::FileSystemIndexingJob::Start() {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  impl_task_runner()->PostTask(
      FROM_HERE, BindOnce(&FileSystemIndexingJob::CollectFilesToIndex, this));
}

void DevToolsFileSystemIndexer::FileSystemIndexingJob::Stop() {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  impl_task_runner()->PostTask(
      FROM_HERE, BindOnce(&FileSystemIndexingJob::StopOnImplSequence, this));
}

void DevToolsFileSystemIndexer::FileSystemIndexingJob::StopOnImplSequence() {
  stopped_ = true;
}

void DevToolsFileSystemIndexer::FileSystemIndexingJob::CollectFilesToIndex() {
  DCHECK(impl_task_runner()->RunsTasksInCurrentSequence());
  if (stopped_)
    return;
  if (!file_enumerator_) {
    file_enumerator_.reset(
        new FileEnumerator(file_system_path_, true, FileEnumerator::FILES));
  }
  FilePath file_path = file_enumerator_->Next();
  if (file_path.empty()) {
    BrowserThread::PostTask(
        BrowserThread::UI, FROM_HERE,
        BindOnce(total_work_callback_, file_path_times_.size()));
    indexing_it_ = file_path_times_.begin();
    IndexFiles();
    return;
  }
  Time saved_last_modified_time =
      g_trigram_index.Get().LastModifiedTimeForFile(file_path);
  FileEnumerator::FileInfo file_info = file_enumerator_->GetInfo();
  Time current_last_modified_time = file_info.GetLastModifiedTime();
  if (current_last_modified_time > saved_last_modified_time) {
    file_path_times_[file_path] = current_last_modified_time;
  }
  impl_task_runner()->PostTask(
      FROM_HERE, BindOnce(&FileSystemIndexingJob::CollectFilesToIndex, this));
}

void DevToolsFileSystemIndexer::FileSystemIndexingJob::IndexFiles() {
  DCHECK(impl_task_runner()->RunsTasksInCurrentSequence());
  if (stopped_)
    return;
  if (indexing_it_ == file_path_times_.end()) {
    g_trigram_index.Get().NormalizeVectors();
    BrowserThread::PostTask(BrowserThread::UI, FROM_HERE, done_callback_);
    return;
  }
  FilePath file_path = indexing_it_->first;
  current_file_.Initialize(file_path,
                           base::File::FLAG_OPEN | base::File::FLAG_READ);

  if (!current_file_.IsValid()) {
    FinishFileIndexing(false);
    return;
  }
  current_file_offset_ = 0;
  current_trigrams_.clear();
  std::fill(current_trigrams_set_.begin(), current_trigrams_set_.end(), false);
  ReadFromFile();
}

void DevToolsFileSystemIndexer::FileSystemIndexingJob::ReadFromFile() {
  if (stopped_) {
    CloseFile();
    return;
  }
  std::unique_ptr<char[]> data_ptr(new char[kMaxReadLength]);
  const char* const data = data_ptr.get();
  int bytes_read =
      current_file_.Read(current_file_offset_, data_ptr.get(), kMaxReadLength);
  if (bytes_read < 0) {
    FinishFileIndexing(false);
    return;
  }

  if (bytes_read < 3) {
    FinishFileIndexing(true);
    return;
  }

  size_t size = static_cast<size_t>(bytes_read);
  vector<TrigramChar> trigram_chars;
  trigram_chars.reserve(size);
  for (size_t i = 0; i < size; ++i) {
    TrigramChar trigram_char = TrigramCharForChar(data[i]);
    if (trigram_char == kBinaryTrigramChar) {
      current_trigrams_.clear();
      FinishFileIndexing(true);
      return;
    }
    trigram_chars.push_back(trigram_char);
  }

  for (size_t i = 0; i + 2 < size; ++i) {
    Trigram trigram = TrigramAtIndex(trigram_chars, i);
    if ((trigram != kUndefinedTrigram) && !current_trigrams_set_[trigram]) {
      current_trigrams_set_[trigram] = true;
      current_trigrams_.push_back(trigram);
    }
  }
  current_file_offset_ += bytes_read - 2;
  impl_task_runner()->PostTask(
      FROM_HERE, base::BindOnce(&FileSystemIndexingJob::ReadFromFile, this));
}

void DevToolsFileSystemIndexer::FileSystemIndexingJob::FinishFileIndexing(
    bool success) {
  DCHECK(impl_task_runner()->RunsTasksInCurrentSequence());
  CloseFile();
  if (success) {
    FilePath file_path = indexing_it_->first;
    g_trigram_index.Get().SetTrigramsForFile(
        file_path, current_trigrams_, file_path_times_[file_path]);
  }
  ReportWorked();
  ++indexing_it_;
  impl_task_runner()->PostTask(
      FROM_HERE, base::BindOnce(&FileSystemIndexingJob::IndexFiles, this));
}

void DevToolsFileSystemIndexer::FileSystemIndexingJob::CloseFile() {
  if (current_file_.IsValid())
    current_file_.Close();
}

void DevToolsFileSystemIndexer::FileSystemIndexingJob::ReportWorked() {
  TimeTicks current_time = TimeTicks::Now();
  bool should_send_worked_nitification = true;
  if (!last_worked_notification_time_.is_null()) {
    TimeDelta delta = current_time - last_worked_notification_time_;
    if (delta.InMilliseconds() < kMinTimeoutBetweenWorkedNitification)
      should_send_worked_nitification = false;
  }
  ++files_indexed_;
  if (should_send_worked_nitification) {
    last_worked_notification_time_ = current_time;
    BrowserThread::PostTask(BrowserThread::UI, FROM_HERE,
                            BindOnce(worked_callback_, files_indexed_));
    files_indexed_ = 0;
  }
}

DevToolsFileSystemIndexer::DevToolsFileSystemIndexer() {}

DevToolsFileSystemIndexer::~DevToolsFileSystemIndexer() {}

scoped_refptr<DevToolsFileSystemIndexer::FileSystemIndexingJob>
DevToolsFileSystemIndexer::IndexPath(
    const string& file_system_path,
    const TotalWorkCallback& total_work_callback,
    const WorkedCallback& worked_callback,
    const DoneCallback& done_callback) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  scoped_refptr<FileSystemIndexingJob> indexing_job =
      new FileSystemIndexingJob(FilePath::FromUTF8Unsafe(file_system_path),
                                total_work_callback,
                                worked_callback,
                                done_callback);
  indexing_job->Start();
  return indexing_job;
}

void DevToolsFileSystemIndexer::SearchInPath(
    const std::string& file_system_path,
    const std::string& query,
    const SearchCallback& callback) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  impl_task_runner()->PostTask(
      FROM_HERE,
      BindOnce(&DevToolsFileSystemIndexer::SearchInPathOnImplSequence, this,
               file_system_path, query, callback));
}

void DevToolsFileSystemIndexer::SearchInPathOnImplSequence(
    const std::string& file_system_path,
    const std::string& query,
    const SearchCallback& callback) {
  DCHECK(impl_task_runner()->RunsTasksInCurrentSequence());
  vector<FilePath> file_paths = g_trigram_index.Get().Search(query);
  vector<string> result;
  FilePath path = FilePath::FromUTF8Unsafe(file_system_path);
  vector<FilePath>::const_iterator it = file_paths.begin();
  for (; it != file_paths.end(); ++it) {
    if (path.IsParent(*it))
      result.push_back(it->AsUTF8Unsafe());
  }
  BrowserThread::PostTask(BrowserThread::UI, FROM_HERE,
                          BindOnce(callback, std::move(result)));
}
