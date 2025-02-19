// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

cr.define('chrome.invalidations', function() {
  /**
   * Local variable where we maintain a count of the invalidations received
   * and of every ObjectId that has ever been updated (note that this doesn't
   * log any invalidations ocurred prior to opening the about:invalidation
   * page).
   */
  var tableObjects = {};

  /**
   * Local variable that contains the detailed information in an object form.
   * This was done this way as to allow multiple calls to updateDetailedStatus
   * to keep adding new items.
   */
  var cachedDetails = {};

  function quote(str) {
    return '\"' + str + '\"';
  }

  function nowTimeString() {
    return '[' + new Date().getTime() + '] ';
  }

  /**
   * Appends a string to a textarea log.
   * @param {string} logMessage The string to be appended.
   */
  function appendToLog(logMessage) {
    var invalidationsLog = $('invalidations-log');
    invalidationsLog.value += logMessage + '\n';
  }
  /**
   *  Updates the jstemplate with the latest ObjectIds, ordered by registrar.
   */
  function repaintTable() {
    var keys = [];
    for (var key in tableObjects) {
      keys.push(key);
    }
    keys.sort();
    var sortedInvalidations = [];
    for (var i = 0; i < keys.length; i++) {
      sortedInvalidations.push(tableObjects[keys[i]]);
    }
    var wrapped = {objectsidtable: sortedInvalidations};
    jstProcess(new JsEvalContext(wrapped), $('objectsid-table-div'));
  }

  /**
   * Shows the current state of the InvalidatorService.
   * @param {string} newState The string to be displayed and logged.
   * @param {number} lastChangedTime The time in epoch when the state was last
   *     changed.
   */
  function updateInvalidatorState(newState, lastChangedTime) {
    var logMessage = nowTimeString() +
        'Invalidations service state changed to ' + quote(newState);

    appendToLog(logMessage);
    $('invalidations-state').textContent =
        newState + ' (since ' + new Date(lastChangedTime) + ')';
  }

  /**
   * Adds to the log the latest invalidations received
   * @param {!Array<!Object>} allInvalidations The array of ObjectId
   *     that contains the invalidations received by the InvalidatorService.
   */
  function logInvalidations(allInvalidations) {
    for (var i = 0; i < allInvalidations.length; i++) {
      var inv = allInvalidations[i];
      if (inv.hasOwnProperty('objectId')) {
        var logMessage = nowTimeString() + 'Received Invalidation with type ' +
            quote(inv.objectId.name) + ' version ' +
            quote((inv.isUnknownVersion ? 'Unknown' : inv.version)) +
            ' with payload ' + quote(inv.payload);

        appendToLog(logMessage);
        var isInvalidation = true;
        logToTable(inv, isInvalidation);
      }
    }
    repaintTable();
  }

  /**
   * Marks a change in the table whether a new invalidation has arrived
   * or a new ObjectId is currently being added or updated.
   * @param {!Object} oId The ObjectId being added or updated.
   * @param {!boolean} isInvaldation A flag that says that an invalidation
   *     for this ObjectId has arrived or we just need to add it to the table
   *     as it was just updated its state.
   */
  function logToTable(oId, isInvalidation) {
    var registrar = oId.registrar;
    var name = oId.objectId.name;
    var source = oId.objectId.source;
    var totalCount = oId.objectId.totalCount || 0;
    var key = source + '-' + name;
    var time = new Date();
    var version = oId.isUnknownVersion ? '?' : oId.version;
    var payload = '';
    if (oId.hasOwnProperty('payload'))
      payload = oId.payload;
    if (!(key in tableObjects)) {
      tableObjects[key] = {
        name: name,
        source: source,
        totalCount: totalCount,
        sessionCount: 0,
        registrar: registrar,
        time: '',
        version: '',
        payload: '',
        type: 'content'
      };
    }
    // Refresh the type to be a content because it might have been
    // greyed out.
    tableObjects[key].type = 'content';
    if (isInvalidation) {
      tableObjects[key].totalCount = tableObjects[key].totalCount + 1;
      tableObjects[key].sessionCount = tableObjects[key].sessionCount + 1;
      tableObjects[key].time = time.toTimeString();
      tableObjects[key].version = version;
      tableObjects[key].payload = payload;
    }
  }

  /**
   * Shows the handlers that are currently registered for invalidations
   * (but might not have objects ids registered yet).
   * @param {!Array<string>} allHandlers An array of Strings that are
   *     the names of all the handlers currently registered in the
   *     InvalidatorService.
   */
  function updateHandlers(allHandlers) {
    var allHandlersFormatted = allHandlers.join(', ');
    $('registered-handlers').textContent = allHandlersFormatted;
    var logMessage = nowTimeString() +
        'InvalidatorHandlers currently registered: ' + allHandlersFormatted;
    appendToLog(logMessage);
  }

  /**
   * Updates the table with the objects ids registered for invalidations
   * @param {string} registrar The name of the owner of the InvalidationHandler
   *     that is registered for invalidations
   * @param {Array of Object} allIds An array of ObjectsIds that are currently
   *     registered for invalidations. It is not differential (as in, whatever
   *     is not registered now but was before, it mean it was taken out the
   *     registered objects)
   */
  function updateIds(registrar, allIds) {
    // Grey out every datatype assigned to this registrar
    // (and reenable them later in case they are still registered).
    for (var key in tableObjects) {
      if (tableObjects[key]['registrar'] === registrar)
        tableObjects[key].type = 'greyed';
    }
    // Reenable those ObjectsIds still registered with this registrar.
    for (var i = 0; i < allIds.length; i++) {
      var oId = {objectId: allIds[i], registrar: registrar};
      var isInvalidation = false;
      logToTable(oId, isInvalidation);
    }
    repaintTable();
  }

  /**
   * Update the internal status display, merging new detailed information.
   * @param {!Object} newDetails The dictionary containing assorted debugging
   *      details (e.g. Network Channel information).
   */
  function updateDetailedStatus(newDetails) {
    for (var key in newDetails) {
      cachedDetails[key] = newDetails[key];
    }
    $('internal-display').value = JSON.stringify(cachedDetails, null, 2);
  }

  /**
   * Function that notifies the InvalidationsMessageHandler that the UI is
   * ready to receive real-time notifications.
   */
  function onLoadWork() {
    $('request-detailed-status').onclick = function() {
      cachedDetails = {};
      chrome.send('requestDetailedStatus');
    };
    chrome.send('doneLoading');
  }

  return {
    logInvalidations: logInvalidations,
    onLoadWork: onLoadWork,
    updateDetailedStatus: updateDetailedStatus,
    updateHandlers: updateHandlers,
    updateIds: updateIds,
    updateInvalidatorState: updateInvalidatorState,
  };
});

document.addEventListener('DOMContentLoaded', chrome.invalidations.onLoadWork);
