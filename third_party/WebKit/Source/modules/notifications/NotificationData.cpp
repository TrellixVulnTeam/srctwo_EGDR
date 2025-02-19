// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "modules/notifications/NotificationData.h"

#include "bindings/core/v8/ExceptionState.h"
#include "bindings/core/v8/serialization/SerializedScriptValue.h"
#include "bindings/core/v8/serialization/SerializedScriptValueFactory.h"
#include "core/dom/ExecutionContext.h"
#include "modules/notifications/Notification.h"
#include "modules/notifications/NotificationOptions.h"
#include "modules/vibration/VibrationController.h"
#include "platform/wtf/CurrentTime.h"
#include "public/platform/WebURL.h"

namespace blink {
namespace {

WebNotificationData::Direction ToDirectionEnumValue(const String& direction) {
  if (direction == "ltr")
    return WebNotificationData::kDirectionLeftToRight;
  if (direction == "rtl")
    return WebNotificationData::kDirectionRightToLeft;

  return WebNotificationData::kDirectionAuto;
}

WebURL CompleteURL(ExecutionContext* execution_context,
                   const String& string_url) {
  WebURL url = execution_context->CompleteURL(string_url);
  if (url.IsValid())
    return url;
  return WebURL();
}

}  // namespace

WebNotificationData CreateWebNotificationData(
    ExecutionContext* execution_context,
    const String& title,
    const NotificationOptions& options,
    ExceptionState& exception_state) {
  // If silent is true, the notification must not have a vibration pattern.
  if (options.hasVibrate() && options.silent()) {
    exception_state.ThrowTypeError(
        "Silent notifications must not specify vibration patterns.");
    return WebNotificationData();
  }

  // If renotify is true, the notification must have a tag.
  if (options.renotify() && options.tag().IsEmpty()) {
    exception_state.ThrowTypeError(
        "Notifications which set the renotify flag must specify a non-empty "
        "tag.");
    return WebNotificationData();
  }

  WebNotificationData web_data;

  web_data.title = title;
  web_data.direction = ToDirectionEnumValue(options.dir());
  web_data.lang = options.lang();
  web_data.body = options.body();
  web_data.tag = options.tag();

  if (options.hasImage() && !options.image().IsEmpty())
    web_data.image = CompleteURL(execution_context, options.image());

  if (options.hasIcon() && !options.icon().IsEmpty())
    web_data.icon = CompleteURL(execution_context, options.icon());

  if (options.hasBadge() && !options.badge().IsEmpty())
    web_data.badge = CompleteURL(execution_context, options.badge());

  web_data.vibrate =
      VibrationController::SanitizeVibrationPattern(options.vibrate());
  web_data.timestamp = options.hasTimestamp()
                           ? static_cast<double>(options.timestamp())
                           : WTF::CurrentTimeMS();
  web_data.renotify = options.renotify();
  web_data.silent = options.silent();
  web_data.require_interaction = options.requireInteraction();

  if (options.hasData()) {
    const ScriptValue& data = options.data();
    v8::Isolate* isolate = data.GetIsolate();
    DCHECK(isolate->InContext());
    SerializedScriptValue::SerializeOptions options;
    options.for_storage = SerializedScriptValue::kForStorage;
    RefPtr<SerializedScriptValue> serialized_script_value =
        SerializedScriptValue::Serialize(isolate, data.V8Value(), options,
                                         exception_state);
    if (exception_state.HadException())
      return WebNotificationData();

    Vector<char> serialized_data;
    serialized_script_value->ToWireBytes(serialized_data);

    web_data.data = serialized_data;
  }

  Vector<WebNotificationAction> actions;

  const size_t max_actions = Notification::maxActions();
  for (const NotificationAction& action : options.actions()) {
    if (actions.size() >= max_actions)
      break;

    WebNotificationAction web_action;
    web_action.action = action.action();
    web_action.title = action.title();

    if (action.type() == "button")
      web_action.type = WebNotificationAction::kButton;
    else if (action.type() == "text")
      web_action.type = WebNotificationAction::kText;
    else
      NOTREACHED() << "Unknown action type: " << action.type();

    if (action.hasPlaceholder() &&
        web_action.type == WebNotificationAction::kButton) {
      exception_state.ThrowTypeError(
          "Notifications of type \"button\" cannot specify a placeholder.");
      return WebNotificationData();
    }

    web_action.placeholder = action.placeholder();

    if (action.hasIcon() && !action.icon().IsEmpty())
      web_action.icon = CompleteURL(execution_context, action.icon());

    actions.push_back(web_action);
  }

  web_data.actions = actions;

  return web_data;
}

}  // namespace blink
