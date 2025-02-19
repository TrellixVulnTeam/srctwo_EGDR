// Copyright 2014 The ChromeOS IME Authors. All Rights Reserved.
// limitations under the License.
// See the License for the specific language governing permissions and
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// distributed under the License is distributed on an "AS-IS" BASIS,
// Unless required by applicable law or agreed to in writing, software
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// You may obtain a copy of the License at
// you may not use this file except in compliance with the License.
// Licensed under the Apache License, Version 2.0 (the "License");
//
goog.provide('i18n.input.lang.InputToolCode');


/**
 * Enumeration for input tool codes
 *
 * @enum {string}
 */
i18n.input.lang.InputToolCode = {
  // For IME.
  INPUTMETHOD_ARRAY92_CHINESE_TRADITIONAL: 'zh-hant-t-i0-array-1992',
  INPUTMETHOD_CANGJIE82_CHINESE_SIMPLIFIED: 'zh-hans-t-i0-cangjie-1982',
  INPUTMETHOD_CANGJIE82_CHINESE_TRADITIONAL: 'zh-hant-t-i0-cangjie-1982',
  INPUTMETHOD_CANGJIE87_CHINESE_SIMPLIFIED: 'zh-hans-t-i0-cangjie-1987',
  INPUTMETHOD_CANGJIE87_CHINESE_TRADITIONAL: 'zh-hant-t-i0-cangjie-1987',
  INPUTMETHOD_CANGJIE87_QUICK_CHINESE_TRADITIONAL:
      'zh-hant-t-i0-cangjie-1987-x-m0-simplified',
  INPUTMETHOD_CANTONESE_TRADITIONAL: 'yue-hant-t-i0-und',
  INPUTMETHOD_DAYI88_CHINESE_TRADITIONAL: 'zh-hant-t-i0-dayi-1988',
  INPUTMETHOD_PINYIN_CHINESE_SIMPLIFIED: 'zh-t-i0-pinyin',
  INPUTMETHOD_PINYIN_CHINESE_TRADITIONAL: 'zh-hant-t-i0-pinyin',
  INPUTMETHOD_HANGUL_KOREAN: 'ko-t-i0-und',
  INPUTMETHOD_SHUANGPING_ABC: 'zh-t-i0-pinyin-x0-shuangpin-abc',
  INPUTMETHOD_SHUANGPING_FLYPY: 'zh-t-i0-pinyin-x0-shuangpin-flypy',
  INPUTMETHOD_SHUANGPING_JIAJIA: 'zh-t-i0-pinyin-x0-shuangpin-jiajia',
  INPUTMETHOD_SHUANGPING_MS: 'zh-t-i0-pinyin-x0-shuangpin-ms',
  INPUTMETHOD_SHUANGPING_ZIGUANG: 'zh-t-i0-pinyin-x0-shuangpin-ziguang',
  INPUTMETHOD_SHUANGPING_ZIRANMA: 'zh-t-i0-pinyin-x0-shuangpin-ziranma',
  INPUTMETHOD_TRANSLITERATION_AMHARIC: 'am-t-i0-und',
  INPUTMETHOD_TRANSLITERATION_ARABIC: 'ar-t-i0-und',
  INPUTMETHOD_TRANSLITERATION_BELARUSIAN: 'be-t-i0-und',
  INPUTMETHOD_TRANSLITERATION_BENGALI: 'bn-t-i0-und',
  INPUTMETHOD_TRANSLITERATION_BULGARIAN: 'bg-t-i0-und',
  INPUTMETHOD_TRANSLITERATION_DUTCH: 'nl-t-i0-und',
  INPUTMETHOD_TRANSLITERATION_ENGLISH: 'en-t-i0-und',
  INPUTMETHOD_TRANSLITERATION_FRENCH: 'fr-t-i0-und',
  INPUTMETHOD_TRANSLITERATION_GERMAN: 'de-t-i0-und',
  INPUTMETHOD_TRANSLITERATION_GREEK: 'el-t-i0-und',
  INPUTMETHOD_TRANSLITERATION_GUJARATI: 'gu-t-i0-und',
  INPUTMETHOD_TRANSLITERATION_HEBREW: 'he-t-i0-und',
  INPUTMETHOD_TRANSLITERATION_HINDI: 'hi-t-i0-und',
  INPUTMETHOD_TRANSLITERATION_HIRAGANA: 'ja-hira-t-i0-und',
  INPUTMETHOD_TRANSLITERATION_ITALIAN: 'it-t-i0-und',
  INPUTMETHOD_TRANSLITERATION_JAPANESE: 'ja-t-ja-hira-i0-und',
  INPUTMETHOD_TRANSLITERATION_KANNADA: 'kn-t-i0-und',
  INPUTMETHOD_TRANSLITERATION_MALAYALAM: 'ml-t-i0-und',
  INPUTMETHOD_TRANSLITERATION_MARATHI: 'mr-t-i0-und',
  INPUTMETHOD_TRANSLITERATION_NEPALI: 'ne-t-i0-und',
  INPUTMETHOD_TRANSLITERATION_ORIYA: 'or-t-i0-und',
  INPUTMETHOD_TRANSLITERATION_PERSIAN: 'fa-t-i0-und',
  INPUTMETHOD_TRANSLITERATION_POLISH: 'pl-t-i0-und',
  INPUTMETHOD_TRANSLITERATION_PORTUGUESE: 'pt-t-i0-und',
  INPUTMETHOD_TRANSLITERATION_PORTUGUESE_BRRAZIL: 'pt-br-t-i0-und',
  INPUTMETHOD_TRANSLITERATION_PORTUGUESE_PORTUGAL: 'pt-pt-t-i0-und',
  INPUTMETHOD_TRANSLITERATION_PUNJABI: 'pa-t-i0-und',
  INPUTMETHOD_TRANSLITERATION_RUSSIAN: 'ru-t-i0-und',
  INPUTMETHOD_TRANSLITERATION_SANSKRIT: 'sa-t-i0-und',
  INPUTMETHOD_TRANSLITERATION_SERBIAN: 'sr-t-i0-und',
  INPUTMETHOD_TRANSLITERATION_SINHALESE: 'si-t-i0-und',
  INPUTMETHOD_TRANSLITERATION_SPANISH: 'es-t-i0-und',
  INPUTMETHOD_TRANSLITERATION_TAMIL: 'ta-t-i0-und',
  INPUTMETHOD_TRANSLITERATION_TELUGU: 'te-t-i0-und',
  INPUTMETHOD_TRANSLITERATION_TIGRINYA: 'ti-t-i0-und',
  INPUTMETHOD_TRANSLITERATION_TURKISH: 'tr' + '-t-i0-und',
  INPUTMETHOD_TRANSLITERATION_UKRAINE: 'uk-t-i0-und',
  INPUTMETHOD_TRANSLITERATION_URDU: 'ur-t-i0-und',
  INPUTMETHOD_TRANSLITERATION_VIETNAMESE: 'vi-t-i0-und',
  INPUTMETHOD_WUBI_CHINESE_SIMPLIFIED: 'zh-t-i0-wubi-1986',
  INPUTMETHOD_ZHUYIN_CHINESE_TRADITIONAL: 'zh-hant-t-i0-und',
  INPUTMETHOD_ZHUYIN_CHINESE_TRADITIONAL_BOPOMOFO: 'zh-hant-t-i0-bopomofo',


  // For keyboard
  KEYBOARD_ALBANIAN: 'sq-t-k0-und',
  KEYBOARD_ARABIC: 'ar-t-k0-und',
  KEYBOARD_ARMENIAN_EASTERN: 'hy-hyr-t-k0-und',
  KEYBOARD_ARMENIAN_WESTERN: 'hy-hyt-t-k0-und',
  KEYBOARD_BASQUE: 'eu-t-k0-und',
  KEYBOARD_BELARUSIAN: 'be-t-k0-und',
  KEYBOARD_BENGALI_INSCRIPT: 'bn-t-k0-und',
  KEYBOARD_BENGALI_PHONETIC: 'bn-t-und-latn-k0-und',
  KEYBOARD_BOSNIAN: 'bs-t-k0-und',
  KEYBOARD_BRAZILIAN_PORTUGUESE: 'pt-br-t-k0-und',
  KEYBOARD_BULGARIAN: 'bg-t-k0-und',
  KEYBOARD_BULGARIAN_PHONETIC: 'bg-t-k0-qwerty',
  KEYBOARD_CATALAN: 'ca-t-k0-und',
  KEYBOARD_CHEROKEE: 'chr-t-k0-und',
  KEYBOARD_CHEROKEE_PHONETIC: 'chr-t-und-latn-k0-und',
  KEYBOARD_CROATIAN: 'hr-t-k0-und',
  KEYBOARD_CZECH: 'cs-t-k0-und',
  KEYBOARD_CZECH_QWERTZ: 'cs-t-k0-qwertz',
  KEYBOARD_DANISH: 'da-t-k0-und',
  KEYBOARD_DARI: 'prs-t-k0-und',
  KEYBOARD_DEVANAGARI_PHONETIC: 'hi-t-k0-qwerty',
  KEYBOARD_DUTCH: 'nl-t-k0-und',
  KEYBOARD_DUTCH_INTL: 'nl-t-k0-intl',
  KEYBOARD_DZONGKHA: 'dz-t-k0-und',
  KEYBOARD_ENGLISH: 'en-t-k0-und',
  KEYBOARD_ENGLISH_DVORAK: 'en-t-k0-dvorak',
  KEYBOARD_ESTONIAN: 'et-t-k0-und',
  KEYBOARD_ETHIOPIC: 'und-ethi-t-k0-und',
  KEYBOARD_TIGRINYA_ETHIOPIC: 'ti-ethi-t-k0-und',
  KEYBOARD_FINNISH: 'fi-t-k0-und',
  KEYBOARD_FRENCH: 'fr-t-k0-und',
  KEYBOARD_FRENCH_INTL: 'fr-t-k0-intl',
  KEYBOARD_GALICIAN: 'gl-t-k0-und',
  KEYBOARD_GEORGIAN_QWERTY: 'ka-t-k0-und',
  KEYBOARD_GEORGIAN_TYPEWRITER: 'ka-t-k0-legacy',
  KEYBOARD_GERMAN: 'de-t-k0-und',
  KEYBOARD_GERMAN_INTL: 'de-t-k0-intl',
  KEYBOARD_GREEK: 'el-t-k0-und',
  KEYBOARD_GUJARATI_INSCRIPT: 'gu-t-k0-und',
  KEYBOARD_GUJARATI_PHONETIC: 'gu-t-und-latn-k0-qwerty',
  KEYBOARD_GURMUKHI_INSCRIPT: 'pa-guru-t-k0-und',
  KEYBOARD_GURMUKHI_PHONETIC: 'pa-guru-t-und-latn-k0-und',
  KEYBOARD_HAITIAN: 'ht-t-k0-und',
  KEYBOARD_HEBREW: 'he-t-k0-und',
  KEYBOARD_HINDI: 'hi-t-k0-und',
  KEYBOARD_HUNGARIAN_101: 'hu-t-k0-101key',
  KEYBOARD_ICELANDIC: 'is-t-k0-und',
  KEYBOARD_INDONESIAN: 'id-t-k0-und',
  KEYBOARD_INUKTITUT_NUNAVIK: 'iu-t-k0-nunavik',
  KEYBOARD_INUKTITUT_NUNAVUT: 'iu-t-k0-nunavut',
  KEYBOARD_IRISH: 'ga-t-k0-und',
  KEYBOARD_ITALIAN: 'it-t-k0-und',
  KEYBOARD_ITALIAN_INTL: 'it-t-k0-intl',
  KEYBOARD_JAVANESE: 'jw-t-k0-und',
  KEYBOARD_KANNADA_INSCRIPT: 'kn-t-k0-und',
  KEYBOARD_KANNADA_PHONETIC: 'kn-t-und-latn-k0-und',
  KEYBOARD_KAZAKH: 'kk-t-k0-und',
  KEYBOARD_KHMER: 'km-t-k0-und',
  KEYBOARD_KOREAN: 'ko-t-k0-und',
  KEYBOARD_KYRGYZ: 'ky-cyrl-t-k0-und',
  KEYBOARD_LAO: 'lo-t-k0-und',
  KEYBOARD_LATVIAN: 'lv-t-k0-und',
  KEYBOARD_LISU: 'lis-t-k0-und',
  KEYBOARD_LITHUANIAN: 'lt-t-k0-und',
  KEYBOARD_MACEDONIAN: 'mk-t-k0-und',
  KEYBOARD_MALAY: 'ms-t-k0-und',
  KEYBOARD_MALAYALAM_INSCRIPT: 'ml-t-k0-und',
  KEYBOARD_MALAYALAM_PHONETIC: 'ml-t-und-latn-k0-und',
  KEYBOARD_MALTESE: 'mt-t-k0-und',
  KEYBOARD_MAORI: 'mi-t-k0-und',
  KEYBOARD_MARATHI: 'mr-t-k0-und',
  KEYBOARD_MARATHI_INSCRIPT: 'mr-t-k0-devanaga',
  KEYBOARD_MONGOLIAN_CYRILLIC: 'mn-cyrl-t-k0-und',
  KEYBOARD_MONTENEGRIN: 'srp-t-k0-und',
  KEYBOARD_MYANMAR: 'my-t-k0-und',
  KEYBOARD_MYANMAR_MYANSAN: 'my-t-k0-myansan',
  KEYBOARD_NAVAJO: 'nv-t-k0-und',
  KEYBOARD_NAVAJO_STANDARD: 'nv-t-k0-std',
  KEYBOARD_NEPALI_INSCRIPT: 'ne-t-k0-und',
  KEYBOARD_NEPALI_PHONETIC: 'ne-t-und-latn-k0-und',
  KEYBOARD_NORWEGIAN: 'no-t-k0-und',
  KEYBOARD_ORIYA_INSCRIPT: 'or-t-k0-und',
  KEYBOARD_ORIYA_PHONETIC: 'or-t-und-latn-k0-und',
  KEYBOARD_PAN_AFRICA_LATIN: 'latn-002-t-k0-und',
  KEYBOARD_PASHTO: 'ps-t-k0-und',
  KEYBOARD_PERSIAN: 'fa-t-k0-und',
  KEYBOARD_POLISH: 'pl-t-k0-und',
  KEYBOARD_PORTUGUESE: 'pt-pt-t-k0-und',
  KEYBOARD_PORTUGUESE_BRAZIL_INTL: 'pt-br-t-k0-intl',
  KEYBOARD_PORTUGUESE_PORTUGAL_INTL: 'pt-pt-t-k0-intl',
  KEYBOARD_ROMANI: 'rom-t-k0-und',
  KEYBOARD_ROMANIAN: 'ro-t-k0-und',
  KEYBOARD_ROMANIAN_SR13392_PRIMARY: 'ro-t-k0-legacy',
  KEYBOARD_ROMANIAN_SR13392_SECONDARY: 'ro-t-k0-extended',
  KEYBOARD_RUSSIAN: 'ru-t-k0-und',
  KEYBOARD_RUSSIAN_PHONETIC: 'ru-t-k0-qwerty',
  KEYBOARD_RUSSIAN_PHONETIC_AATSEEL: 'ru-t-k0-aatseel',
  KEYBOARD_RUSSIAN_PHONETIC_YAZHERT: 'ru-t-k0-yazhert',
  KEYBOARD_SANSKRIT_PHONETIC: 'sa-t-und-latn-k0-und',
  KEYBOARD_SANSKRIT_INSCRIPT: 'sa-t-k0-devanaga',
  KEYBOARD_SENECA: 'see-t-k0-und',
  KEYBOARD_SERBIAN_CYRILLIC: 'sr-cyrl-t-k0-und',
  KEYBOARD_SERBIAN_LATIN: 'sr-latn-t-k0-und',
  KEYBOARD_SINDHI: 'sd-t-k0-und',
  KEYBOARD_SINHALA: 'si-t-k0-und',
  KEYBOARD_SLOVAK: 'sk-t-k0-und',
  KEYBOARD_SLOVAK_QWERTY: 'sk-t-k0-qwerty',
  KEYBOARD_SLOVENIAN: 'sl-t-k0-und',
  KEYBOARD_SORANI_KURDISH_AR: 'ckb-t-k0-ar',  // Arabic-based
  KEYBOARD_SORANI_KURDISH_EN: 'ckb-t-k0-en',  // English-based
  KEYBOARD_SOUTHERN_UZBEK: 'uzs-t-k0-und',
  KEYBOARD_SPANISH: 'es-t-k0-und',
  KEYBOARD_SPANISH_INTL: 'es-t-k0-intl',
  KEYBOARD_SWAHILI: 'sw-t-k0-und',
  KEYBOARD_SWEDISH: 'sv-t-k0-und',
  KEYBOARD_SWISS_GERMAN: 'de-ch-t-k0-und',
  KEYBOARD_TAGALOG: 'tl-t-k0-und',
  KEYBOARD_TAJIK: 'tg-t-k0-und',
  KEYBOARD_TAMIL_99: 'ta-t-k0-ta99',
  KEYBOARD_TAMIL_INSCRIPT: 'ta-t-k0-und',
  KEYBOARD_TAMIL_ITRANS: 'ta-t-k0-itrans',
  KEYBOARD_TAMIL_PHONETIC: 'ta-t-und-latn-k0-und',
  KEYBOARD_TAMIL_TYPEWRITER: 'ta-t-k0-typewriter',
  KEYBOARD_TATAR: 'tt-t-k0-und',
  KEYBOARD_TELUGU_INSCRIPT: 'te-t-k0-und',
  KEYBOARD_TELUGU_PHONETIC: 'te-t-und-latn-k0-und',
  KEYBOARD_THAI: 'th-t-k0-und',
  KEYBOARD_THAI_PATTAJOTI: 'th-t-k0-pattajoti',
  KEYBOARD_THAI_TIS: 'th-t-k0-tis',
  KEYBOARD_TIGRINYA: 'ti-t-k0-und',
  // Gmail doesn't allow any string prefix is 'tr'.
  // String with 'tr' prefix will obfuscated in css compiling pharse.
  KEYBOARD_TURKISH_F: 'tr' + '-t-k0-legacy',
  KEYBOARD_TURKISH_Q: 'tr' + '-t-k0-und',
  KEYBOARD_UIGHUR: 'ug-t-k0-und',
  KEYBOARD_UKRAINIAN_101: 'uk-t-k0-101key',
  KEYBOARD_URDU: 'ur-t-k0-und',
  KEYBOARD_US_INTERNATIONAL: 'en-us-t-k0-intl',
  KEYBOARD_UZBEK_CYRILLIC_PHONETIC: 'uz-cyrl-t-k0-und',
  KEYBOARD_UZBEK_CYRILLIC_TYPEWRITTER: 'uz-cyrl-t-k0-legacy',
  KEYBOARD_UZBEK_LATIN: 'uz-latn-t-k0-und',
  KEYBOARD_VIETNAMESE_TCVN: 'vi-t-k0-und',
  KEYBOARD_VIETNAMESE_TELEX: 'vi-t-k0-legacy',
  KEYBOARD_VIETNAMESE_VIQR: 'vi-t-k0-viqr',
  KEYBOARD_VIETNAMESE_VNI: 'vi-t-k0-vni',
  KEYBOARD_WELSH: 'cy-t-k0-und',
  KEYBOARD_YIDDISH: 'yi-t-k0-und',

  // Accents
  KEYBOARD_GENMAN_ACCENTS: 'de-t-k0-accents',
  KEYBOARD_SPANISH_ACCENTS: 'es-t-k0-accents',
  KEYBOARD_FRENCH_ACCENTS: 'fr-t-k0-accents',
  KEYBOARD_ITALIAN_ACCENTS: 'it-t-k0-accents',
  EYBOARD_DUTCH_ACCENTS: 'nl-t-k0-accents',
  KEYBOARD_POLISH_ACCENTS: 'pl-t-k0-accents',
  KEYBOARD_PORTUGUESE_BRAZIL_ACCENTS: 'pt-br-t-k0-accents',
  KEYBOARD_PORTUGUESE_PORTUGAL_ACCENTS: 'pt-pt-t-k0-accents',
  // Gmail doesn't allow any string prefix is 'tr'.
  // String with 'tr' prefix will obfuscated in css compiling pharse.
  KEYBOARD_TURKISH_F_ACCENTS: 'tr' + '-t-k0-f-accents',
  KEYBOARD_TURKISH_Q_ACCENTS: 'tr' + '-t-k0-q-accents',
  KEYBOARD_CORSICAN_ACCENTS: 'co-t-k0-accents',
  KEYBOARD_HAWAIIAN_ACCENTS: 'haw-t-k0-accents',
  KEYBOARD_SAMOAN_ACCENTS: 'sm-t-k0-accents',
  KEYBOARD_SCOTS_GAELIC_ACCENTS: 'gd-t-k0-accents',
  KEYBOARD_WEST_FRISIAN_ACCENTS: 'fy-t-k0-accents',
  KEYBORAD_LUXEMBOURGISH_ACCENTS: 'lb-t-k0-accents',

  // Handwriting codes
  HANDWRIT_AFRIKAANS: 'af-t-i0-handwrit',
  HANDWRIT_ALBANIAN: 'sq-t-i0-handwrit',
  HANDWRIT_ARABIC: 'ar-t-i0-handwrit',
  HANDWRIT_AZERBAIJANI: 'az-t-i0-handwrit',
  HANDWRIT_BASQUE: 'eu-t-i0-handwrit',
  HANDWRIT_BELARUSIAN: 'be-t-i0-handwrit',
  HANDWRIT_BENGALI: 'bn-t-i0-handwrit',
  HANDWRIT_BOSNIAN: 'bs-t-i0-handwrit',
  HANDWRIT_BULGARIAN: 'bg-t-i0-handwrit',
  HANDWRIT_BURMESE: 'my-t-i0-handwrit',
  HANDWRIT_CANTONESE: 'zh-yue-t-i0-handwrit',
  HANDWRIT_CATALAN: 'ca-t-i0-handwrit',
  HANDWRIT_CEBUANO: 'ceb-t-i0-handwrit',
  HANDWRIT_CHINESE: 'zh-t-i0-handwrit',
  HANDWRIT_CHINESE_SIMPLIFIED: 'zh-hans-t-i0-handwrit',
  HANDWRIT_CHINESE_TRADITIONAL: 'zh-hant-t-i0-handwrit',
  HANDWRIT_CORSICAN: 'co-t-i0-handwrit',
  HANDWRIT_CROATIAN: 'hr-t-i0-handwrit',
  HANDWRIT_CZECH: 'cs-t-i0-handwrit',
  HANDWRIT_DANISH: 'da-t-i0-handwrit',
  HANDWRIT_DUTCH: 'nl-t-i0-handwrit',
  HANDWRIT_ENGLISH: 'en-t-i0-handwrit',
  HANDWRIT_ESPERANTO: 'eo-t-i0-handwrit',
  HANDWRIT_ESTONIAN: 'et-t-i0-handwrit',
  HANDWRIT_FILIPINO: 'fil-t-i0-handwrit',
  HANDWRIT_FINNISH: 'fi-t-i0-handwrit',
  HANDWRIT_FRENCH: 'fr-t-i0-handwrit',
  HANDWRIT_GALICIAN: 'gl-t-i0-handwrit',
  HANDWRIT_GERMAN: 'de-t-i0-handwrit',
  HANDWRIT_GREEK: 'el-t-i0-handwrit',
  HANDWRIT_GUJARATI: 'gu-t-i0-handwrit',
  HANDWRIT_HAITIAN: 'ht-t-i0-handwrit',
  HANDWRIT_HAWAIIAN: 'haw-t-i0-handwrit',
  HANDWRIT_HEBREW: 'he-t-i0-handwrit',
  HANDWRIT_HINDI: 'hi-t-i0-handwrit',
  HANDWRIT_HMONG: 'hmn-t-i0-handwrit',
  HANDWRIT_HUNGARIAN: 'hu-t-i0-handwrit',
  HANDWRIT_ICELANDIC: 'is-t-i0-handwrit',
  HANDWRIT_INDONESIAN: 'id-t-i0-handwrit',
  HANDWRIT_IRISH: 'ga-t-i0-handwrit',
  HANDWRIT_ITALIAN: 'it-t-i0-handwrit',
  HANDWRIT_JAPANESE: 'ja-t-i0-handwrit',
  HANDWRIT_JAVANESE: 'jv-t-i0-handwrit',
  HANDWRIT_KANNADA: 'kn-t-i0-handwrit',
  HANDWRIT_KHMER: 'km-t-i0-handwrit',
  HANDWRIT_KOREAN: 'ko-t-i0-handwrit',
  HANDWRIT_KURDISH: 'ku-t-i0-handwrit',
  HANDWRIT_KYRGYZ: 'ky-t-i0-handwrit',
  HANDWRIT_LAO: 'lo-t-i0-handwrit',
  HANDWRIT_LATIN: 'la-t-i0-handwrit',
  HANDWRIT_LATVIAN: 'lv-t-i0-handwrit',
  HANDWRIT_LITHUANIAN: 'lt-t-i0-handwrit',
  HANDWRIT_LUXEMBOURGISH: 'lb-t-i0-handwrit',
  HANDWRIT_MACEDONIAN: 'mk-t-i0-handwrit',
  HANDWRIT_MALAGASY: 'mg-t-i0-handwrit',
  HANDWRIT_MALAY: 'ms-t-i0-handwrit',
  HANDWRIT_MALAYALAM: 'ml-t-i0-handwrit',
  HANDWRIT_MALTESE: 'mt-t-i0-handwrit',
  HANDWRIT_MAORI: 'mi-t-i0-handwrit',
  HANDWRIT_MARATHI: 'mr-t-i0-handwrit',
  HANDWRIT_MONGOLIAN: 'mn-t-i0-handwrit',
  HANDWRIT_MULTIPLE_LANGUAGES: 'mul-t-i0-handwrit',
  HANDWRIT_NEPALI: 'ne-t-i0-handwrit',
  HANDWRIT_NORWEGIAN: 'no-t-i0-handwrit',
  HANDWRIT_NORWEGIAN_BOKMAL: 'nb-t-i0-handwrit',
  HANDWRIT_NORWEGIAN_NYNORSK: 'nn-t-i0-handwrit',
  HANDWRIT_NYANJA: 'ny-t-i0-handwrit',
  HANDWRIT_ORIYA: 'or-t-i0-handwrit',
  HANDWRIT_PERSIAN: 'fa-t-i0-handwrit',
  HANDWRIT_POLISH: 'pl-t-i0-handwrit',
  HANDWRIT_PORTUGUESE: 'pt-t-i0-handwrit',
  HANDWRIT_PORTUGUESE_BRAZIL: 'pt-br-t-i0-handwrit',
  HANDWRIT_PORTUGUESE_PORTUGAL: 'pt-pt-t-i0-handwrit',
  HANDWRIT_PUNJABI: 'pa-t-i0-handwrit',
  HANDWRIT_ROMANIAN: 'ro-t-i0-handwrit',
  HANDWRIT_RUSSIAN: 'ru-t-i0-handwrit',
  HANDWRIT_SAMOAN: 'sm-t-i0-handwrit',
  HANDWRIT_SCOTTISH_GAELIC: 'gd-t-i0-handwrit',
  HANDWRIT_SERBIAN: 'sr-t-i0-handwrit',
  HANDWRIT_SHONA: 'sn-t-i0-handwrit',
  HANDWRIT_SINHALA: 'si-t-i0-handwrit',
  HANDWRIT_SLOVAK: 'sk-t-i0-handwrit',
  HANDWRIT_SLOVENIAN: 'sl-t-i0-handwrit',
  HANDWRIT_SOMALI: 'so-t-i0-handwrit',
  HANDWRIT_SPANISH: 'es-t-i0-handwrit',
  HANDWRIT_SUNDANESE: 'su-t-i0-handwrit',
  HANDWRIT_SWAHILI: 'sw-t-i0-handwrit',
  HANDWRIT_SWEDISH: 'sv-t-i0-handwrit',
  HANDWRIT_TAMIL: 'ta-t-i0-handwrit',
  HANDWRIT_TELUGU: 'te-t-i0-handwrit',
  HANDWRIT_THAI: 'th-t-i0-handwrit',
  HANDWRIT_TURKISH: 'tr' + '-t-i0-handwrit',
  HANDWRIT_UKRAINIAN: 'uk-t-i0-handwrit',
  HANDWRIT_URDU: 'ur-t-i0-handwrit',
  HANDWRIT_VIETNAMESE: 'vi-t-i0-handwrit',
  HANDWRIT_WELSH: 'cy-t-i0-handwrit',
  HANDWRIT_WESTERN_FRISIAN: 'fy-t-i0-handwrit',
  HANDWRIT_XHOSA: 'xh-t-i0-handwrit',
  HANDWRIT_ZULU: 'zu-t-i0-handwrit',
  HANDWRIT_UNIVERSAL: 'und-t-i0-handwrit',

  // Voice
  VOICE_ENGLISH: 'en-t-i0-voice',
  VOICE_CHINESE_SIMPLIFIED: 'zh-hans-t-i0-voice',
  VOICE_CHINESE_TRADITIONAL: 'zh-hant-t-i0-voice',

  // XKB
  XKB_AM_PHONETIC_ARM: 'xkb:am:phonetic:arm',
  XKB_BE_FRA: 'xkb:be::fra',
  XKB_BE_GER: 'xkb:be::ger',
  XKB_BE_NLD: 'xkb:be::nld',
  XKB_BG_BUL: 'xkb:bg::bul',
  XKB_BG_PHONETIC_BUL: 'xkb:bg:phonetic:bul',
  XKB_BR_POR: 'xkb:br::por',
  XKB_BY_BEL: 'xkb:by::bel',
  XKB_CA_FRA: 'xkb:ca::fra',
  XKB_CA_ENG_ENG: 'xkb:ca:eng:eng',
  XKB_CA_MULTIX_FRA: 'xkb:ca:multix:fra',
  XKB_CH_GER: 'xkb:ch::ger',
  XKB_CH_FR_FRA: 'xkb:ch:fr:fra',
  XKB_CZ_CZE: 'xkb:cz::cze',
  XKB_CZ_QWERTY_CZE: 'xkb:cz:qwerty:cze',
  XKB_DE_GER: 'xkb:de::ger',
  XKB_DE_NEO_GER: 'xkb:de:neo:ger',
  XKB_DK_DAN: 'xkb:dk::dan',
  XKB_EE_EST: 'xkb:ee::est',
  XKB_ES_SPA: 'xkb:es::spa',
  XKB_ES_CAT_CAT: 'xkb:es:cat:cat',
  XKB_FO_FAO: 'xkb:fo::fao',
  XKB_FI_FIN: 'xkb:fi::fin',
  XKB_FR_BEPO_FRA: 'xkb:fr:bepo:fra',
  XKB_FR_FRA: 'xkb:fr::fra',
  XKB_GB_DVORAK_ENG: 'xkb:gb:dvorak:eng',
  XKB_GB_EXTD_ENG: 'xkb:gb:extd:eng',
  XKB_GE_GEO: 'xkb:ge::geo',
  XKB_GR_GRE: 'xkb:gr::gre',
  XKB_HR_SCR: 'xkb:hr::scr',
  XKB_HU_HUN: 'xkb:hu::hun',
  XKB_HU_QWERTY_HUN: 'xkb:hu:qwerty:hun',
  XKB_IE_GA: 'xkb:ie::ga',
  XKB_IL_HEB: 'xkb:il::heb',
  XKB_IS_ICE: 'xkb:is::ice',
  XKB_IT_ITA: 'xkb:it::ita',
  XKB_JP_JPN: 'xkb:jp::jpn',
  XKB_KZ_KAZ: 'xkb:kz::kaz',
  XKB_LATAM_SPA: 'xkb:latam::spa',
  XKB_LT_LIT: 'xkb:lt::lit',
  XKB_LV_APOSTROPHE_LAV: 'xkb:lv:apostrophe:lav',
  XKB_MN_MON: 'xkb:mn::mon',
  XKB_MK_MKD: 'xkb:mk::mkd',
  XKB_MT_MLT: 'xkb:mt::mlt',
  XKB_NO_NOB: 'xkb:no::nob',
  XKB_PL_POL: 'xkb:pl::pol',
  XKB_PT_POR: 'xkb:pt::por',
  XKB_RO_RUM: 'xkb:ro::rum',
  XKB_RO_STD_RUM: 'xkb:ro:std:rum',
  XKB_RS_SRP: 'xkb:rs::srp',
  XKB_RU_RUS: 'xkb:ru::rus',
  XKB_RU_PHONETIC_RUS: 'xkb:ru:phonetic:rus',
  XKB_SE_SWE: 'xkb:se::swe',
  XKB_SI_SLV: 'xkb:si::slv',
  XKB_SK_SLO: 'xkb:sk::slo',
  XKB_TR_TUR: 'xkb:tr::tur',
  XKB_TR_F_TUR: 'xkb:tr:f:tur',
  XKB_UA_UKR: 'xkb:ua::ukr',
  XKB_US_ENG: 'xkb:us::eng',
  XKB_US_FIL: 'xkb:us::fil',
  XKB_US_IND: 'xkb:us::ind',
  XKB_US_MSA: 'xkb:us::msa',
  XKB_US_ALTGR_INTL_ENG: 'xkb:us:altgr-intl:eng',
  XKB_US_COLEMAK_ENG: 'xkb:us:colemak:eng',
  XKB_US_DVORAK_ENG: 'xkb:us:dvorak:eng',
  XKB_US_DVP_ENG: 'xkb:us:dvp:eng',
  XKB_US_INTL_ENG: 'xkb:us:intl:eng',
  XKB_US_INTL_NLD: 'xkb:us:intl:nld',
  XKB_US_INTL_POR: 'xkb:us:intl:por'
};
