/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef FormSubmission_h
#define FormSubmission_h

#include "core/loader/FrameLoadRequest.h"
#include "platform/heap/Handle.h"
#include "platform/weborigin/KURL.h"
#include "platform/weborigin/Referrer.h"
#include "platform/wtf/Allocator.h"

namespace blink {

class Document;
class EncodedFormData;
class Event;
class HTMLFormControlElement;
class HTMLFormElement;

class FormSubmission : public GarbageCollectedFinalized<FormSubmission> {
 public:
  enum SubmitMethod { kGetMethod, kPostMethod, kDialogMethod };

  class Attributes {
    DISALLOW_NEW();
    WTF_MAKE_NONCOPYABLE(Attributes);

   public:
    Attributes()
        : method_(kGetMethod),
          is_multi_part_form_(false),
          encoding_type_("application/x-www-form-urlencoded") {}

    SubmitMethod Method() const { return method_; }
    static SubmitMethod ParseMethodType(const String&);
    void UpdateMethodType(const String&);
    static String MethodString(SubmitMethod);

    const String& Action() const { return action_; }
    void ParseAction(const String&);

    const AtomicString& Target() const { return target_; }
    void SetTarget(const AtomicString& target) { target_ = target; }

    const AtomicString& EncodingType() const { return encoding_type_; }
    static AtomicString ParseEncodingType(const String&);
    void UpdateEncodingType(const String&);
    bool IsMultiPartForm() const { return is_multi_part_form_; }

    const String& AcceptCharset() const { return accept_charset_; }
    void SetAcceptCharset(const String& value) { accept_charset_ = value; }

    void CopyFrom(const Attributes&);

   private:
    SubmitMethod method_;
    bool is_multi_part_form_;

    String action_;
    AtomicString target_;
    AtomicString encoding_type_;
    String accept_charset_;
  };

  static FormSubmission* Create(HTMLFormElement*,
                                const Attributes&,
                                Event*,
                                HTMLFormControlElement* submit_button);
  DECLARE_TRACE();

  FrameLoadRequest CreateFrameLoadRequest(Document* origin_document);

  KURL RequestURL() const;

  SubmitMethod Method() const { return method_; }
  const KURL& Action() const { return action_; }
  const AtomicString& Target() const { return target_; }
  void ClearTarget() { target_ = g_null_atom; }
  HTMLFormElement* Form() const { return form_.Get(); }
  EncodedFormData* Data() const { return form_data_.Get(); }

  const String& Result() const { return result_; }

 private:
  FormSubmission(SubmitMethod,
                 const KURL& action,
                 const AtomicString& target,
                 const AtomicString& content_type,
                 HTMLFormElement*,
                 RefPtr<EncodedFormData>,
                 const String& boundary,
                 Event*);
  // FormSubmission for DialogMethod
  explicit FormSubmission(const String& result);

  // FIXME: Hold an instance of Attributes instead of individual members.
  SubmitMethod method_;
  KURL action_;
  AtomicString target_;
  AtomicString content_type_;
  Member<HTMLFormElement> form_;
  RefPtr<EncodedFormData> form_data_;
  String boundary_;
  Member<Event> event_;
  String result_;
};

}  // namespace blink

#endif  // FormSubmission_h
