// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/wm/core/default_activation_client.h"

#include "base/macros.h"
#include "ui/aura/window.h"
#include "ui/wm/public/activation_change_observer.h"
#include "ui/wm/public/activation_delegate.h"

namespace wm {

// Takes care of observing root window destruction & destroying the client.
class DefaultActivationClient::Deleter : public aura::WindowObserver {
 public:
  Deleter(DefaultActivationClient* client, aura::Window* root_window)
      : client_(client),
        root_window_(root_window) {
    root_window_->AddObserver(this);
  }

 private:
  ~Deleter() override {}

  // Overridden from WindowObserver:
  void OnWindowDestroyed(aura::Window* window) override {
    DCHECK_EQ(window, root_window_);
    root_window_->RemoveObserver(this);
    delete client_;
    delete this;
  }

  DefaultActivationClient* client_;
  aura::Window* root_window_;

  DISALLOW_COPY_AND_ASSIGN(Deleter);
};

////////////////////////////////////////////////////////////////////////////////
// DefaultActivationClient, public:

DefaultActivationClient::DefaultActivationClient(aura::Window* root_window)
    : last_active_(nullptr) {
  SetActivationClient(root_window, this);
  new Deleter(this, root_window);
}

////////////////////////////////////////////////////////////////////////////////
// DefaultActivationClient, ActivationClient implementation:

void DefaultActivationClient::AddObserver(ActivationChangeObserver* observer) {
  observers_.AddObserver(observer);
}

void DefaultActivationClient::RemoveObserver(
    ActivationChangeObserver* observer) {
  observers_.RemoveObserver(observer);
}

void DefaultActivationClient::ActivateWindow(aura::Window* window) {
  ActivateWindowImpl(
      ActivationChangeObserver::ActivationReason::ACTIVATION_CLIENT, window);
}

void DefaultActivationClient::ActivateWindowImpl(
    ActivationChangeObserver::ActivationReason reason,
    aura::Window* window) {
  aura::Window* last_active = ActivationClient::GetActiveWindow();
  if (last_active == window)
    return;

  last_active_ = last_active;
  RemoveActiveWindow(window);
  active_windows_.push_back(window);
  window->parent()->StackChildAtTop(window);
  window->AddObserver(this);

  for (auto& observer : observers_)
    observer.OnWindowActivated(reason, window, last_active);

  ActivationChangeObserver* observer = GetActivationChangeObserver(last_active);
  if (observer) {
    observer->OnWindowActivated(reason, window, last_active);
  }
  observer = GetActivationChangeObserver(window);
  if (observer) {
    observer->OnWindowActivated(reason, window, last_active);
  }
}

void DefaultActivationClient::DeactivateWindow(aura::Window* window) {
  ActivationChangeObserver* observer = GetActivationChangeObserver(window);
  if (observer) {
    observer->OnWindowActivated(
        ActivationChangeObserver::ActivationReason::ACTIVATION_CLIENT, nullptr,
        window);
  }
  if (last_active_)
    ActivateWindow(last_active_);
}

const aura::Window* DefaultActivationClient::GetActiveWindow() const {
  if (active_windows_.empty())
    return nullptr;
  return active_windows_.back();
}

aura::Window* DefaultActivationClient::GetActivatableWindow(
    aura::Window* window) {
  return nullptr;
}

aura::Window* DefaultActivationClient::GetToplevelWindow(aura::Window* window) {
  return nullptr;
}

bool DefaultActivationClient::CanActivateWindow(aura::Window* window) const {
  return true;
}

////////////////////////////////////////////////////////////////////////////////
// DefaultActivationClient, aura::WindowObserver implementation:

void DefaultActivationClient::OnWindowDestroyed(aura::Window* window) {
  if (window == last_active_)
    last_active_ = nullptr;

  if (window == GetActiveWindow()) {
    active_windows_.pop_back();
    aura::Window* next_active = ActivationClient::GetActiveWindow();
    if (next_active && GetActivationChangeObserver(next_active)) {
      GetActivationChangeObserver(next_active)
          ->OnWindowActivated(ActivationChangeObserver::ActivationReason::
                                  WINDOW_DISPOSITION_CHANGED,
                              next_active, nullptr);
    }
    return;
  }

  RemoveActiveWindow(window);
}

////////////////////////////////////////////////////////////////////////////////
// DefaultActivationClient, private:

DefaultActivationClient::~DefaultActivationClient() {
  for (unsigned int i = 0; i < active_windows_.size(); ++i) {
    active_windows_[i]->RemoveObserver(this);
  }
}

void DefaultActivationClient::RemoveActiveWindow(aura::Window* window) {
  for (unsigned int i = 0; i < active_windows_.size(); ++i) {
    if (active_windows_[i] == window) {
      active_windows_.erase(active_windows_.begin() + i);
      window->RemoveObserver(this);
      return;
    }
  }
}

}  // namespace wm
