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
goog.require('i18n.input.chrome.inputview.content.ContextlayoutUtil');
goog.require('i18n.input.chrome.inputview.content.compact.letter');
goog.require('i18n.input.chrome.inputview.content.compact.more');
goog.require('i18n.input.chrome.inputview.content.compact.numberpad');
goog.require('i18n.input.chrome.inputview.content.compact.symbol');
goog.require('i18n.input.chrome.inputview.content.compact.util');
goog.require('i18n.input.chrome.inputview.content.util');
goog.require('i18n.input.chrome.message.ContextType');

(function() {
  var ContextType = i18n.input.chrome.message.ContextType;
  var viewIdPrefix_ = '102kbd-k-';

  var keyCharacters = [
    ['\u0023', '\u007c', '\u005c', '\u0000'], // TLDE
    ['\u0031', '\u0021', '\u00b1', '\u0000'], // AE01
    ['\u0032', '\u0022', '\u0040', '\u0000'], // AE02
    ['\u0033', '\u002f', '\u00a3', '\u0000'], // AE03
    ['\u0034', '\u0024', '\u00a2', '\u0000'], // AE04
    ['\u0035', '\u0025', '\u00a4', '\u0000'], // AE05
    ['\u0036', '\u003f', '\u00ac', '\u0000'], // AE06
    ['\u0037', '\u0026', '\u00a6', '\u0000'], // AE07
    ['\u0038', '\u002a', '\u00b2', '\u0000'], // AE08
    ['\u0039', '\u0028', '\u00b3', '\u0000'], // AE09
    ['\u0030', '\u0029', '\u00bc', '\u0000'], // AE10
    ['\u002d', '\u005f', '\u00bd', '\u0000'], // AE11
    ['\u003d', '\u002b', '\u00be', '\u0000'], // AE12
    ['\u0071', '\u0051'], // AD01
    ['\u0077', '\u0057'], // AD02
    ['\u0065', '\u0045'], // AD03
    ['\u0072', '\u0052'], // AD04
    ['\u0074', '\u0054'], // AD05
    ['\u0079', '\u0059'], // AD06
    ['\u0075', '\u0055'], // AD07
    ['\u0069', '\u0049'], // AD08
    ['\u006f', '\u004f', '\u00a7', '\u0000'], // AD09
    ['\u0070', '\u0050', '\u00b6', '\u0000'], // AD10
    ['\u0302', '\u0302', '\u005b', '\u0000'], // AD11
    ['\u0327', '\u0308', '\u005d', '\u0000'], // AD12
    ['\u0061', '\u0041'], // AC01
    ['\u0073', '\u0053'], // AC02
    ['\u0064', '\u0044'], // AC03
    ['\u0066', '\u0046'], // AC04
    ['\u0067', '\u0047'], // AC05
    ['\u0068', '\u0048'], // AC06
    ['\u006a', '\u004a'], // AC07
    ['\u006b', '\u004b'], // AC08
    ['\u006c', '\u004c'], // AC09
    ['\u003b', '\u003a', '\u007e', '\u0000'], // AC10
    ['\u0300', '\u0300', '\u007b', '\u0000'], // AC11
    ['\u003c', '\u003e', '\u007d', '\u0000'], // BKSL
    ['\u00ab', '\u00bb', '\u00b0', '\u00a6'], // LSGT
    ['\u007a', '\u005a'], // AB01
    ['\u0078', '\u0058'], // AB02
    ['\u0063', '\u0043'], // AB03
    ['\u0076', '\u0056'], // AB04
    ['\u0062', '\u0042'], // AB05
    ['\u006e', '\u004e'], // AB06
    ['\u006d', '\u004d', '\u00b5', '\u0000'], // AB07
    ['\u002c', '\u0027', '\u00af', '\u0000'], // AB08
    ['\u002e', '\u002e', '\u00ad', '\u0000'], // AB09
    ['\u00e9', '\u00c9', '\u0301', '\u0000'], // AB10
    ['\u0020', '\u0020', '\u00a0', '\u0000'] // SPCE
  ];

  var keyCodes = [
    0xDE, // TLDE
    0x31, // AE01
    0x32, // AE02
    0x33, // AE03
    0x34, // AE04
    0x35, // AE05
    0x36, // AE06
    0x37, // AE07
    0x38, // AE08
    0x39, // AE09
    0x30, // AE10
    0xBD, // AE11
    0xBB, // AE12
    0x51, // AD01
    0x57, // AD02
    0x45, // AD03
    0x52, // AD04
    0x54, // AD05
    0x59, // AD06
    0x55, // AD07
    0x49, // AD08
    0x4F, // AD09
    0x50, // AD10
    0xDB, // AD11
    0xDD, // AD12
    0x41, // AC01
    0x53, // AC02
    0x44, // AC03
    0x46, // AC04
    0x47, // AC05
    0x48, // AC06
    0x4A, // AC07
    0x4B, // AC08
    0x4C, // AC09
    0xBA, // AC10
    0xC0, // AC11
    0xDC, // BKSL
    0xE2, // LTGT
    0x5A, // AB01
    0x58, // AB02
    0x43, // AB03
    0x56, // AB04
    0x42, // AB05
    0x4E, // AB06
    0x4D, // AB07
    0xBC, // AB08
    0xBE, // AB09
    0xBF, // AB10
    0x20  // SPCE
  ];

  var data = i18n.input.chrome.inputview.content.util.createData(
      keyCharacters, viewIdPrefix_, true, true, keyCodes, 'ca.compact.qwerty');
  data['id'] = 'ca';
  google.ime.chrome.inputview.onConfigLoaded(data);

  var keysetSpecNode =
      i18n.input.chrome.inputview.content.compact.util.CompactKeysetSpec;
  var letterKeysetSpec = {};
  letterKeysetSpec[keysetSpecNode.ID] = 'ca.compact.qwerty';
  letterKeysetSpec[keysetSpecNode.LAYOUT] = 'compactkbd-qwerty';
  letterKeysetSpec[keysetSpecNode.DATA] =
      i18n.input.chrome.inputview.content.compact.letter.keyQwertyCharacters();

  var symbolKeysetSpec = {};
  symbolKeysetSpec[keysetSpecNode.ID] = 'ca.compact.symbol';
  symbolKeysetSpec[keysetSpecNode.LAYOUT] = 'compactkbd-qwerty';
  symbolKeysetSpec[keysetSpecNode.DATA] = i18n.input.chrome.inputview.content.
      compact.symbol.keyNASymbolCharacters();

  var moreKeysetSpec = {};
  moreKeysetSpec[keysetSpecNode.ID] = 'ca.compact.more';
  moreKeysetSpec[keysetSpecNode.LAYOUT] = 'compactkbd-qwerty';
  moreKeysetSpec[keysetSpecNode.DATA] =
      i18n.input.chrome.inputview.content.compact.more.keyNAMoreCharacters();

  i18n.input.chrome.inputview.content.compact.util.generateCompactKeyboard(
      letterKeysetSpec, symbolKeysetSpec, moreKeysetSpec,
      google.ime.chrome.inputview.onConfigLoaded);

  var inputTypeToKeysetSpecMap = {};

  var numberKeysetSpec = {};
  numberKeysetSpec[keysetSpecNode.ID] = 'ca.compact.numberpad';
  numberKeysetSpec[keysetSpecNode.LAYOUT] = 'compactkbd-numberpad';
  numberKeysetSpec[keysetSpecNode.DATA] = i18n.input.chrome.inputview.content.
      compact.numberpad.keyNumberpadCharacters();
  inputTypeToKeysetSpecMap[ContextType.NUMBER] = numberKeysetSpec;

  var phoneKeysetSpec = {};
  phoneKeysetSpec[keysetSpecNode.ID] = 'ca.compact.phonepad';
  phoneKeysetSpec[keysetSpecNode.LAYOUT] = 'compactkbd-numberpad';
  phoneKeysetSpec[keysetSpecNode.DATA] = i18n.input.chrome.inputview.content.
      compact.numberpad.keyPhonepadCharacters();
  inputTypeToKeysetSpecMap[ContextType.PHONE] = phoneKeysetSpec;

  i18n.input.chrome.inputview.content.ContextlayoutUtil.generateContextLayouts(
      inputTypeToKeysetSpecMap, google.ime.chrome.inputview.onConfigLoaded);
}) ();
