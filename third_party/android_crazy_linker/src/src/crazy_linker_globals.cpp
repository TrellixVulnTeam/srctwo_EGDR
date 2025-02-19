// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crazy_linker_globals.h"

#include <pthread.h>

#include "crazy_linker_system.h"

namespace crazy {

namespace {

Globals* g_globals = NULL;
pthread_once_t g_globals_once = PTHREAD_ONCE_INIT;

void CreateGlobalsInstance() { g_globals = new Globals(); }

}  // namespace

Globals::Globals() : search_paths_(), rdebug_() {
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
  pthread_mutex_init(&lock_, &attr);
  search_paths_.ResetFromEnv("LD_LIBRARY_PATH");
}

Globals::~Globals() { pthread_mutex_destroy(&lock_); }

Globals* Globals::Get() {
  pthread_once(&g_globals_once, CreateGlobalsInstance);
  return g_globals;
}

// static
int Globals::sdk_build_version_ = 0;

}  // namespace crazy
