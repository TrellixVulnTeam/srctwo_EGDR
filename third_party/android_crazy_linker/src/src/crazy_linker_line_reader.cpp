// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crazy_linker_line_reader.h"

#include "crazy_linker_debug.h"

// Set to 1 to enable debug logs here.
#define DEBUG_LINE_READER 0

#define LLOG(...) LOG_IF(DEBUG_LINE_READER, __VA_ARGS__)

namespace crazy {

LineReader::LineReader() : fd_(), buff_(NULL) {
  Reset(true);
}

LineReader::LineReader(const char* path) : fd_(), buff_(NULL) {
  Open(path);
}

LineReader::~LineReader() {
  ::free(buff_);
}

void LineReader::Open(const char* path) {
  Reset(!fd_.OpenReadOnly(path));
}

void LineReader::Reset(bool eof) {
  eof_ = eof;
  line_start_ = 0;
  line_len_ = 0;
  buff_size_ = 0;
  buff_capacity_ = 128;
  buff_ = static_cast<char*>(::realloc(buff_, buff_capacity_));
}

bool LineReader::GetNextLine() {
  // Eat previous line.
  line_start_ += line_len_;
  line_len_ = 0;

  for (;;) {
    LLOG("%s: LOOP line_start=%d buff_size=%d buff_capacity=%d\n",
         __FUNCTION__,
         line_start_,
         buff_size_,
         buff_capacity_);

    // Find the end of the current line in the current buffer. The result
    // of memchr(_,_,0) is undefined, treated as not-found.
    const char* line = buff_ + line_start_;
    const size_t range = buff_size_ - line_start_;
    const char* line_end;
    if (range > 0)
      line_end = reinterpret_cast<const char*>(::memchr(line, '\n', range));
    else
      line_end = NULL;
    if (line_end != NULL) {
      // Found one, return it directly.
      line_len_ = static_cast<size_t>(line_end + 1 - line);
      LLOG("%s: LINE line_start=%d line_len=%d '%.*s'\n",
           __FUNCTION__,
           line_start_,
           line_len_,
           line_len_,
           buff_ + line_start_);
      return true;
    }

    // Eat the start of the buffer
    if (line_start_ > 0) {
      ::memmove(buff_, buff_ + line_start_, buff_size_ - line_start_);
      buff_size_ -= line_start_;
      line_start_ = 0;
      LLOG("%s: MOVE buff_size=%d\n", __FUNCTION__, buff_size_);
    }

    // Handle end of input now.
    if (eof_) {
      // If there is a last line that isn't terminated by a newline, and
      // there is room for it in the buffer. Manually add a \n and return
      // the line.
      if (buff_size_ > 0 && buff_size_ < buff_capacity_) {
        buff_[buff_size_++] = '\n';
        line_len_ = buff_size_;
        LLOG("%s: EOF_LINE buff_size=%d '%.*s'\n",
             __FUNCTION__,
             buff_size_,
             buff_size_,
             buff_);
        return true;
      }
      // Otherwise, ignore the last line.
      LLOG("%s: EOF\n", __FUNCTION__);
      return false;
    }

    // Before reading more data, grow the buffer if needed.
    if (buff_size_ == buff_capacity_) {
      buff_capacity_ *= 2;
      buff_ = static_cast<char*>(::realloc(buff_, buff_capacity_));

      LLOG("%s: GROW buff_size=%d buff_capacity=%d '%.*s'\n",
           __FUNCTION__,
           buff_size_,
           buff_capacity_,
           buff_size_,
           buff_);
    }

    // Try to fill the rest of buffer after current content.
    size_t avail = buff_capacity_ - buff_size_;
    int ret = fd_.Read(buff_ + buff_size_, avail);
    LLOG("%s: READ buff_size=%d buff_capacity=%d avail=%d ret=%d\n",
         __FUNCTION__,
         buff_size_,
         buff_capacity_,
         avail,
         ret);
    if (ret <= 0) {
      eof_ = true;
      ret = 0;
    }
    buff_size_ += static_cast<size_t>(ret);
  }
}

const char* LineReader::line() const { return buff_ + line_start_; }

size_t LineReader::length() const { return line_len_; }

}  // namespace crazy
