// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_EDK_SYSTEM_PORTS_PORT_LOCKER_H_
#define MOJO_EDK_SYSTEM_PORTS_PORT_LOCKER_H_

#include <stdint.h>

#include "base/macros.h"
#include "mojo/edk/system/ports/port_ref.h"

namespace mojo {
namespace edk {
namespace ports {

class Port;
class PortRef;

// A helper which must be used to acquire individual Port locks. Any given
// thread may have at most one of these alive at any time. This ensures that
// when multiple ports are locked, they're locked in globally consistent order.
//
// Port locks are acquired upon construction of this object and released upon
// destruction.
class PortLocker {
 public:
  // Constructs a PortLocker over a sequence of |num_ports| contiguous
  // |PortRef*|s. The sequence may be reordered by this constructor, and upon
  // return, all referenced ports' locks are held.
  PortLocker(const PortRef** port_refs, size_t num_ports);
  ~PortLocker();

  // Provides safe access to a PortRef's Port. Note that in release builds this
  // doesn't do anything other than pass through to the private accessor on
  // |port_ref|, but it does force callers to go through a PortLocker to get to
  // the state, thus minimizing the likelihood that they'll go and do something
  // stupid.
  Port* GetPort(const PortRef& port_ref) const {
#if DCHECK_IS_ON()
    // Sanity check when DCHECK is on to ensure this is actually a port whose
    // lock is held by this PortLocker.
    bool is_port_locked = false;
    for (size_t i = 0; i < num_ports_ && !is_port_locked; ++i)
      if (port_refs_[i]->port() == port_ref.port())
        is_port_locked = true;
    DCHECK(is_port_locked);
#endif
    return port_ref.port();
  }

// A helper which can be used to verify that no Port locks are held on the
// current thread. In non-DCHECK builds this is a no-op.
#if DCHECK_IS_ON()
  static void AssertNoPortsLockedOnCurrentThread();
#else
  static void AssertNoPortsLockedOnCurrentThread() {}
#endif

 private:
  const PortRef** const port_refs_;
  const size_t num_ports_;

  DISALLOW_COPY_AND_ASSIGN(PortLocker);
};

// Convenience wrapper for a PortLocker that locks a single port.
class SinglePortLocker {
 public:
  explicit SinglePortLocker(const PortRef* port_ref);
  ~SinglePortLocker();

  Port* port() const { return locker_.GetPort(*port_ref_); }

 private:
  const PortRef* port_ref_;
  PortLocker locker_;

  DISALLOW_COPY_AND_ASSIGN(SinglePortLocker);
};

}  // namespace ports
}  // namespace edk
}  // namespace mojo

#endif  // MOJO_EDK_SYSTEM_PORTS_PORT_LOCKER_H_
