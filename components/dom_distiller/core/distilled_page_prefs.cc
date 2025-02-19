// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/dom_distiller/core/distilled_page_prefs.h"

#include "base/bind.h"
#include "base/location.h"
#include "base/memory/weak_ptr.h"
#include "base/observer_list.h"
#include "base/single_thread_task_runner.h"
#include "base/threading/thread_task_runner_handle.h"
#include "components/pref_registry/pref_registry_syncable.h"
#include "components/prefs/pref_service.h"

namespace {

// Path to the integer corresponding to user's preference theme.
const char kFontPref[] = "dom_distiller.font_family";
// Path to the integer corresponding to user's preference font family.
const char kThemePref[] = "dom_distiller.theme";
// Path to the float corresponding to user's preference font scaling.
const char kFontScalePref[] = "dom_distiller.font_scale";
}

namespace dom_distiller {

DistilledPagePrefs::DistilledPagePrefs(PrefService* pref_service)
    : pref_service_(pref_service), weak_ptr_factory_(this) {
}

DistilledPagePrefs::~DistilledPagePrefs() {
}

// static
void DistilledPagePrefs::RegisterProfilePrefs(
    user_prefs::PrefRegistrySyncable* registry) {
  registry->RegisterIntegerPref(
      kThemePref,
      DistilledPagePrefs::LIGHT,
      user_prefs::PrefRegistrySyncable::SYNCABLE_PREF);
  registry->RegisterIntegerPref(
      kFontPref,
      DistilledPagePrefs::SANS_SERIF,
      user_prefs::PrefRegistrySyncable::SYNCABLE_PREF);
  registry->RegisterDoublePref(
      kFontScalePref,
      1.0);
}

void DistilledPagePrefs::SetFontFamily(
    DistilledPagePrefs::FontFamily new_font_family) {
  pref_service_->SetInteger(kFontPref, new_font_family);
  base::ThreadTaskRunnerHandle::Get()->PostTask(
      FROM_HERE, base::Bind(&DistilledPagePrefs::NotifyOnChangeFontFamily,
                            weak_ptr_factory_.GetWeakPtr(), new_font_family));
}

DistilledPagePrefs::FontFamily DistilledPagePrefs::GetFontFamily() {
  int font_family = pref_service_->GetInteger(kFontPref);
  if (font_family < 0 || font_family >= DistilledPagePrefs::FONT_FAMILY_COUNT) {
    // Persisted data was incorrect, trying to clean it up by storing the
    // default.
    SetFontFamily(DistilledPagePrefs::SANS_SERIF);
    return DistilledPagePrefs::SANS_SERIF;
  }
  return static_cast<FontFamily>(font_family);
}

void DistilledPagePrefs::SetTheme(DistilledPagePrefs::Theme new_theme) {
  pref_service_->SetInteger(kThemePref, new_theme);
  base::ThreadTaskRunnerHandle::Get()->PostTask(
      FROM_HERE, base::Bind(&DistilledPagePrefs::NotifyOnChangeTheme,
                            weak_ptr_factory_.GetWeakPtr(), new_theme));
}

DistilledPagePrefs::Theme DistilledPagePrefs::GetTheme() {
  int theme = pref_service_->GetInteger(kThemePref);
  if (theme < 0 || theme >= DistilledPagePrefs::THEME_COUNT) {
    // Persisted data was incorrect, trying to clean it up by storing the
    // default.
    SetTheme(DistilledPagePrefs::LIGHT);
    return DistilledPagePrefs::LIGHT;
  }
  return static_cast<Theme>(theme);
}

void DistilledPagePrefs::SetFontScaling(float scaling) {
  pref_service_->SetDouble(kFontScalePref, scaling);
  base::ThreadTaskRunnerHandle::Get()->PostTask(
      FROM_HERE,
      base::Bind(&DistilledPagePrefs::NotifyOnChangeFontScaling,
                 weak_ptr_factory_.GetWeakPtr(),
                 scaling));
}

float DistilledPagePrefs::GetFontScaling() {
  float scaling = pref_service_->GetDouble(kFontScalePref);
  if (scaling < 0.4 || scaling > 2.5) {
    // Persisted data was incorrect, trying to clean it up by storing the
    // default.
    SetFontScaling(1.0);
    return 1.0;
  }
  return scaling;
}

void DistilledPagePrefs::AddObserver(Observer* obs) {
  observers_.AddObserver(obs);
}

void DistilledPagePrefs::RemoveObserver(Observer* obs) {
  observers_.RemoveObserver(obs);
}

void DistilledPagePrefs::NotifyOnChangeFontFamily(
    DistilledPagePrefs::FontFamily new_font_family) {
  for (Observer& observer : observers_)
    observer.OnChangeFontFamily(new_font_family);
}

void DistilledPagePrefs::NotifyOnChangeTheme(
    DistilledPagePrefs::Theme new_theme) {
  for (Observer& observer : observers_)
    observer.OnChangeTheme(new_theme);
}

void DistilledPagePrefs::NotifyOnChangeFontScaling(
    float scaling) {
  for (Observer& observer : observers_)
    observer.OnChangeFontScaling(scaling);
}

}  // namespace dom_distiller
