// Copyright 2015 The Crashpad Authors. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <time.h>

extern "C" {

struct tm* gmtime_r(const time_t* timep, struct tm* result) {
  if (gmtime_s(result, timep) != 0)
    return nullptr;
  return result;
}

struct tm* localtime_r(const time_t* timep, struct tm* result) {
  if (localtime_s(result, timep) != 0)
    return nullptr;
  return result;
}

const char* strptime(const char* buf, const char* format, struct tm* tm) {
  // TODO(scottmg): strptime implementation.
  return nullptr;
}

time_t timegm(struct tm* tm) {
  return _mkgmtime(tm);
}

}  // extern "C"
