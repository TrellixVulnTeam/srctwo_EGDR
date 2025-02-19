// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_UI_PAYMENTS_CONTACT_INFO_EDIT_MEDIATOR_H_
#define IOS_CHROME_BROWSER_UI_PAYMENTS_CONTACT_INFO_EDIT_MEDIATOR_H_

#import <Foundation/Foundation.h>

#import "ios/chrome/browser/ui/payments/payment_request_edit_view_controller_data_source.h"

@protocol PaymentRequestEditConsumer;

namespace autofill {
class AutofillProfile;
}  // namespace autofill

namespace payments {
class PaymentRequest;
}  // namespace payments

// Serves as data source for AddressEditViewController.
@interface ContactInfoEditMediator
    : NSObject<PaymentRequestEditViewControllerDataSource>

// The consumer for this object. This can change during the lifetime of this
// object and may be nil.
@property(nonatomic, weak) id<PaymentRequestEditConsumer> consumer;

// Initializes this object with an instance of PaymentRequest which has a copy
// of web::PaymentRequest as provided by the page invoking the Payment Request
// API as well as |profile| which is the profile to be edited, if any.
// This object will not take ownership of |paymentRequest| or |profile|.
- (instancetype)initWithPaymentRequest:(payments::PaymentRequest*)paymentRequest
                               profile:(autofill::AutofillProfile*)profile
    NS_DESIGNATED_INITIALIZER;

- (instancetype)init NS_UNAVAILABLE;

@end

#endif  // IOS_CHROME_BROWSER_UI_PAYMENTS_CONTACT_INFO_EDIT_MEDIATOR_H_
