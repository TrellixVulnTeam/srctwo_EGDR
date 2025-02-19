// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/installer/mini_installer/appid.h"

namespace google_update {

#if defined(GOOGLE_CHROME_BUILD)
const wchar_t kAppGuid[] = L"{8A69D345-D564-463c-AFF1-A69D9E530F96}";
const wchar_t kMultiInstallAppGuid[] =
    L"{4DC8B4CA-1BDA-483e-B5FA-D3C12E15B62D}";
const wchar_t kBetaAppGuid[] = L"{8237E44A-0054-442C-B6B6-EA0509993955}";
const wchar_t kDevAppGuid[] = L"{401C381F-E0DE-4B85-8BD8-3F3F14FBDA57}";
const wchar_t kSxSAppGuid[] = L"{4ea16ac7-fd5a-47c3-875b-dbf4a2008c20}";
#else   // defined(GOOGLE_CHROME_BUILD)
const wchar_t kAppGuid[] = L"";
const wchar_t kMultiInstallAppGuid[] = L"";
#endif  // defined(GOOGLE_CHROME_BUILD)

}  // namespace google_update
