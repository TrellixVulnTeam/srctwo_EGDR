// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_AUTOFILL_CORE_BROWSER_WEBDATA_AUTOFILL_WEBDATA_BACKEND_H_
#define COMPONENTS_AUTOFILL_CORE_BROWSER_WEBDATA_AUTOFILL_WEBDATA_BACKEND_H_

#include "components/sync/base/model_type.h"

class WebDatabase;

namespace autofill {

class AutofillWebDataServiceObserverOnDBSequence;

// Interface for doing Autofill work directly on the DB sequence (used by
// Sync, mostly), without fully exposing the AutofillWebDataBackend to clients.
class AutofillWebDataBackend {
 public:
  virtual ~AutofillWebDataBackend() {}

  // Get a raw pointer to the WebDatabase.
  virtual WebDatabase* GetDatabase() = 0;

  // Add an observer to be notified of changes on the DB sequence.
  virtual void AddObserver(
      AutofillWebDataServiceObserverOnDBSequence* observer) = 0;

  // Remove an observer.
  virtual void RemoveObserver(
      AutofillWebDataServiceObserverOnDBSequence* observer) = 0;

  // Remove expired elements from the database and commit if needed.
  virtual void RemoveExpiredFormElements() = 0;

  // Notifies listeners on both DB and UI sequences that multiple changes have
  // been made to to Autofill records of the database.
  // NOTE: This method is intended to be called from the DB sequence. The UI
  // sequence notifications are asynchronous.
  virtual void NotifyOfMultipleAutofillChanges() = 0;

  // Notifies listeners on the UI sequence that sync has started for
  // |model_type|.
  // NOTE: This method is intended to be called from the DB sequence. The UI
  // sequence notifications are asynchronous.
  virtual void NotifyThatSyncHasStarted(syncer::ModelType model_type) = 0;
};

} // namespace autofill

#endif  // COMPONENTS_AUTOFILL_CORE_BROWSER_WEBDATA_AUTOFILL_WEBDATA_BACKEND_H_
