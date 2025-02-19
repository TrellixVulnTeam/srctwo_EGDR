// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/base/dragdrop/os_exchange_data_provider_factory.h"

#include "base/memory/ptr_util.h"
#include "build/build_config.h"

#if defined(USE_X11)
#include "ui/base/dragdrop/os_exchange_data_provider_aurax11.h"
#elif defined(OS_LINUX)
#include "ui/base/dragdrop/os_exchange_data_provider_aura.h"
#elif defined(OS_MACOSX)
#include "ui/base/dragdrop/os_exchange_data_provider_builder_mac.h"
#elif defined(OS_WIN)
#include "ui/base/dragdrop/os_exchange_data_provider_win.h"
#endif

namespace ui {

OSExchangeDataProviderFactory::Factory* factory_ = nullptr;

// static
void OSExchangeDataProviderFactory::SetFactory(Factory* factory) {
  DCHECK(!factory_ || !factory);
  factory_ = factory;
}

// static
OSExchangeDataProviderFactory::Factory*
OSExchangeDataProviderFactory::TakeFactory() {
  OSExchangeDataProviderFactory::Factory* to_return = factory_;
  factory_ = nullptr;
  return to_return;
}

//static
std::unique_ptr<OSExchangeData::Provider>
OSExchangeDataProviderFactory::CreateProvider() {
  if (factory_)
    return factory_->BuildProvider();

#if defined(USE_X11)
  return base::MakeUnique<OSExchangeDataProviderAuraX11>();
#elif defined(OS_LINUX)
  return base::MakeUnique<OSExchangeDataProviderAura>();
#elif defined(OS_MACOSX)
  return ui::BuildOSExchangeDataProviderMac();
#elif defined(OS_WIN)
  return base::MakeUnique<OSExchangeDataProviderWin>();
#elif defined(OS_FUCHSIA)
  // TODO(fuchsia): Implement this when UI support is added. (crbug.com/750934)
  NOTIMPLEMENTED();
  return nullptr;
#else
#error "Unknown operating system"
#endif
}

}  // namespace ui
