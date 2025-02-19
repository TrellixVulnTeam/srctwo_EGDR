// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "remoting/host/host_status_monitor.h"

namespace remoting {

HostStatusMonitor::HostStatusMonitor() {}
HostStatusMonitor::~HostStatusMonitor() {}

void HostStatusMonitor::AddStatusObserver(HostStatusObserver* observer) {
  observers_.AddObserver(observer);
}

void HostStatusMonitor::RemoveStatusObserver(HostStatusObserver* observer) {
  observers_.RemoveObserver(observer);
}

}  // namespace remoting
