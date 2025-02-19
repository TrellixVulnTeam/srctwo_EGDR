// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_AUTOFILL_CORE_BROWSER_AUTOFILL_TYPE_H_
#define COMPONENTS_AUTOFILL_CORE_BROWSER_AUTOFILL_TYPE_H_

#include <string>

#include "components/autofill/core/browser/field_types.h"

namespace autofill {

// The high-level description of Autofill types, used to categorize form fields
// and for associating form fields with form values in the Web Database.
class AutofillType {
 public:
  explicit AutofillType(ServerFieldType field_type);
  AutofillType(HtmlFieldType field_type, HtmlFieldMode mode);
  AutofillType(const AutofillType& autofill_type);
  AutofillType& operator=(const AutofillType& autofill_type);

  HtmlFieldType html_type() const { return html_type_; }

  FieldTypeGroup group() const;

  // Returns true if both the |server_type_| and the |html_type_| are set to
  // their respective enum's unknown value.
  bool IsUnknown() const;

  // Maps |this| type to a field type that can be directly stored in an Autofill
  // data model (in the sense that it makes sense to call
  // |AutofillDataModel::SetRawInfo()| with the returned field type as the first
  // parameter).  Note that the returned type might not be exactly equivalent to
  // |this| type.  For example, the HTML types 'country' and 'country-name' both
  // map to ADDRESS_HOME_COUNTRY.
  ServerFieldType GetStorableType() const;

  // Serializes |this| type to a string.
  std::string ToString() const;

  // Maps |field_type| to the corresponding billing field type if the field type
  // is an address, name, or phone number type.
  static ServerFieldType GetEquivalentBillingFieldType(
      ServerFieldType field_type);

  // Translates the ServerFieldType values into the corresponding strings.
  static std::string ServerFieldTypeToString(ServerFieldType type);

 private:
  // The server-native field type, or UNKNOWN_TYPE if unset.
  ServerFieldType server_type_;

  // The HTML autocomplete field type and mode hints, or HTML_TYPE_UNKNOWN and
  // HTML_MODE_NONE if unset.
  HtmlFieldType html_type_;
  HtmlFieldMode html_mode_;
};

}  // namespace autofill

#endif  // COMPONENTS_AUTOFILL_CORE_BROWSER_AUTOFILL_TYPE_H_
