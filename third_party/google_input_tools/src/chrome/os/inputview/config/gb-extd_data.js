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
    ['\u0300', '\u00ac', '\u00a6', '\u00ac'], // TLDE
    ['\u0031', '\u0021', '\u00b9', '\u00a1'], // AE01
    ['\u0032', '\u0022', '\u0308', '\u00bd'], // AE02
    ['\u0033', '\u00a3', '\u00b3', '\u2153'], // AE03
    ['\u0034', '\u0024', '\u20ac', '\u00bc'], // AE04
    ['\u0035', '\u0025', '\u00bd', '\u215c'], // AE05
    ['\u0036', '\u005e', '\u0302', '\u215d'], // AE06
    ['\u0037', '\u0026', '\u007b', '\u215e'], // AE07
    ['\u0038', '\u002a', '\u005b', '\u2122'], // AE08
    ['\u0039', '\u0028', '\u005d', '\u00b1'], // AE09
    ['\u0030', '\u0029', '\u007d', '\u00b0'], // AE10
    ['\u002d', '\u005f', '\u005c', '\u00bf'], // AE11
    ['\u003d', '\u002b', '\u0327', '\u0328'], // AE12
    ['\u0071', '\u0051', '\u0040', '\u03a9'], // AD01
    ['\u0077', '\u0057', '\u1e83', '\u1e82'], // AD02
    ['\u0065', '\u0045', '\u00e9', '\u00c9'], // AD03
    ['\u0072', '\u0052', '\u00b6', '\u00ae'], // AD04
    ['\u0074', '\u0054', '\u0167', '\u0166'], // AD05
    ['\u0079', '\u0059', '\u00fd', '\u00dd'], // AD06
    ['\u0075', '\u0055', '\u00fa', '\u00da'], // AD07
    ['\u0069', '\u0049', '\u00ed', '\u00cd'], // AD08
    ['\u006f', '\u004f', '\u00f3', '\u00d3'], // AD09
    ['\u0070', '\u0050', '\u00fe', '\u00de'], // AD10
    ['\u005b', '\u007b', '\u0308', '\u030a'], // AD11
    ['\u005d', '\u007d', '\u0303', '\u0304'], // AD12
    ['\u0061', '\u0041', '\u00e1', '\u00c1'], // AC01
    ['\u0073', '\u0053', '\u00df', '\u00a7'], // AC02
    ['\u0064', '\u0044', '\u00f0', '\u00d0'], // AC03
    ['\u0066', '\u0046', '\u0111', '\u00aa'], // AC04
    ['\u0067', '\u0047', '\u014b', '\u014a'], // AC05
    ['\u0068', '\u0048', '\u0127', '\u0126'], // AC06
    ['\u006a', '\u004a', '\u0309', '\u031b'], // AC07
    ['\u006b', '\u004b', '\u0138', '\u0026'], // AC08
    ['\u006c', '\u004c', '\u0142', '\u0141'], // AC09
    ['\u003b', '\u003a', '\u0301', '\u030b'], // AC10
    ['\u0027', '\u0040', '\u0301', '\u030c'], // AC11
    ['\u0023', '\u007e', '\u0303', '\u0306'], // BKSL
    ['\u005c', '\u007c', '\u007c', '\u00a6'], // LSGT
    ['\u007a', '\u005a', '\u00ab', '\u003c'], // AB01
    ['\u0078', '\u0058', '\u00bb', '\u003e'], // AB02
    ['\u0063', '\u0043', '\u00e7', '\u00c7'], // AB03
    ['\u0076', '\u0056', '\u201c', '\u2018'], // AB04
    ['\u0062', '\u0042', '\u201d', '\u2019'], // AB05
    ['\u006e', '\u004e'], // AB06
    ['\u006d', '\u004d', '\u00b5', '\u00ba'], // AB07
    ['\u002c', '\u003c', '\u0000', '\u00d7'], // AB08
    ['\u002e', '\u003e', '\u00b7', '\u00f7'], // AB09
    ['\u002f', '\u003f', '\u0323', '\u0307'], // AB10
    ['\u0020', '\u0020'] // SPCE
  ];

  var keyCodes = [
    0xDF, // TLDE
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
    0xDE, // BKSL
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
      keyCharacters, viewIdPrefix_, true, true, keyCodes,
      'gb-extd.compact.qwerty');
  data['id'] = 'gb-extd';
  google.ime.chrome.inputview.onConfigLoaded(data);

  var keysetSpecNode =
      i18n.input.chrome.inputview.content.compact.util.CompactKeysetSpec;
  var letterKeysetSpec = {};
  letterKeysetSpec[keysetSpecNode.ID] = 'gb-extd.compact.qwerty';
  letterKeysetSpec[keysetSpecNode.LAYOUT] = 'compactkbd-qwerty';
  letterKeysetSpec[keysetSpecNode.DATA] =
      i18n.input.chrome.inputview.content.compact.letter.keyQwertyCharacters();

  var symbolKeysetSpec = {};
  symbolKeysetSpec[keysetSpecNode.ID] = 'gb-extd.compact.symbol';
  symbolKeysetSpec[keysetSpecNode.LAYOUT] = 'compactkbd-qwerty';
  symbolKeysetSpec[keysetSpecNode.DATA] = i18n.input.chrome.inputview.content.
      compact.symbol.keyUKSymbolCharacters();

  var moreKeysetSpec = {};
  moreKeysetSpec[keysetSpecNode.ID] = 'gb-extd.compact.more';
  moreKeysetSpec[keysetSpecNode.LAYOUT] = 'compactkbd-qwerty';
  moreKeysetSpec[keysetSpecNode.DATA] =
      i18n.input.chrome.inputview.content.compact.more.keyUKMoreCharacters();

  i18n.input.chrome.inputview.content.compact.util.generateCompactKeyboard(
      letterKeysetSpec, symbolKeysetSpec, moreKeysetSpec,
      google.ime.chrome.inputview.onConfigLoaded);

  var inputTypeToKeysetSpecMap = {};

  var numberKeysetSpec = {};
  numberKeysetSpec[keysetSpecNode.ID] = 'gb-extd.compact.numberpad';
  numberKeysetSpec[keysetSpecNode.LAYOUT] = 'compactkbd-numberpad';
  numberKeysetSpec[keysetSpecNode.DATA] = i18n.input.chrome.inputview.content.
      compact.numberpad.keyNumberpadCharacters();
  inputTypeToKeysetSpecMap[ContextType.NUMBER] = numberKeysetSpec;

  var phoneKeysetSpec = {};
  phoneKeysetSpec[keysetSpecNode.ID] = 'gb-extd.compact.phonepad';
  phoneKeysetSpec[keysetSpecNode.LAYOUT] = 'compactkbd-numberpad';
  phoneKeysetSpec[keysetSpecNode.DATA] = i18n.input.chrome.inputview.content.
      compact.numberpad.keyPhonepadCharacters();
  inputTypeToKeysetSpecMap[ContextType.PHONE] = phoneKeysetSpec;

  i18n.input.chrome.inputview.content.ContextlayoutUtil.generateContextLayouts(
      inputTypeToKeysetSpecMap, google.ime.chrome.inputview.onConfigLoaded);
}) ();
