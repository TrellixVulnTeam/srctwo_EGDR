// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/android/infobars/update_password_infobar.h"

#include <vector>

#include "base/android/jni_android.h"
#include "base/android/jni_array.h"
#include "base/android/jni_string.h"
#include "chrome/browser/password_manager/update_password_infobar_delegate_android.h"
#include "components/password_manager/core/common/credential_manager_types.h"
#include "jni/UpdatePasswordInfoBar_jni.h"

using base::android::JavaParamRef;

UpdatePasswordInfoBar::UpdatePasswordInfoBar(
    std::unique_ptr<UpdatePasswordInfoBarDelegate> delegate)
    : ConfirmInfoBar(std::move(delegate)) {}

UpdatePasswordInfoBar::~UpdatePasswordInfoBar() {}

int UpdatePasswordInfoBar::GetIdOfSelectedUsername() const {
  return Java_UpdatePasswordInfoBar_getSelectedUsername(
      base::android::AttachCurrentThread(), java_infobar_);
}

base::android::ScopedJavaLocalRef<jobject>
UpdatePasswordInfoBar::CreateRenderInfoBar(JNIEnv* env) {
  using base::android::ConvertUTF16ToJavaString;
  using base::android::ScopedJavaLocalRef;
  UpdatePasswordInfoBarDelegate* update_password_delegate =
      static_cast<UpdatePasswordInfoBarDelegate*>(delegate());
  ScopedJavaLocalRef<jstring> ok_button_text = ConvertUTF16ToJavaString(
      env, GetTextFor(ConfirmInfoBarDelegate::BUTTON_OK));
  ScopedJavaLocalRef<jstring> message_text =
      ConvertUTF16ToJavaString(env, update_password_delegate->GetMessageText());

  std::vector<base::string16> usernames;
  if (update_password_delegate->ShowMultipleAccounts()) {
    for (const auto& form : update_password_delegate->GetCurrentForms())
      usernames.push_back(form->username_value);
  } else {
    usernames.push_back(
        update_password_delegate->get_username_for_single_account());
  }

  base::android::ScopedJavaLocalRef<jobject> infobar;
  infobar.Reset(Java_UpdatePasswordInfoBar_show(
      env, GetEnumeratedIconId(),
      base::android::ToJavaArrayOfStrings(env, usernames), message_text,
      update_password_delegate->message_link_range().start(),
      update_password_delegate->message_link_range().end(), ok_button_text));

  java_infobar_.Reset(env, infobar.obj());
  return infobar;
}

void UpdatePasswordInfoBar::OnLinkClicked(JNIEnv* env,
                                          const JavaParamRef<jobject>& obj) {
  GetDelegate()->LinkClicked(WindowOpenDisposition::NEW_FOREGROUND_TAB);
}
