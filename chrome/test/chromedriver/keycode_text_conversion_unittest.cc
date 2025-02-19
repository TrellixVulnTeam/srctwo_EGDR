// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "base/strings/string16.h"
#include "base/strings/utf_string_conversions.h"
#include "build/build_config.h"
#include "chrome/test/chromedriver/chrome/ui_events.h"
#include "chrome/test/chromedriver/keycode_text_conversion.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/events/keycodes/keyboard_codes.h"
#include "ui/events/test/keyboard_layout.h"

namespace {

void CheckCharToKeyCode16(base::char16 character, ui::KeyboardCode key_code,
                          int modifiers) {
  ui::KeyboardCode actual_key_code = ui::VKEY_UNKNOWN;
  int actual_modifiers = 0;
  std::string error_msg;
  EXPECT_TRUE(ConvertCharToKeyCode(
      character, &actual_key_code, &actual_modifiers, &error_msg));
  EXPECT_EQ(key_code, actual_key_code) << "Char: " << character;
  EXPECT_EQ(modifiers, actual_modifiers) << "Char: " << character;
}

void CheckCharToKeyCode(char character, ui::KeyboardCode key_code,
                        int modifiers) {
  CheckCharToKeyCode16(base::UTF8ToUTF16(std::string(1, character))[0],
                       key_code, modifiers);
}

void CheckCharToKeyCode(wchar_t character, ui::KeyboardCode key_code,
                        int modifiers) {
  CheckCharToKeyCode16(base::WideToUTF16(std::wstring(1, character))[0],
                       key_code, modifiers);
}

void CheckCantConvertChar(wchar_t character) {
  std::wstring character_string;
  character_string.push_back(character);
  base::char16 character_utf16 = base::WideToUTF16(character_string)[0];
  ui::KeyboardCode actual_key_code = ui::VKEY_UNKNOWN;
  int actual_modifiers = 0;
  std::string error_msg;
  EXPECT_FALSE(ConvertCharToKeyCode(
      character_utf16, &actual_key_code, &actual_modifiers, &error_msg));
}

std::string ConvertKeyCodeToTextNoError(ui::KeyboardCode key_code,
                                        int modifiers) {
  std::string error_msg;
  std::string text;
  EXPECT_TRUE(ConvertKeyCodeToText(key_code, modifiers, &text, &error_msg));
  return text;
}

}  // namespace

#if defined(OS_LINUX) && !defined(OS_CHROMEOS)
// Fails on bots: crbug.com/174962
#define MAYBE_KeyCodeToText DISABLED_KeyCodeToText
#else
#define MAYBE_KeyCodeToText KeyCodeToText
#endif

TEST(KeycodeTextConversionTest, MAYBE_KeyCodeToText) {
  EXPECT_EQ("a", ConvertKeyCodeToTextNoError(ui::VKEY_A, 0));
  EXPECT_EQ("A",
      ConvertKeyCodeToTextNoError(ui::VKEY_A, kShiftKeyModifierMask));

  EXPECT_EQ("1", ConvertKeyCodeToTextNoError(ui::VKEY_1, 0));
  EXPECT_EQ("!",
      ConvertKeyCodeToTextNoError(ui::VKEY_1, kShiftKeyModifierMask));

  EXPECT_EQ(",", ConvertKeyCodeToTextNoError(ui::VKEY_OEM_COMMA, 0));
  EXPECT_EQ("<", ConvertKeyCodeToTextNoError(
      ui::VKEY_OEM_COMMA, kShiftKeyModifierMask));

  EXPECT_EQ("", ConvertKeyCodeToTextNoError(ui::VKEY_F1, 0));
  EXPECT_EQ("",
      ConvertKeyCodeToTextNoError(ui::VKEY_F1, kShiftKeyModifierMask));

  EXPECT_EQ("/", ConvertKeyCodeToTextNoError(ui::VKEY_DIVIDE, 0));
  EXPECT_EQ("/",
      ConvertKeyCodeToTextNoError(ui::VKEY_DIVIDE, kShiftKeyModifierMask));

  EXPECT_EQ("", ConvertKeyCodeToTextNoError(ui::VKEY_SHIFT, 0));
  EXPECT_EQ("",
      ConvertKeyCodeToTextNoError(ui::VKEY_SHIFT, kShiftKeyModifierMask));
}

#if defined(OS_LINUX) && !defined(OS_CHROMEOS)
// Fails on bots: crbug.com/174962
#define MAYBE_CharToKeyCode DISABLED_CharToKeyCode
#else
#define MAYBE_CharToKeyCode CharToKeyCode
#endif

TEST(KeycodeTextConversionTest, MAYBE_CharToKeyCode) {
  CheckCharToKeyCode('a', ui::VKEY_A, 0);
  CheckCharToKeyCode('A', ui::VKEY_A, kShiftKeyModifierMask);

  CheckCharToKeyCode('1', ui::VKEY_1, 0);
  CheckCharToKeyCode('!', ui::VKEY_1, kShiftKeyModifierMask);

  CheckCharToKeyCode(',', ui::VKEY_OEM_COMMA, 0);
  CheckCharToKeyCode('<', ui::VKEY_OEM_COMMA, kShiftKeyModifierMask);

  CheckCharToKeyCode('/', ui::VKEY_OEM_2, 0);
  CheckCharToKeyCode('?', ui::VKEY_OEM_2, kShiftKeyModifierMask);

  CheckCantConvertChar(L'\u00E9');
  CheckCantConvertChar(L'\u2159');
}

#if (defined(OS_LINUX) && !defined(OS_CHROMEOS)) || defined(OS_MACOSX)
// Not implemented on Linux.
// Fails if German layout is not installed on Mac.
#define MAYBE_NonShiftModifiers DISABLED_NonShiftModifiers
#else
#define MAYBE_NonShiftModifiers NonShiftModifiers
#endif

TEST(KeycodeTextConversionTest, MAYBE_NonShiftModifiers) {
  ui::ScopedKeyboardLayout keyboard_layout(ui::KEYBOARD_LAYOUT_GERMAN);
#if defined(OS_WIN)
  int ctrl_and_alt = kControlKeyModifierMask | kAltKeyModifierMask;
  CheckCharToKeyCode('@', ui::VKEY_Q, ctrl_and_alt);
  EXPECT_EQ("@", ConvertKeyCodeToTextNoError(ui::VKEY_Q, ctrl_and_alt));
#elif defined(OS_MACOSX)
  EXPECT_EQ("@", ConvertKeyCodeToTextNoError(
      ui::VKEY_L, kAltKeyModifierMask));
#endif
}

#if (defined(OS_LINUX) && !defined(OS_CHROMEOS)) || defined(OS_MACOSX)
// Not implemented on Linux.
// Fails if tested layouts are not installed on Mac.
#define MAYBE_NonEnglish DISABLED_NonEnglish
#else
#define MAYBE_NonEnglish NonEnglish
#endif

TEST(KeycodeTextConversionTest, MAYBE_NonEnglish) {
  // For Greek and Russian keyboard layouts, which are very different from
  // QWERTY, Windows just uses virtual key codes that match the QWERTY layout,
  // and translates them to other characters.  If we wanted to test something
  // like German, whose layout is very similar to QWERTY, we'd need to be
  // careful, as in this case Windows maps the keyboard scan codes to the
  // appropriate (different) VKEYs instead of mapping the VKEYs to different
  // characters.
  {
    ui::ScopedKeyboardLayout greek_layout(ui::KEYBOARD_LAYOUT_GREEK);
    CheckCharToKeyCode(';', ui::VKEY_Q, 0);
    EXPECT_EQ(";", ConvertKeyCodeToTextNoError(ui::VKEY_Q, 0));
  }
  {
    // Regression test for chromedriver bug #405.
    ui::ScopedKeyboardLayout russian_layout(ui::KEYBOARD_LAYOUT_RUSSIAN);
    CheckCharToKeyCode(L'\u0438', ui::VKEY_B, 0);
    EXPECT_EQ(base::WideToUTF8(L"\u0438"),
              ConvertKeyCodeToTextNoError(ui::VKEY_B, 0));
  }
}
