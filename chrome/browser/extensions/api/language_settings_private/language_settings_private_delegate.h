// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_EXTENSIONS_API_LANGUAGE_SETTINGS_PRIVATE_LANGUAGE_SETTINGS_PRIVATE_DELEGATE_H_
#define CHROME_BROWSER_EXTENSIONS_API_LANGUAGE_SETTINGS_PRIVATE_LANGUAGE_SETTINGS_PRIVATE_DELEGATE_H_

#include <vector>

#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "chrome/browser/spellchecker/spellcheck_custom_dictionary.h"
#include "chrome/browser/spellchecker/spellcheck_hunspell_dictionary.h"
#include "chrome/common/extensions/api/language_settings_private.h"
#include "components/keyed_service/core/keyed_service.h"
#include "components/prefs/pref_change_registrar.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"
#include "extensions/browser/event_router.h"

namespace content {
class BrowserContext;
}

namespace extensions {

// Observes language settings and routes changes as events to listeners of the
// languageSettingsPrivate API.
class LanguageSettingsPrivateDelegate
    : public KeyedService,
      public EventRouter::Observer,
      public content::NotificationObserver,
      public SpellcheckHunspellDictionary::Observer,
      public SpellcheckCustomDictionary::Observer {
 public:
  static LanguageSettingsPrivateDelegate* Create(
      content::BrowserContext* browser_context);
  ~LanguageSettingsPrivateDelegate() override;

  // Returns the languages and statuses of the enabled spellcheck dictionaries.
  std::vector<api::language_settings_private::SpellcheckDictionaryStatus>
  GetHunspellDictionaryStatuses();

 protected:
  explicit LanguageSettingsPrivateDelegate(content::BrowserContext* context);

  // KeyedService implementation.
  void Shutdown() override;

  // EventRouter::Observer implementation.
  void OnListenerAdded(const EventListenerInfo& details) override;
  void OnListenerRemoved(const EventListenerInfo& details) override;

  // content::NotificationObserver implementation.
  void Observe(int type,
               const content::NotificationSource& source,
               const content::NotificationDetails& details) override;

  // SpellcheckHunspellDictionary::Observer implementation.
  void OnHunspellDictionaryInitialized(const std::string& language) override;
  void OnHunspellDictionaryDownloadBegin(const std::string& language) override;
  void OnHunspellDictionaryDownloadSuccess(
      const std::string& language) override;
  void OnHunspellDictionaryDownloadFailure(
      const std::string& language) override;

  // SpellcheckCustomDictionary::Observer implementation.
  void OnCustomDictionaryLoaded() override;
  void OnCustomDictionaryChanged(
      const SpellcheckCustomDictionary::Change& dictionary_change) override;

 private:
  typedef std::vector<base::WeakPtr<SpellcheckHunspellDictionary>>
      WeakDictionaries;

  // Updates the dictionaries that are used for spellchecking.
  void RefreshDictionaries(bool was_listening, bool should_listen);

  // Returns the hunspell dictionaries that are used for spellchecking.
  const WeakDictionaries& GetHunspellDictionaries();

  // If there are any JavaScript listeners registered for spellcheck events,
  // ensures we are registered for change notifications. Otherwise, unregisters
  // any observers.
  void StartOrStopListeningForSpellcheckChanges();

  // Handles the preference for which languages should be used for spellcheck
  // by resetting the dictionaries and broadcasting an event.
  void OnSpellcheckDictionariesChanged();

  // Broadcasts an event with the list of spellcheck dictionary statuses.
  void BroadcastDictionariesChangedEvent();

  // Removes observers from hunspell_dictionaries_.
  void RemoveDictionaryObservers();

  // The hunspell dictionaries that are used for spellchecking.
  WeakDictionaries hunspell_dictionaries_;

  // The custom dictionary that is used for spellchecking.
  SpellcheckCustomDictionary* custom_dictionary_;

  content::BrowserContext* context_;

  // True if there are observers listening for spellcheck events.
  bool listening_spellcheck_;

  // True if the profile has finished initializing.
  bool profile_added_;

  content::NotificationRegistrar notification_registrar_;

  PrefChangeRegistrar pref_change_registrar_;

  DISALLOW_COPY_AND_ASSIGN(LanguageSettingsPrivateDelegate);
};

}  // namespace extensions

#endif  // CHROME_BROWSER_EXTENSIONS_API_LANGUAGE_SETTINGS_PRIVATE_LANGUAGE_SETTINGS_PRIVATE_DELEGATE_H_
