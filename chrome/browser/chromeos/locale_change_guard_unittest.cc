// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/locale_change_guard.h"

#include <stddef.h>
#include <string.h>

#include "base/macros.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/base/l10n/l10n_util.h"

namespace {

// These languages require user notification when locale is automatically
// switched between different regions within the same language.
const char* const kShowNotificationLanguages[] = {
    "af",   // Afrikaans
    "am",   // Amharic
    "an",   // Aragonese
    "ar",   // Arabic
    "ast",  // Asturian
    "az",   // Azerbaijani
    "be",   // Belarusian
    "bg",   // Bulgarian
    "bh",   // Bihari
    "bn",   // Bengali
    "br",   // Breton
    "bs",   // Bosnian
    "ca",   // Catalan
    "ckb",  // Sorani (Kurdish-Arabic)
    "co",   // Corsican
    "cs",   // Czech
    "cy",   // Welsh
    "da",   // Danish
    "el",   // Greek
    "eo",   // Esperanto
    "es",   // Spanish
    "et",   // Estonian
    "eu",   // Basque
    "fa",   // Persian
    "fi",   // Finnish
    "fil",  // Filipino
    "fo",   // Faroese
    "fy",   // Frisian
    "ga",   // Irish
    "gd",   // Scots Gaelic
    "gl",   // Galician
    "gn",   // Guarani
    "gu",   // Gujarati
    "ha",   // Hausa
    "haw",  // Hawaiian
    "he",   // Hebrew
    "hi",   // Hindi
    "hmn",  // Hmong
    "hr",   // Croatian
    "hu",   // Hungarian
    "hy",   // Armenian
    "ia",   // Interlingua
    "id",   // Indonesian
    "is",   // Icelandic
    "ja",   // Japanese
    "jv",   // Javanese
    "ka",   // Georgian
    "kk",   // Kazakh
    "km",   // Cambodian
    "kn",   // Kannada
    "ko",   // Korean
    "ku",   // Kurdish
    "ky",   // Kyrgyz
    "la",   // Latin
    "lb",   // Luxembourgish
    "ln",   // Lingala
    "lo",   // Laothian
    "lt",   // Lithuanian
    "lv",   // Latvian
    "mk",   // Macedonian
    "ml",   // Malayalam
    "mn",   // Mongolian
    "mo",   // Moldavian
    "mr",   // Marathi
    "ms",   // Malay
    "mt",   // Maltese
    "nb",   // Norwegian (Bokmal)
    "ne",   // Nepali
    "nl",   // Dutch
    "nn",   // Norwegian (Nynorsk)
    "no",   // Norwegian
    "oc",   // Occitan
    "om",   // Oromo
    "or",   // Oriya
    "pa",   // Punjabi
    "pl",   // Polish
    "ps",   // Pashto
    "pt",   // Portuguese
    "qu",   // Quechua
    "rm",   // Romansh
    "ro",   // Romanian
    "ru",   // Russian
    "sd",   // Sindhi
    "sh",   // Serbo-Croatian
    "si",   // Sinhalese
    "sk",   // Slovak
    "sl",   // Slovenian
    "sm",   // Samoan
    "sn",   // Shona
    "so",   // Somali
    "sq",   // Albanian
    "sr",   // Serbian
    "st",   // Sesotho
    "su",   // Sundanese
    "sv",   // Swedish
    "sw",   // Swahili
    "ta",   // Tamil
    "te",   // Telugu
    "tg",   // Tajik
    "th",   // Thai
    "ti",   // Tigrinya
    "tk",   // Turkmen
    "to",   // Tonga
    "tr",   // Turkish
    "tt",   // Tatar
    "tw",   // Twi
    "ug",   // Uighur
    "uk",   // Ukrainian
    "ur",   // Urdu
    "uz",   // Uzbek
    "vi",   // Vietnamese
    "wa",   // Walloon
    "xh",   // Xhosa
    "yi",   // Yiddish
    "yo",   // Yoruba
    "zh",   // Chinese
    "zu",   // Zulu
};

}  // anonymous namespace

namespace chromeos {

TEST(LocaleChangeGuardTest, ShowNotificationLocaleChanged) {
  // "en" is used as "global default" in many places.
  EXPECT_TRUE(
      LocaleChangeGuard::ShouldShowLocaleChangeNotification("en", "it"));
  EXPECT_TRUE(
      LocaleChangeGuard::ShouldShowLocaleChangeNotification("it", "en"));

  // Between two latin locales.
  EXPECT_TRUE(
      LocaleChangeGuard::ShouldShowLocaleChangeNotification("fr", "it"));
  EXPECT_TRUE(
      LocaleChangeGuard::ShouldShowLocaleChangeNotification("it", "fr"));

  // en <-> non-latin locale
  EXPECT_TRUE(
      LocaleChangeGuard::ShouldShowLocaleChangeNotification("en", "zh"));
  EXPECT_TRUE(
      LocaleChangeGuard::ShouldShowLocaleChangeNotification("zh", "en"));

  // latin <-> non-latin locale
  EXPECT_TRUE(
      LocaleChangeGuard::ShouldShowLocaleChangeNotification("fr", "zh"));
  EXPECT_TRUE(
      LocaleChangeGuard::ShouldShowLocaleChangeNotification("zh", "fr"));

  // same language
  EXPECT_FALSE(
      LocaleChangeGuard::ShouldShowLocaleChangeNotification("en", "en"));
  EXPECT_FALSE(
      LocaleChangeGuard::ShouldShowLocaleChangeNotification("fr", "fr"));
  EXPECT_FALSE(
      LocaleChangeGuard::ShouldShowLocaleChangeNotification("zh", "zh"));
  EXPECT_FALSE(
      LocaleChangeGuard::ShouldShowLocaleChangeNotification("en", "en-US"));
  EXPECT_FALSE(
      LocaleChangeGuard::ShouldShowLocaleChangeNotification("en-GB", "en-US"));

  // Different regions within the same language
  EXPECT_FALSE(
      LocaleChangeGuard::ShouldShowLocaleChangeNotification("en", "en-au"));
  EXPECT_FALSE(
      LocaleChangeGuard::ShouldShowLocaleChangeNotification("en-AU", "en"));
  EXPECT_FALSE(
      LocaleChangeGuard::ShouldShowLocaleChangeNotification("en-AU", "en-GB"));

  EXPECT_TRUE(
      LocaleChangeGuard::ShouldShowLocaleChangeNotification("zh", "zh-CN"));
  EXPECT_TRUE(
      LocaleChangeGuard::ShouldShowLocaleChangeNotification("zh-CN", "zh-TW"));
  EXPECT_TRUE(
      LocaleChangeGuard::ShouldShowLocaleChangeNotification("es", "es-419"));
  EXPECT_TRUE(
      LocaleChangeGuard::ShouldShowLocaleChangeNotification("es", "es-ES"));
}

TEST(LocaleChangeGuardTest, ShowNotificationLocaleChangedList) {
  for (size_t i = 0; i < l10n_util::GetAcceptLanguageListSizeForTesting();
       ++i) {
    const char* const locale = l10n_util::GetAcceptLanguageListForTesting()[i];
    const char* const dash = strchr(locale, '-');
    const std::string language =
        (dash ? std::string(locale, dash - locale) : std::string(locale));

    const char* const* allowed_begin = kShowNotificationLanguages;
    const char* const* allowed_end =
        kShowNotificationLanguages + arraysize(kShowNotificationLanguages);
    const bool notification_allowed =
        (std::find(allowed_begin, allowed_end, language) != allowed_end);

    const char* const* skipped_begin =
        LocaleChangeGuard::GetSkipShowNotificationLanguagesForTesting();
    const char* const* skipped_end =
        skipped_begin +
        LocaleChangeGuard::GetSkipShowNotificationLanguagesSizeForTesting();
    const bool notification_skipped =
        (std::find(skipped_begin, skipped_end, language) != skipped_end);

    EXPECT_TRUE(notification_allowed ^ notification_skipped)
        << "Language '" << language << "' (from locale '" << locale
        << "') must be in exactly one list: either "
           "kSkipShowNotificationLanguages (found=" << notification_skipped
        << ") or kShowNotificationLanguages (found=" << notification_allowed
        << ").";
  }
}

}  // namespace chromeos
