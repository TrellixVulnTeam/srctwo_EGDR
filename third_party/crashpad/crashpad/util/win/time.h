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

#ifndef CRASHPAD_UTIL_WIN_TIME_H_
#define CRASHPAD_UTIL_WIN_TIME_H_

#include <sys/time.h>
#include <windows.h>

namespace crashpad {

//! \brief Convert Windows `FILETIME` to `timeval`, converting from Windows
//!     epoch to POSIX epoch.
timeval FiletimeToTimevalEpoch(const FILETIME& filetime);

//! \brief Convert Windows `FILETIME` to `timeval`, treating the values as
//!     an interval of elapsed time.
timeval FiletimeToTimevalInterval(const FILETIME& filetime);

//! \brief Similar to POSIX gettimeofday(), gets the current system time in UTC.
void GetTimeOfDay(timeval* tv);

}  // namespace crashpad

#endif  // CRASHPAD_UTIL_WIN_TIME_H_
