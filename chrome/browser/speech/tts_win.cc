// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <math.h>
#include <objbase.h>
#include <sapi.h>
#include <sphelper.h>
#include <stdint.h>

#include "base/macros.h"
#include "base/memory/singleton.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_piece.h"
#include "base/strings/utf_string_conversions.h"
#include "base/values.h"
#include "base/win/scoped_co_mem.h"
#include "base/win/scoped_comptr.h"
#include "chrome/browser/speech/tts_controller.h"
#include "chrome/browser/speech/tts_platform.h"

namespace {

// ISpObjectToken key and value names.
const wchar_t kAttributesKey[] = L"Attributes";
const wchar_t kGenderValue[] = L"Gender";
const wchar_t kLanguageValue[] = L"Language";

}  // anonymous namespace.

class TtsPlatformImplWin : public TtsPlatformImpl {
 public:
  bool PlatformImplAvailable() override {
    return true;
  }

  bool Speak(
      int utterance_id,
      const std::string& utterance,
      const std::string& lang,
      const VoiceData& voice,
      const UtteranceContinuousParameters& params) override;

  bool StopSpeaking() override;

  void Pause() override;

  void Resume() override;

  bool IsSpeaking() override;

  void GetVoices(std::vector<VoiceData>* out_voices) override;

  // Get the single instance of this class.
  static TtsPlatformImplWin* GetInstance();

  static void __stdcall SpeechEventCallback(WPARAM w_param, LPARAM l_param);

 private:
  TtsPlatformImplWin();
  ~TtsPlatformImplWin() override {}

  void OnSpeechEvent();

  void SetVoiceFromName(const std::string& name);

  base::win::ScopedComPtr<ISpVoice> speech_synthesizer_;

  // These apply to the current utterance only.
  std::wstring utterance_;
  int utterance_id_;
  int prefix_len_;
  ULONG stream_number_;
  int char_position_;
  bool paused_;
  std::string last_voice_name_;

  friend struct base::DefaultSingletonTraits<TtsPlatformImplWin>;

  DISALLOW_COPY_AND_ASSIGN(TtsPlatformImplWin);
};

// static
TtsPlatformImpl* TtsPlatformImpl::GetInstance() {
  return TtsPlatformImplWin::GetInstance();
}

bool TtsPlatformImplWin::Speak(
    int utterance_id,
    const std::string& src_utterance,
    const std::string& lang,
    const VoiceData& voice,
    const UtteranceContinuousParameters& params) {
  std::wstring prefix;
  std::wstring suffix;

  if (!speech_synthesizer_.Get())
    return false;

  SetVoiceFromName(voice.name);

  if (params.rate >= 0.0) {
    // Map our multiplicative range of 0.1x to 10.0x onto Microsoft's
    // linear range of -10 to 10:
    //   0.1 -> -10
    //   1.0 -> 0
    //  10.0 -> 10
    speech_synthesizer_->SetRate(static_cast<int32_t>(10 * log10(params.rate)));
  }

  if (params.pitch >= 0.0) {
    // The TTS api allows a range of -10 to 10 for speech pitch.
    // TODO(dtseng): cleanup if we ever use any other properties that
    // require xml.
    std::wstring pitch_value =
        base::IntToString16(static_cast<int>(params.pitch * 10 - 10));
    prefix = L"<pitch absmiddle=\"" + pitch_value + L"\">";
    suffix = L"</pitch>";
  }

  if (params.volume >= 0.0) {
    // The TTS api allows a range of 0 to 100 for speech volume.
    speech_synthesizer_->SetVolume(static_cast<uint16_t>(params.volume * 100));
  }

  // TODO(dmazzoni): convert SSML to SAPI xml. http://crbug.com/88072

  utterance_ = base::UTF8ToWide(src_utterance);
  utterance_id_ = utterance_id;
  char_position_ = 0;
  std::wstring merged_utterance = prefix + utterance_ + suffix;
  prefix_len_ = prefix.size();

  HRESULT result = speech_synthesizer_->Speak(
      merged_utterance.c_str(),
      SPF_ASYNC,
      &stream_number_);
  return (result == S_OK);
}

bool TtsPlatformImplWin::StopSpeaking() {
  if (speech_synthesizer_.Get()) {
    // Clear the stream number so that any further events relating to this
    // utterance are ignored.
    stream_number_ = 0;

    if (IsSpeaking()) {
      // Stop speech by speaking the empty string with the purge flag.
      speech_synthesizer_->Speak(L"", SPF_ASYNC | SPF_PURGEBEFORESPEAK, NULL);
    }
    if (paused_) {
      speech_synthesizer_->Resume();
      paused_ = false;
    }
  }
  return true;
}

void TtsPlatformImplWin::Pause() {
  if (speech_synthesizer_.Get() && utterance_id_ && !paused_) {
    speech_synthesizer_->Pause();
    paused_ = true;
    TtsController::GetInstance()->OnTtsEvent(
        utterance_id_, TTS_EVENT_PAUSE, char_position_, "");
  }
}

void TtsPlatformImplWin::Resume() {
  if (speech_synthesizer_.Get() && utterance_id_ && paused_) {
    speech_synthesizer_->Resume();
    paused_ = false;
    TtsController::GetInstance()->OnTtsEvent(
        utterance_id_, TTS_EVENT_RESUME, char_position_, "");
  }
}

bool TtsPlatformImplWin::IsSpeaking() {
  if (speech_synthesizer_.Get()) {
    SPVOICESTATUS status;
    HRESULT result = speech_synthesizer_->GetStatus(&status, NULL);
    if (result == S_OK) {
      if (status.dwRunningState == 0 ||  // 0 == waiting to speak
          status.dwRunningState == SPRS_IS_SPEAKING) {
        return true;
      }
    }
  }
  return false;
}

void TtsPlatformImplWin::GetVoices(
    std::vector<VoiceData>* out_voices) {
  base::win::ScopedComPtr<IEnumSpObjectTokens> voice_tokens;
  unsigned long voice_count;
  if (S_OK !=
      SpEnumTokens(SPCAT_VOICES, NULL, NULL, voice_tokens.GetAddressOf()))
    return;
  if (S_OK != voice_tokens->GetCount(&voice_count))
    return;

  for (unsigned i = 0; i < voice_count; i++) {
    VoiceData voice;

    base::win::ScopedComPtr<ISpObjectToken> voice_token;
    if (S_OK != voice_tokens->Next(1, voice_token.GetAddressOf(), NULL))
      return;

    base::win::ScopedCoMem<WCHAR> description;
    if (S_OK != SpGetDescription(voice_token.Get(), &description))
      continue;
    voice.name = base::WideToUTF8(description.get());

    base::win::ScopedComPtr<ISpDataKey> attributes;
    if (S_OK != voice_token->OpenKey(kAttributesKey, attributes.GetAddressOf()))
      continue;

    base::win::ScopedCoMem<WCHAR> gender;
    if (S_OK == attributes->GetStringValue(kGenderValue, &gender)) {
      if (0 == _wcsicmp(gender.get(), L"male"))
        voice.gender = TTS_GENDER_MALE;
      else if (0 == _wcsicmp(gender.get(), L"female"))
        voice.gender = TTS_GENDER_FEMALE;
    }

    base::win::ScopedCoMem<WCHAR> language;
    if (S_OK == attributes->GetStringValue(kLanguageValue, &language)) {
      int lcid_value;
      base::HexStringToInt(base::WideToUTF8(language.get()), &lcid_value);
      LCID lcid = MAKELCID(lcid_value, SORT_DEFAULT);
      WCHAR locale_name[LOCALE_NAME_MAX_LENGTH] = {0};
      LCIDToLocaleName(lcid, locale_name, LOCALE_NAME_MAX_LENGTH, 0);
      voice.lang = base::WideToUTF8(locale_name);
    }

    voice.native = true;
    voice.events.insert(TTS_EVENT_START);
    voice.events.insert(TTS_EVENT_END);
    voice.events.insert(TTS_EVENT_MARKER);
    voice.events.insert(TTS_EVENT_WORD);
    voice.events.insert(TTS_EVENT_SENTENCE);
    voice.events.insert(TTS_EVENT_PAUSE);
    voice.events.insert(TTS_EVENT_RESUME);
    out_voices->push_back(voice);
  }
}

void TtsPlatformImplWin::OnSpeechEvent() {
  TtsController* controller = TtsController::GetInstance();
  SPEVENT event;
  while (S_OK == speech_synthesizer_->GetEvents(1, &event, NULL)) {
    if (event.ulStreamNum != stream_number_)
      continue;

    switch (event.eEventId) {
    case SPEI_START_INPUT_STREAM:
      controller->OnTtsEvent(
          utterance_id_, TTS_EVENT_START, 0, std::string());
      break;
    case SPEI_END_INPUT_STREAM:
      char_position_ = utterance_.size();
      controller->OnTtsEvent(
          utterance_id_, TTS_EVENT_END, char_position_, std::string());
      break;
    case SPEI_TTS_BOOKMARK:
      controller->OnTtsEvent(
          utterance_id_, TTS_EVENT_MARKER, char_position_, std::string());
      break;
    case SPEI_WORD_BOUNDARY:
      char_position_ = static_cast<ULONG>(event.lParam) - prefix_len_;
      controller->OnTtsEvent(
          utterance_id_, TTS_EVENT_WORD, char_position_,
          std::string());
      break;
    case SPEI_SENTENCE_BOUNDARY:
      char_position_ = static_cast<ULONG>(event.lParam) - prefix_len_;
      controller->OnTtsEvent(
          utterance_id_, TTS_EVENT_SENTENCE, char_position_,
          std::string());
      break;
    default:
      break;
    }
  }
}

void TtsPlatformImplWin::SetVoiceFromName(const std::string& name) {
  if (name.empty() || name == last_voice_name_)
    return;

  last_voice_name_ = name;

  base::win::ScopedComPtr<IEnumSpObjectTokens> voice_tokens;
  unsigned long voice_count;
  if (S_OK !=
      SpEnumTokens(SPCAT_VOICES, NULL, NULL, voice_tokens.GetAddressOf()))
    return;
  if (S_OK != voice_tokens->GetCount(&voice_count))
    return;

  for (unsigned i = 0; i < voice_count; i++) {
    base::win::ScopedComPtr<ISpObjectToken> voice_token;
    if (S_OK != voice_tokens->Next(1, voice_token.GetAddressOf(), NULL))
      return;

    base::win::ScopedCoMem<WCHAR> description;
    if (S_OK != SpGetDescription(voice_token.Get(), &description))
      continue;
    if (name == base::WideToUTF8(description.get())) {
      speech_synthesizer_->SetVoice(voice_token.Get());
      break;
    }
  }
}

TtsPlatformImplWin::TtsPlatformImplWin()
  : utterance_id_(0),
    prefix_len_(0),
    stream_number_(0),
    char_position_(0),
    paused_(false) {
  ::CoCreateInstance(CLSID_SpVoice, nullptr, CLSCTX_ALL,
                     IID_PPV_ARGS(&speech_synthesizer_));
  if (speech_synthesizer_.Get()) {
    ULONGLONG event_mask =
        SPFEI(SPEI_START_INPUT_STREAM) |
        SPFEI(SPEI_TTS_BOOKMARK) |
        SPFEI(SPEI_WORD_BOUNDARY) |
        SPFEI(SPEI_SENTENCE_BOUNDARY) |
        SPFEI(SPEI_END_INPUT_STREAM);
    speech_synthesizer_->SetInterest(event_mask, event_mask);
    speech_synthesizer_->SetNotifyCallbackFunction(
        TtsPlatformImplWin::SpeechEventCallback, 0, 0);
  }
}

// static
TtsPlatformImplWin* TtsPlatformImplWin::GetInstance() {
  return base::Singleton<TtsPlatformImplWin,
                         base::LeakySingletonTraits<TtsPlatformImplWin>>::get();
}

// static
void TtsPlatformImplWin::SpeechEventCallback(
    WPARAM w_param, LPARAM l_param) {
  GetInstance()->OnSpeechEvent();
}
