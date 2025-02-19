// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "printing/backend/cups_deleters.h"

namespace printing {

void HttpDeleter::operator()(http_t* http) const {
  httpClose(http);
}

void DestinationDeleter::operator()(cups_dest_t* dest) const {
  cupsFreeDests(1, dest);
}

void DestInfoDeleter::operator()(cups_dinfo_t* info) const {
  cupsFreeDestInfo(info);
}

void OptionDeleter::operator()(cups_option_t* option) const {
  // Frees the name and value buffers then the struct itself
  cupsFreeOptions(1, option);
}

JobsDeleter::JobsDeleter(int num_jobs) : num_jobs_(num_jobs) {}

void JobsDeleter::operator()(cups_job_t* jobs) const {
  cupsFreeJobs(num_jobs_, jobs);
}

}  // namespace printing
