// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_TRANSLATE_TRANSLATE_SERVICE_H_
#define CHROME_BROWSER_TRANSLATE_TRANSLATE_SERVICE_H_

#include "components/web_resource/resource_request_allowed_notifier.h"

class GURL;
class PrefService;

// Singleton managing the resources required for Translate.
class TranslateService
    : public web_resource::ResourceRequestAllowedNotifier::Observer {
 public:
   // Must be called before the Translate feature can be used.
  static void Initialize();

  // Must be called to shut down the Translate feature.
  static void Shutdown(bool cleanup_pending_fetcher);

  // Initializes the TranslateService in a way that it can be initialized
  // multiple times in a unit test suite (once for each test). Should be paired
  // with ShutdownForTesting at the end of the test.
  static void InitializeForTesting();

  // Shuts down the TranslateService at the end of a test in a way that the next
  // test can initialize and use the service.
  static void ShutdownForTesting();

  // Returns true if the new translate bubble is enabled.
  static bool IsTranslateBubbleEnabled();

  // Returns the language to translate to. The language returned is the
  // first language found in the following list that is supported by the
  // translation service:
  //     the UI language
  //     the accept-language list
  // If no language is found then an empty string is returned.
  static std::string GetTargetLanguage(PrefService* prefs);

  // Returns true if the URL can be translated.
  static bool IsTranslatableURL(const GURL& url);

 private:
  TranslateService();
  ~TranslateService();

  // ResourceRequestAllowedNotifier::Observer methods.
  void OnResourceRequestsAllowed() override;

  // Helper class to know if it's allowed to make network resource requests.
  web_resource::ResourceRequestAllowedNotifier
      resource_request_allowed_notifier_;
};

#endif  // CHROME_BROWSER_TRANSLATE_TRANSLATE_SERVICE_H_
