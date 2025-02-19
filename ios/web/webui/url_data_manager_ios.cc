// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ios/web/webui/url_data_manager_ios.h"

#include <stddef.h>

#include <vector>

#include "base/bind.h"
#include "base/lazy_instance.h"
#include "base/memory/ptr_util.h"
#include "base/memory/ref_counted_memory.h"
#include "base/message_loop/message_loop.h"
#include "base/stl_util.h"
#include "base/strings/string_util.h"
#include "base/synchronization/lock.h"
#include "ios/web/public/browser_state.h"
#include "ios/web/public/url_data_source_ios.h"
#include "ios/web/public/web_thread.h"
#include "ios/web/webui/url_data_manager_ios_backend.h"
#include "ios/web/webui/url_data_source_ios_impl.h"
#include "ios/web/webui/web_ui_ios_data_source_impl.h"

namespace web {
namespace {

const char kURLDataManagerIOSKeyName[] = "url_data_manager";

base::LazyInstance<base::Lock>::Leaky g_delete_lock = LAZY_INSTANCE_INITIALIZER;

URLDataManagerIOS* GetFromBrowserState(BrowserState* browser_state) {
  if (!browser_state->GetUserData(kURLDataManagerIOSKeyName)) {
    browser_state->SetUserData(
        kURLDataManagerIOSKeyName,
        base::MakeUnique<URLDataManagerIOS>(browser_state));
  }
  return static_cast<URLDataManagerIOS*>(
      browser_state->GetUserData(kURLDataManagerIOSKeyName));
}

}  // namespace

// static
URLDataManagerIOS::URLDataSources* URLDataManagerIOS::data_sources_ = NULL;

// static
void URLDataManagerIOS::AddDataSourceOnIOThread(
    BrowserState* browser_state,
    scoped_refptr<URLDataSourceIOSImpl> data_source) {
  DCHECK_CURRENTLY_ON(web::WebThread::IO);
  browser_state->GetURLDataManagerIOSBackendOnIOThread()->AddDataSource(
      data_source.get());
}

URLDataManagerIOS::URLDataManagerIOS(BrowserState* browser_state)
    : browser_state_(browser_state) {
}

URLDataManagerIOS::~URLDataManagerIOS() {
}

void URLDataManagerIOS::AddDataSource(URLDataSourceIOSImpl* source) {
  DCHECK_CURRENTLY_ON(web::WebThread::UI);
  web::WebThread::PostTask(
      web::WebThread::IO, FROM_HERE,
      base::Bind(&AddDataSourceOnIOThread, base::Unretained(browser_state_),
                 make_scoped_refptr(source)));
}

// static
void URLDataManagerIOS::DeleteDataSources() {
  DCHECK_CURRENTLY_ON(web::WebThread::UI);
  URLDataSources sources;
  {
    base::AutoLock lock(g_delete_lock.Get());
    if (!data_sources_)
      return;
    data_sources_->swap(sources);
  }
  for (size_t i = 0; i < sources.size(); ++i)
    delete sources[i];
}

// static
void URLDataManagerIOS::DeleteDataSource(
    const URLDataSourceIOSImpl* data_source) {
  // Invoked when a DataSource is no longer referenced and needs to be deleted.
  if (web::WebThread::CurrentlyOn(web::WebThread::UI)) {
    // We're on the UI thread, delete right away.
    delete data_source;
    return;
  }

  // We're not on the UI thread, add the DataSource to the list of DataSources
  // to delete.
  bool schedule_delete = false;
  {
    base::AutoLock lock(g_delete_lock.Get());
    if (!data_sources_)
      data_sources_ = new URLDataSources();
    schedule_delete = data_sources_->empty();
    data_sources_->push_back(data_source);
  }
  if (schedule_delete) {
    // Schedule a task to delete the DataSource back on the UI thread.
    web::WebThread::PostTask(web::WebThread::UI, FROM_HERE,
                             base::Bind(&URLDataManagerIOS::DeleteDataSources));
  }
}

// static
void URLDataManagerIOS::AddDataSource(BrowserState* browser_state,
                                      URLDataSourceIOS* source) {
  GetFromBrowserState(browser_state)
      ->AddDataSource(new URLDataSourceIOSImpl(source->GetSource(), source));
}

// static
void URLDataManagerIOS::AddWebUIIOSDataSource(BrowserState* browser_state,
                                              WebUIIOSDataSource* source) {
  WebUIIOSDataSourceImpl* impl = static_cast<WebUIIOSDataSourceImpl*>(source);
  GetFromBrowserState(browser_state)->AddDataSource(impl);
}

// static
bool URLDataManagerIOS::IsScheduledForDeletion(
    const URLDataSourceIOSImpl* data_source) {
  base::AutoLock lock(g_delete_lock.Get());
  if (!data_sources_)
    return false;
  return base::ContainsValue(*data_sources_, data_source);
}

}  // namespace web
