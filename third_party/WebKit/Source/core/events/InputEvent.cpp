// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/events/InputEvent.h"

#include "core/dom/Range.h"
#include "core/dom/events/EventDispatcher.h"
#include "public/platform/WebEditingCommandType.h"

namespace blink {

namespace {

const struct {
  InputEvent::InputType input_type;
  const char* string_name;
} kInputTypeStringNameMap[] = {
    {InputEvent::InputType::kNone, ""},
    {InputEvent::InputType::kInsertText, "insertText"},
    {InputEvent::InputType::kInsertLineBreak, "insertLineBreak"},
    {InputEvent::InputType::kInsertParagraph, "insertParagraph"},
    {InputEvent::InputType::kInsertOrderedList, "insertOrderedList"},
    {InputEvent::InputType::kInsertUnorderedList, "insertUnorderedList"},
    {InputEvent::InputType::kInsertHorizontalRule, "insertHorizontalRule"},
    {InputEvent::InputType::kInsertFromPaste, "insertFromPaste"},
    {InputEvent::InputType::kInsertFromDrop, "insertFromDrop"},
    {InputEvent::InputType::kInsertFromYank, "insertFromYank"},
    {InputEvent::InputType::kInsertTranspose, "insertTranspose"},
    {InputEvent::InputType::kInsertReplacementText, "insertReplacementText"},
    {InputEvent::InputType::kInsertCompositionText, "insertCompositionText"},
    {InputEvent::InputType::kDeleteWordBackward, "deleteWordBackward"},
    {InputEvent::InputType::kDeleteWordForward, "deleteWordForward"},
    {InputEvent::InputType::kDeleteSoftLineBackward, "deleteSoftLineBackward"},
    {InputEvent::InputType::kDeleteSoftLineForward, "deleteSoftLineForward"},
    {InputEvent::InputType::kDeleteHardLineBackward, "deleteHardLineBackward"},
    {InputEvent::InputType::kDeleteHardLineForward, "deleteHardLineForward"},
    {InputEvent::InputType::kDeleteContentBackward, "deleteContentBackward"},
    {InputEvent::InputType::kDeleteContentForward, "deleteContentForward"},
    {InputEvent::InputType::kDeleteByCut, "deleteByCut"},
    {InputEvent::InputType::kDeleteByDrag, "deleteByDrag"},
    {InputEvent::InputType::kHistoryUndo, "historyUndo"},
    {InputEvent::InputType::kHistoryRedo, "historyRedo"},
    {InputEvent::InputType::kFormatBold, "formatBold"},
    {InputEvent::InputType::kFormatItalic, "formatItalic"},
    {InputEvent::InputType::kFormatUnderline, "formatUnderline"},
    {InputEvent::InputType::kFormatStrikeThrough, "formatStrikeThrough"},
    {InputEvent::InputType::kFormatSuperscript, "formatSuperscript"},
    {InputEvent::InputType::kFormatSubscript, "formatSubscript"},
    {InputEvent::InputType::kFormatJustifyCenter, "formatJustifyCenter"},
    {InputEvent::InputType::kFormatJustifyFull, "formatJustifyFull"},
    {InputEvent::InputType::kFormatJustifyRight, "formatJustifyRight"},
    {InputEvent::InputType::kFormatJustifyLeft, "formatJustifyLeft"},
    {InputEvent::InputType::kFormatIndent, "formatIndent"},
    {InputEvent::InputType::kFormatOutdent, "formatOutdent"},
    {InputEvent::InputType::kFormatRemove, "formatRemove"},
    {InputEvent::InputType::kFormatSetBlockTextDirection,
     "formatSetBlockTextDirection"},
};

static_assert(
    arraysize(kInputTypeStringNameMap) ==
        static_cast<size_t>(InputEvent::InputType::kNumberOfInputTypes),
    "must handle all InputEvent::InputType");

String ConvertInputTypeToString(InputEvent::InputType input_type) {
  const auto& it =
      std::begin(kInputTypeStringNameMap) + static_cast<size_t>(input_type);
  if (it >= std::begin(kInputTypeStringNameMap) &&
      it < std::end(kInputTypeStringNameMap))
    return AtomicString(it->string_name);
  return g_empty_string;
}

InputEvent::InputType ConvertStringToInputType(const String& string_name) {
  // TODO(chongz): Use binary search if the map goes larger.
  for (const auto& entry : kInputTypeStringNameMap) {
    if (string_name == entry.string_name)
      return entry.input_type;
  }
  return InputEvent::InputType::kNone;
}

}  // anonymous namespace

InputEvent::InputEvent(const AtomicString& type,
                       const InputEventInit& initializer)
    : UIEvent(type, initializer) {
  // TODO(ojan): We should find a way to prevent conversion like
  // String->enum->String just in order to use initializer.
  // See InputEvent::createBeforeInput() for the first conversion.
  if (initializer.hasInputType())
    input_type_ = ConvertStringToInputType(initializer.inputType());
  if (initializer.hasData())
    data_ = initializer.data();
  if (initializer.hasDataTransfer())
    data_transfer_ = initializer.dataTransfer();
  if (initializer.hasIsComposing())
    is_composing_ = initializer.isComposing();
  if (!initializer.hasTargetRanges())
    return;
  for (const auto& range : initializer.targetRanges())
    ranges_.push_back(range->toRange());
}

/* static */
InputEvent* InputEvent::CreateBeforeInput(InputType input_type,
                                          const String& data,
                                          EventCancelable cancelable,
                                          EventIsComposing is_composing,
                                          const StaticRangeVector* ranges) {
  InputEventInit input_event_init;

  input_event_init.setBubbles(true);
  input_event_init.setCancelable(cancelable == kIsCancelable);
  // TODO(ojan): We should find a way to prevent conversion like
  // String->enum->String just in order to use initializer.
  // See InputEvent::InputEvent() for the second conversion.
  input_event_init.setInputType(ConvertInputTypeToString(input_type));
  input_event_init.setData(data);
  input_event_init.setIsComposing(is_composing == kIsComposing);
  if (ranges)
    input_event_init.setTargetRanges(*ranges);
  input_event_init.setComposed(true);
  return InputEvent::Create(EventTypeNames::beforeinput, input_event_init);
}

/* static */
InputEvent* InputEvent::CreateBeforeInput(InputType input_type,
                                          DataTransfer* data_transfer,
                                          EventCancelable cancelable,
                                          EventIsComposing is_composing,
                                          const StaticRangeVector* ranges) {
  InputEventInit input_event_init;

  input_event_init.setBubbles(true);
  input_event_init.setCancelable(cancelable == kIsCancelable);
  input_event_init.setInputType(ConvertInputTypeToString(input_type));
  input_event_init.setDataTransfer(data_transfer);
  input_event_init.setIsComposing(is_composing == kIsComposing);
  if (ranges)
    input_event_init.setTargetRanges(*ranges);
  input_event_init.setComposed(true);
  return InputEvent::Create(EventTypeNames::beforeinput, input_event_init);
}

/* static */
InputEvent* InputEvent::CreateInput(InputType input_type,
                                    const String& data,
                                    EventIsComposing is_composing,
                                    const StaticRangeVector* ranges) {
  InputEventInit input_event_init;

  input_event_init.setBubbles(true);
  input_event_init.setCancelable(false);
  // TODO(ojan): We should find a way to prevent conversion like
  // String->enum->String just in order to use initializer.
  // See InputEvent::InputEvent() for the second conversion.
  input_event_init.setInputType(ConvertInputTypeToString(input_type));
  input_event_init.setData(data);
  input_event_init.setIsComposing(is_composing == kIsComposing);
  if (ranges)
    input_event_init.setTargetRanges(*ranges);
  input_event_init.setComposed(true);
  return InputEvent::Create(EventTypeNames::input, input_event_init);
}

String InputEvent::inputType() const {
  return ConvertInputTypeToString(input_type_);
}

StaticRangeVector InputEvent::getTargetRanges() const {
  StaticRangeVector static_ranges;
  for (const auto& range : ranges_)
    static_ranges.push_back(StaticRange::Create(range));
  return static_ranges;
}

bool InputEvent::IsInputEvent() const {
  return true;
}

// TODO(chongz): We should get rid of this |EventDispatchMediator| pattern and
// introduce simpler interface such as |beforeDispatchEvent()| and
// |afterDispatchEvent()| virtual methods.
EventDispatchMediator* InputEvent::CreateMediator() {
  return InputEventDispatchMediator::Create(this);
}

DEFINE_TRACE(InputEvent) {
  UIEvent::Trace(visitor);
  visitor->Trace(data_transfer_);
  visitor->Trace(ranges_);
}

InputEventDispatchMediator* InputEventDispatchMediator::Create(
    InputEvent* input_event) {
  return new InputEventDispatchMediator(input_event);
}

InputEventDispatchMediator::InputEventDispatchMediator(InputEvent* input_event)
    : EventDispatchMediator(input_event) {}

InputEvent& InputEventDispatchMediator::Event() const {
  return ToInputEvent(EventDispatchMediator::GetEvent());
}

DispatchEventResult InputEventDispatchMediator::DispatchEvent(
    EventDispatcher& dispatcher) const {
  DispatchEventResult result = dispatcher.Dispatch();
  // It's weird to hold and clear live |Range| objects internally, and only
  // expose |StaticRange| through |getTargetRanges()|. However there is no
  // better solutions due to the following issues:
  //   1. We don't want to expose live |Range| objects for the author to hold as
  //      it will slow down all DOM operations. So we just expose |StaticRange|.
  //   2. Event handlers in chain might modify DOM, which means we have to keep
  //      a copy of live |Range| internally and return snapshots.
  //   3. We don't want authors to hold live |Range| indefinitely by holding
  //      |InputEvent|, so we clear them after dispatch.
  // Authors should explicitly call |getTargetRanges()|->|toRange()| if they
  // want to keep a copy of |Range|.  See Editing TF meeting notes:
  // https://docs.google.com/document/d/1hCj6QX77NYIVY0RWrMHT1Yra6t8_Qu8PopaWLG0AM58/edit?usp=sharing
  Event().ranges_.clear();
  return result;
}

}  // namespace blink
