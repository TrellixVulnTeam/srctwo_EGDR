// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_AUTOFILL_CORE_COMMON_FORM_DATA_H_
#define COMPONENTS_AUTOFILL_CORE_COMMON_FORM_DATA_H_

#include <vector>

#include "base/strings/string16.h"
#include "components/autofill/core/common/form_field_data.h"
#include "url/gurl.h"

namespace autofill {

// Holds information about a form to be filled and/or submitted.
struct FormData {
  FormData();
  FormData(const FormData& data);
  ~FormData();

  // Returns true if two forms are the same, not counting the values of the
  // form elements.
  bool SameFormAs(const FormData& other) const;

  // Same as SameFormAs() except calling FormFieldData.SimilarFieldAs() to
  // compare fields.
  bool SimilarFormAs(const FormData& other) const;

  // Note: operator==() performs a full-field-comparison(byte by byte), this is
  // different from SameFormAs(), which ignores comparison for those "values" of
  // all form fields, just like what FormFieldData::SameFieldAs() ignores.
  bool operator==(const FormData& form) const;
  bool operator!=(const FormData& form) const;
  // Allow FormData to be a key in STL containers.
  bool operator<(const FormData& form) const;

  // The name of the form.
  base::string16 name;
  // The URL (minus query parameters) containing the form.
  GURL origin;
  // The action target of the form.
  GURL action;
  // True if this form is a form tag.
  bool is_form_tag;
  // True if the form is made of unowned fields in a non checkout flow.
  bool is_formless_checkout;
  // A vector of all the input fields in the form.
  std::vector<FormFieldData> fields;
};

// For testing.
std::ostream& operator<<(std::ostream& os, const FormData& form);

// Serialize FormData. Used by the PasswordManager to persist FormData
// pertaining to password forms. Serialized data is appended to |pickle|.
void SerializeFormData(const FormData& form_data, base::Pickle* pickle);
// Deserialize FormData. This assumes that |iter| is currently pointing to
// the part of a pickle created by SerializeFormData. Returns true on success.
bool DeserializeFormData(base::PickleIterator* iter, FormData* form_data);

// Serialize FormData. Used by the PasswordManager to persist FormData
// pertaining to password forms in base64 string. It is useful since in some
// cases we need to store C strings without embedded '\0' symbols.
void SerializeFormDataToBase64String(const FormData& form_data,
                                     std::string* output);
// Deserialize FormData. Returns true on success.
bool DeserializeFormDataFromBase64String(const base::StringPiece& input,
                                         FormData* form_data);

}  // namespace autofill

#endif  // COMPONENTS_AUTOFILL_CORE_COMMON_FORM_DATA_H_
