// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_COMPONENTS_TETHER_INITIALIZER_H_
#define CHROMEOS_COMPONENTS_TETHER_INITIALIZER_H_

#include "base/macros.h"
#include "base/observer_list.h"

namespace chromeos {

namespace tether {

// Initializes the Tether component.
class Initializer {
 public:
  class Observer {
   public:
    Observer() {}
    virtual ~Observer() {}

    virtual void OnShutdownComplete() = 0;
  };

  enum class Status { ACTIVE, SHUTTING_DOWN, SHUT_DOWN };

  Initializer();
  virtual ~Initializer();

  // Requests that the Tether component shuts down. If the component can be shut
  // down synchronously, this causes Initializer to transition to the SHUT_DOWN
  // status immediately. However, if the component requires an asynchronous
  // shutdown, the class transitions to the SHUTTING_DOWN status; once an
  // asynchronous shutdown completes, Initializer transitions to the SHUT_DOWN
  // status and notifies observers.
  virtual void RequestShutdown() = 0;

  void AddObserver(Observer* observer);
  void RemoveObserver(Observer* observer);

  Status status() { return status_; }
  void TransitionToStatus(Status new_status);

 private:
  Status status_ = Status::ACTIVE;
  base::ObserverList<Observer> observer_list_;

  DISALLOW_COPY_AND_ASSIGN(Initializer);
};

}  // namespace tether

}  // namespace chromeos

#endif  // CHROMEOS_COMPONENTS_TETHER_INITIALIZER_H_
