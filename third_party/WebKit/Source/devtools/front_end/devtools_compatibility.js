// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
/* eslint-disable indent */
(function(window) {

  // DevToolsAPI ----------------------------------------------------------------

  /**
   * @unrestricted
   */
  var DevToolsAPIImpl = class {
    constructor() {
      /**
       * @type {number}
       */
      this._lastCallId = 0;

      /**
       * @type {!Object.<number, function(?Object)>}
       */
      this._callbacks = {};
    }

    /**
     * @param {number} id
     * @param {?Object} arg
     */
    embedderMessageAck(id, arg) {
      var callback = this._callbacks[id];
      delete this._callbacks[id];
      if (callback)
        callback(arg);
    }

    /**
     * @param {string} method
     * @param {!Array.<*>} args
     * @param {?function(?Object)} callback
     */
    sendMessageToEmbedder(method, args, callback) {
      var callId = ++this._lastCallId;
      if (callback)
        this._callbacks[callId] = callback;
      var message = {'id': callId, 'method': method};
      if (args.length)
        message.params = args;
      DevToolsHost.sendMessageToEmbedder(JSON.stringify(message));
    }

    /**
     * @param {string} method
     * @param {!Array<*>} args
     */
    _dispatchOnInspectorFrontendAPI(method, args) {
      const inspectorFrontendAPI = /** @type {!Object<string, function()>} */ (window['InspectorFrontendAPI']);
      inspectorFrontendAPI[method].apply(inspectorFrontendAPI, args);
    }

    // API methods below this line --------------------------------------------

    /**
     * @param {!Array.<!ExtensionDescriptor>} extensions
     */
    addExtensions(extensions) {
      // Support for legacy front-ends (<M41).
      if (window['WebInspector'] && window['WebInspector']['addExtensions']) {
        window['WebInspector']['addExtensions'](extensions);
      } else if (window['InspectorFrontendAPI']) {
        // The addExtensions command is sent as the onload event happens for
        // DevTools front-end. In case of hosted mode, this
        // happens before the InspectorFrontendAPI is initialized.
        this._dispatchOnInspectorFrontendAPI('addExtensions', [extensions]);
      }
    }

    /**
     * @param {string} url
     */
    appendedToURL(url) {
      this._dispatchOnInspectorFrontendAPI('appendedToURL', [url]);
    }

    /**
     * @param {string} url
     */
    canceledSaveURL(url) {
      this._dispatchOnInspectorFrontendAPI('canceledSaveURL', [url]);
    }

    contextMenuCleared() {
      this._dispatchOnInspectorFrontendAPI('contextMenuCleared', []);
    }

    /**
     * @param {string} id
     */
    contextMenuItemSelected(id) {
      this._dispatchOnInspectorFrontendAPI('contextMenuItemSelected', [id]);
    }

    /**
     * @param {number} count
     */
    deviceCountUpdated(count) {
      this._dispatchOnInspectorFrontendAPI('deviceCountUpdated', [count]);
    }

    /**
     * @param {!Adb.Config} config
     */
    devicesDiscoveryConfigChanged(config) {
      this._dispatchOnInspectorFrontendAPI('devicesDiscoveryConfigChanged', [config]);
    }

    /**
     * @param {!Adb.PortForwardingStatus} status
     */
    devicesPortForwardingStatusChanged(status) {
      this._dispatchOnInspectorFrontendAPI('devicesPortForwardingStatusChanged', [status]);
    }

    /**
     * @param {!Array.<!Adb.Device>} devices
     */
    devicesUpdated(devices) {
      this._dispatchOnInspectorFrontendAPI('devicesUpdated', [devices]);
    }

    /**
     * @param {string} message
     */
    dispatchMessage(message) {
      this._dispatchOnInspectorFrontendAPI('dispatchMessage', [message]);
    }

    /**
     * @param {string} messageChunk
     * @param {number} messageSize
     */
    dispatchMessageChunk(messageChunk, messageSize) {
      this._dispatchOnInspectorFrontendAPI('dispatchMessageChunk', [messageChunk, messageSize]);
    }

    enterInspectElementMode() {
      this._dispatchOnInspectorFrontendAPI('enterInspectElementMode', []);
    }

    /**
     * @param {number} callId
     * @param {string} script
     */
    evaluateForTestInFrontend(callId, script) {
      this._dispatchOnInspectorFrontendAPI('evaluateForTestInFrontend', [callId, script]);
    }

    /**
     * @param {!{r: number, g: number, b: number, a: number}} color
     */
    eyeDropperPickedColor(color) {
      this._dispatchOnInspectorFrontendAPI('eyeDropperPickedColor', [color]);
    }

    /**
     * @param {!Array.<!{fileSystemName: string, rootURL: string, fileSystemPath: string}>} fileSystems
     */
    fileSystemsLoaded(fileSystems) {
      this._dispatchOnInspectorFrontendAPI('fileSystemsLoaded', [fileSystems]);
    }

    /**
     * @param {string} fileSystemPath
     */
    fileSystemRemoved(fileSystemPath) {
      this._dispatchOnInspectorFrontendAPI('fileSystemRemoved', [fileSystemPath]);
    }

    /**
     * @param {!{fileSystemName: string, rootURL: string, fileSystemPath: string}} fileSystem
     */
    fileSystemAdded(fileSystem) {
      this._dispatchOnInspectorFrontendAPI('fileSystemAdded', ['', fileSystem]);
    }

    /**
     * @param {!Array<string>} changedPaths
     * @param {!Array<string>} addedPaths
     * @param {!Array<string>} removedPaths
     */
    fileSystemFilesChangedAddedRemoved(changedPaths, addedPaths, removedPaths) {
      // Support for legacy front-ends (<M58)
      if (window['InspectorFrontendAPI'] && window['InspectorFrontendAPI']['fileSystemFilesChanged']) {
        this._dispatchOnInspectorFrontendAPI(
            'fileSystemFilesChanged', [changedPaths.concat(addedPaths).concat(removedPaths)]);
      } else {
        this._dispatchOnInspectorFrontendAPI(
            'fileSystemFilesChangedAddedRemoved', [changedPaths, addedPaths, removedPaths]);
      }
    }

    /**
     * @param {number} requestId
     * @param {string} fileSystemPath
     * @param {number} totalWork
     */
    indexingTotalWorkCalculated(requestId, fileSystemPath, totalWork) {
      this._dispatchOnInspectorFrontendAPI('indexingTotalWorkCalculated', [requestId, fileSystemPath, totalWork]);
    }

    /**
     * @param {number} requestId
     * @param {string} fileSystemPath
     * @param {number} worked
     */
    indexingWorked(requestId, fileSystemPath, worked) {
      this._dispatchOnInspectorFrontendAPI('indexingWorked', [requestId, fileSystemPath, worked]);
    }

    /**
     * @param {number} requestId
     * @param {string} fileSystemPath
     */
    indexingDone(requestId, fileSystemPath) {
      this._dispatchOnInspectorFrontendAPI('indexingDone', [requestId, fileSystemPath]);
    }

    /**
     * @param {{type: string, key: string, code: string, keyCode: number, modifiers: number}} event
     */
    keyEventUnhandled(event) {
      event.keyIdentifier = keyCodeToKeyIdentifier(event.keyCode);
      this._dispatchOnInspectorFrontendAPI('keyEventUnhandled', [event]);
    }

    /**
     * @param {boolean} hard
     */
    reloadInspectedPage(hard) {
      this._dispatchOnInspectorFrontendAPI('reloadInspectedPage', [hard]);
    }

    /**
     * @param {string} url
     * @param {number} lineNumber
     * @param {number} columnNumber
     */
    revealSourceLine(url, lineNumber, columnNumber) {
      this._dispatchOnInspectorFrontendAPI('revealSourceLine', [url, lineNumber, columnNumber]);
    }

    /**
     * @param {string} url
     */
    savedURL(url) {
      this._dispatchOnInspectorFrontendAPI('savedURL', [url]);
    }

    /**
     * @param {number} requestId
     * @param {string} fileSystemPath
     * @param {!Array.<string>} files
     */
    searchCompleted(requestId, fileSystemPath, files) {
      this._dispatchOnInspectorFrontendAPI('searchCompleted', [requestId, fileSystemPath, files]);
    }

    /**
     * @param {string} tabId
     */
    setInspectedTabId(tabId) {
      // Support for legacy front-ends (<M41).
      if (window['WebInspector'] && window['WebInspector']['setInspectedTabId'])
        window['WebInspector']['setInspectedTabId'](tabId);
      else
        this._dispatchOnInspectorFrontendAPI('setInspectedTabId', [tabId]);
    }

    /**
     * @param {boolean} useSoftMenu
     */
    setUseSoftMenu(useSoftMenu) {
      this._dispatchOnInspectorFrontendAPI('setUseSoftMenu', [useSoftMenu]);
    }

    /**
     * @param {string} panelName
     */
    showPanel(panelName) {
      this._dispatchOnInspectorFrontendAPI('showPanel', [panelName]);
    }

    /**
     * @param {number} id
     * @param {string} chunk
     * @param {boolean} encoded
     */
    streamWrite(id, chunk, encoded) {
      this._dispatchOnInspectorFrontendAPI('streamWrite', [id, encoded ? this._decodeBase64(chunk) : chunk]);
    }

    /**
     * @param {string} chunk
     * @return {string}
     */
    _decodeBase64(chunk) {
      var request = new XMLHttpRequest();
      request.open('GET', 'data:text/plain;base64,' + chunk, false);
      request.send(null);
      if (request.status === 200) {
        return request.responseText;
      } else {
        console.error('Error while decoding chunk in streamWrite');
        return '';
      }
    }
  };

  var DevToolsAPI = new DevToolsAPIImpl();
  window.DevToolsAPI = DevToolsAPI;

  // InspectorFrontendHostImpl --------------------------------------------------

  /**
   * @implements {InspectorFrontendHostAPI}
   * @unrestricted
   */
  var InspectorFrontendHostImpl = class {
    /**
     * @override
     * @return {string}
     */
    getSelectionBackgroundColor() {
      return DevToolsHost.getSelectionBackgroundColor();
    }

    /**
     * @override
     * @return {string}
     */
    getSelectionForegroundColor() {
      return DevToolsHost.getSelectionForegroundColor();
    }

    /**
     * @override
     * @return {string}
     */
    platform() {
      return DevToolsHost.platform();
    }

    /**
     * @override
     */
    loadCompleted() {
      DevToolsAPI.sendMessageToEmbedder('loadCompleted', [], null);
      // Support for legacy (<57) frontends.
      if (window.Runtime && window.Runtime.queryParam) {
        var panelToOpen = window.Runtime.queryParam('panel');
        if (panelToOpen)
          window.DevToolsAPI.showPanel(panelToOpen);
      }
    }

    /**
     * @override
     */
    bringToFront() {
      DevToolsAPI.sendMessageToEmbedder('bringToFront', [], null);
    }

    /**
     * @override
     */
    closeWindow() {
      DevToolsAPI.sendMessageToEmbedder('closeWindow', [], null);
    }

    /**
     * @override
     * @param {boolean} isDocked
     * @param {function()} callback
     */
    setIsDocked(isDocked, callback) {
      DevToolsAPI.sendMessageToEmbedder('setIsDocked', [isDocked], callback);
    }

    /**
     * Requests inspected page to be placed atop of the inspector frontend with specified bounds.
     * @override
     * @param {{x: number, y: number, width: number, height: number}} bounds
     */
    setInspectedPageBounds(bounds) {
      DevToolsAPI.sendMessageToEmbedder('setInspectedPageBounds', [bounds], null);
    }

    /**
     * @override
     */
    inspectElementCompleted() {
      DevToolsAPI.sendMessageToEmbedder('inspectElementCompleted', [], null);
    }

    /**
     * @override
     * @param {string} url
     * @param {string} headers
     * @param {number} streamId
     * @param {function(!InspectorFrontendHostAPI.LoadNetworkResourceResult)} callback
     */
    loadNetworkResource(url, headers, streamId, callback) {
      DevToolsAPI.sendMessageToEmbedder(
          'loadNetworkResource', [url, headers, streamId], /** @type {function(?Object)} */ (callback));
    }

    /**
     * @override
     * @param {function(!Object<string, string>)} callback
     */
    getPreferences(callback) {
      DevToolsAPI.sendMessageToEmbedder('getPreferences', [], /** @type {function(?Object)} */ (callback));
    }

    /**
     * @override
     * @param {string} name
     * @param {string} value
     */
    setPreference(name, value) {
      DevToolsAPI.sendMessageToEmbedder('setPreference', [name, value], null);
    }

    /**
     * @override
     * @param {string} name
     */
    removePreference(name) {
      DevToolsAPI.sendMessageToEmbedder('removePreference', [name], null);
    }

    /**
     * @override
     */
    clearPreferences() {
      DevToolsAPI.sendMessageToEmbedder('clearPreferences', [], null);
    }

    /**
     * @override
     * @param {string} origin
     * @param {string} script
     */
    setInjectedScriptForOrigin(origin, script) {
      DevToolsAPI.sendMessageToEmbedder('registerExtensionsAPI', [origin, script], null);
    }

    /**
     * @override
     * @param {string} url
     */
    inspectedURLChanged(url) {
      DevToolsAPI.sendMessageToEmbedder('inspectedURLChanged', [url], null);
    }

    /**
     * @override
     * @param {string} text
     */
    copyText(text) {
      DevToolsHost.copyText(text);
    }

    /**
     * @override
     * @param {string} url
     */
    openInNewTab(url) {
      DevToolsAPI.sendMessageToEmbedder('openInNewTab', [url], null);
    }

    /**
     * @override
     * @param {string} url
     * @param {string} content
     * @param {boolean} forceSaveAs
     */
    save(url, content, forceSaveAs) {
      DevToolsAPI.sendMessageToEmbedder('save', [url, content, forceSaveAs], null);
    }

    /**
     * @override
     * @param {string} url
     * @param {string} content
     */
    append(url, content) {
      DevToolsAPI.sendMessageToEmbedder('append', [url, content], null);
    }

    /**
     * @override
     * @param {string} message
     */
    sendMessageToBackend(message) {
      DevToolsAPI.sendMessageToEmbedder('dispatchProtocolMessage', [message], null);
    }

    /**
     * @override
     * @param {string} actionName
     * @param {number} actionCode
     * @param {number} bucketSize
     */
    recordEnumeratedHistogram(actionName, actionCode, bucketSize) {
      // Support for M49 frontend.
      if (actionName === 'DevTools.DrawerShown')
        return;
      DevToolsAPI.sendMessageToEmbedder('recordEnumeratedHistogram', [actionName, actionCode, bucketSize], null);
    }

    /**
     * @override
     */
    requestFileSystems() {
      DevToolsAPI.sendMessageToEmbedder('requestFileSystems', [], null);
    }

    /**
     * @override
     * @param {string=} fileSystemPath
     */
    addFileSystem(fileSystemPath) {
      DevToolsAPI.sendMessageToEmbedder('addFileSystem', [fileSystemPath || ''], null);
    }

    /**
     * @override
     * @param {string} fileSystemPath
     */
    removeFileSystem(fileSystemPath) {
      DevToolsAPI.sendMessageToEmbedder('removeFileSystem', [fileSystemPath], null);
    }

    /**
     * @override
     * @param {string} fileSystemId
     * @param {string} registeredName
     * @return {?DOMFileSystem}
     */
    isolatedFileSystem(fileSystemId, registeredName) {
      return DevToolsHost.isolatedFileSystem(fileSystemId, registeredName);
    }

    /**
     * @override
     * @param {!FileSystem} fileSystem
     */
    upgradeDraggedFileSystemPermissions(fileSystem) {
      DevToolsHost.upgradeDraggedFileSystemPermissions(fileSystem);
    }

    /**
     * @override
     * @param {number} requestId
     * @param {string} fileSystemPath
     */
    indexPath(requestId, fileSystemPath) {
      DevToolsAPI.sendMessageToEmbedder('indexPath', [requestId, fileSystemPath], null);
    }

    /**
     * @override
     * @param {number} requestId
     */
    stopIndexing(requestId) {
      DevToolsAPI.sendMessageToEmbedder('stopIndexing', [requestId], null);
    }

    /**
     * @override
     * @param {number} requestId
     * @param {string} fileSystemPath
     * @param {string} query
     */
    searchInPath(requestId, fileSystemPath, query) {
      DevToolsAPI.sendMessageToEmbedder('searchInPath', [requestId, fileSystemPath, query], null);
    }

    /**
     * @override
     * @return {number}
     */
    zoomFactor() {
      return DevToolsHost.zoomFactor();
    }

    /**
     * @override
     */
    zoomIn() {
      DevToolsAPI.sendMessageToEmbedder('zoomIn', [], null);
    }

    /**
     * @override
     */
    zoomOut() {
      DevToolsAPI.sendMessageToEmbedder('zoomOut', [], null);
    }

    /**
     * @override
     */
    resetZoom() {
      DevToolsAPI.sendMessageToEmbedder('resetZoom', [], null);
    }

    /**
     * @override
     * @param {string} shortcuts
     */
    setWhitelistedShortcuts(shortcuts) {
      DevToolsAPI.sendMessageToEmbedder('setWhitelistedShortcuts', [shortcuts], null);
    }

    /**
     * @override
     * @param {boolean} active
     */
    setEyeDropperActive(active) {
      DevToolsAPI.sendMessageToEmbedder('setEyeDropperActive', [active], null);
    }

    /**
     * @override
     * @param {!Array<string>} certChain
     */
    showCertificateViewer(certChain) {
      DevToolsAPI.sendMessageToEmbedder('showCertificateViewer', [JSON.stringify(certChain)], null);
    }

    /**
     * @override
     * @return {boolean}
     */
    isUnderTest() {
      return DevToolsHost.isUnderTest();
    }

    /**
     * @override
     * @param {function()} callback
     */
    reattach(callback) {
      DevToolsAPI.sendMessageToEmbedder('reattach', [], callback);
    }

    /**
     * @override
     */
    readyForTest() {
      DevToolsAPI.sendMessageToEmbedder('readyForTest', [], null);
    }

    /**
     * @override
     * @param {!Adb.Config} config
     */
    setDevicesDiscoveryConfig(config) {
      DevToolsAPI.sendMessageToEmbedder(
          'setDevicesDiscoveryConfig',
          [
            config.discoverUsbDevices, config.portForwardingEnabled, JSON.stringify(config.portForwardingConfig),
            config.networkDiscoveryEnabled, JSON.stringify(config.networkDiscoveryConfig)
          ],
          null);
    }

    /**
     * @override
     * @param {boolean} enabled
     */
    setDevicesUpdatesEnabled(enabled) {
      DevToolsAPI.sendMessageToEmbedder('setDevicesUpdatesEnabled', [enabled], null);
    }

    /**
     * @override
     * @param {string} pageId
     * @param {string} action
     */
    performActionOnRemotePage(pageId, action) {
      DevToolsAPI.sendMessageToEmbedder('performActionOnRemotePage', [pageId, action], null);
    }

    /**
     * @override
     * @param {string} browserId
     * @param {string} url
     */
    openRemotePage(browserId, url) {
      DevToolsAPI.sendMessageToEmbedder('openRemotePage', [browserId, url], null);
    }

    /**
     * @override
     */
    openNodeFrontend() {
      DevToolsAPI.sendMessageToEmbedder('openNodeFrontend', [], null);
    }

    /**
     * @override
     * @param {number} x
     * @param {number} y
     * @param {!Array.<!InspectorFrontendHostAPI.ContextMenuDescriptor>} items
     * @param {!Document} document
     */
    showContextMenuAtPoint(x, y, items, document) {
      DevToolsHost.showContextMenuAtPoint(x, y, items, document);
    }

    /**
     * @override
     * @return {boolean}
     */
    isHostedMode() {
      return DevToolsHost.isHostedMode();
    }

    // Backward-compatible methods below this line --------------------------------------------

    /**
     * Support for legacy front-ends (<M50).
     * @param {string} message
     */
    sendFrontendAPINotification(message) {
    }

    /**
     * Support for legacy front-ends (<M41).
     * @return {string}
     */
    port() {
      return 'unknown';
    }

    /**
     * Support for legacy front-ends (<M38).
     * @param {number} zoomFactor
     */
    setZoomFactor(zoomFactor) {
    }

    /**
     * Support for legacy front-ends (<M34).
     */
    sendMessageToEmbedder() {
    }

    /**
     * Support for legacy front-ends (<M34).
     * @param {string} dockSide
     */
    requestSetDockSide(dockSide) {
      DevToolsAPI.sendMessageToEmbedder('setIsDocked', [dockSide !== 'undocked'], null);
    }

    /**
     * Support for legacy front-ends (<M34).
     * @return {boolean}
     */
    supportsFileSystems() {
      return true;
    }

    /**
     * Support for legacy front-ends (<M28).
     * @return {boolean}
     */
    canInspectWorkers() {
      return true;
    }

    /**
     * Support for legacy front-ends (<M28).
     * @return {boolean}
     */
    canSaveAs() {
      return true;
    }

    /**
     * Support for legacy front-ends (<M28).
     * @return {boolean}
     */
    canSave() {
      return true;
    }

    /**
     * Support for legacy front-ends (<M28).
     */
    loaded() {
    }

    /**
     * Support for legacy front-ends (<M28).
     * @return {string}
     */
    hiddenPanels() {
      return '';
    }

    /**
     * Support for legacy front-ends (<M28).
     * @return {string}
     */
    localizedStringsURL() {
      return '';
    }

    /**
     * Support for legacy front-ends (<M28).
     * @param {string} url
     */
    close(url) {
    }

    /**
     * Support for legacy front-ends (<M44).
     * @param {number} actionCode
     */
    recordActionTaken(actionCode) {
      this.recordEnumeratedHistogram('DevTools.ActionTaken', actionCode, 100);
    }

    /**
     * Support for legacy front-ends (<M44).
     * @param {number} panelCode
     */
    recordPanelShown(panelCode) {
      this.recordEnumeratedHistogram('DevTools.PanelShown', panelCode, 20);
    }
  };

  window.InspectorFrontendHost = new InspectorFrontendHostImpl();

  // DevToolsApp ---------------------------------------------------------------

  function installObjectObserve() {
    /** @type {!Array<string>} */
    var properties = [
      'advancedSearchConfig',
      'auditsPanelSplitViewState',
      'auditsSidebarWidth',
      'blockedURLs',
      'breakpoints',
      'cacheDisabled',
      'colorFormat',
      'consoleHistory',
      'consoleTimestampsEnabled',
      'cpuProfilerView',
      'cssSourceMapsEnabled',
      'currentDockState',
      'customColorPalette',
      'customDevicePresets',
      'customEmulatedDeviceList',
      'customFormatters',
      'customUserAgent',
      'databaseTableViewVisibleColumns',
      'dataGrid-cookiesTable',
      'dataGrid-DOMStorageItemsView',
      'debuggerSidebarHidden',
      'disableDataSaverInfobar',
      'disablePausedStateOverlay',
      'domBreakpoints',
      'domWordWrap',
      'elementsPanelSplitViewState',
      'elementsSidebarWidth',
      'emulation.deviceHeight',
      'emulation.deviceModeValue',
      'emulation.deviceOrientationOverride',
      'emulation.deviceScale',
      'emulation.deviceScaleFactor',
      'emulation.deviceUA',
      'emulation.deviceWidth',
      'emulation.geolocationOverride',
      'emulation.showDeviceMode',
      'emulation.showRulers',
      'enableAsyncStackTraces',
      'eventListenerBreakpoints',
      'fileMappingEntries',
      'fileSystemMapping',
      'FileSystemViewSidebarWidth',
      'fileSystemViewSplitViewState',
      'filterBar-consoleView',
      'filterBar-networkPanel',
      'filterBar-promisePane',
      'filterBar-timelinePanel',
      'frameViewerHideChromeWindow',
      'heapSnapshotRetainersViewSize',
      'heapSnapshotSplitViewState',
      'hideCollectedPromises',
      'hideNetworkMessages',
      'highlightNodeOnHoverInOverlay',
      'highResolutionCpuProfiling',
      'inlineVariableValues',
      'Inspector.drawerSplitView',
      'Inspector.drawerSplitViewState',
      'InspectorView.panelOrder',
      'InspectorView.screencastSplitView',
      'InspectorView.screencastSplitViewState',
      'InspectorView.splitView',
      'InspectorView.splitViewState',
      'javaScriptDisabled',
      'jsSourceMapsEnabled',
      'lastActivePanel',
      'lastDockState',
      'lastSelectedSourcesSidebarPaneTab',
      'lastSnippetEvaluationIndex',
      'layerDetailsSplitView',
      'layerDetailsSplitViewState',
      'layersPanelSplitViewState',
      'layersShowInternalLayers',
      'layersSidebarWidth',
      'messageLevelFilters',
      'messageURLFilters',
      'monitoringXHREnabled',
      'navigatorGroupByFolder',
      'navigatorHidden',
      'networkColorCodeResourceTypes',
      'networkConditions',
      'networkConditionsCustomProfiles',
      'networkHideDataURL',
      'networkLogColumnsVisibility',
      'networkLogLargeRows',
      'networkLogShowOverview',
      'networkPanelSplitViewState',
      'networkRecordFilmStripSetting',
      'networkResourceTypeFilters',
      'networkShowPrimaryLoadWaterfall',
      'networkSidebarWidth',
      'openLinkHandler',
      'pauseOnCaughtException',
      'pauseOnExceptionEnabled',
      'preserveConsoleLog',
      'prettyPrintInfobarDisabled',
      'previouslyViewedFiles',
      'profilesPanelSplitViewState',
      'profilesSidebarWidth',
      'promiseStatusFilters',
      'recordAllocationStacks',
      'requestHeaderFilterSetting',
      'request-info-formData-category-expanded',
      'request-info-general-category-expanded',
      'request-info-queryString-category-expanded',
      'request-info-requestHeaders-category-expanded',
      'request-info-requestPayload-category-expanded',
      'request-info-responseHeaders-category-expanded',
      'resources',
      'resourcesLastSelectedItem',
      'resourcesPanelSplitViewState',
      'resourcesSidebarWidth',
      'resourceViewTab',
      'savedURLs',
      'screencastEnabled',
      'scriptsPanelNavigatorSidebarWidth',
      'searchInContentScripts',
      'selectedAuditCategories',
      'selectedColorPalette',
      'selectedProfileType',
      'shortcutPanelSwitch',
      'showAdvancedHeapSnapshotProperties',
      'showEventListenersForAncestors',
      'showFrameowkrListeners',
      'showHeaSnapshotObjectsHiddenProperties',
      'showInheritedComputedStyleProperties',
      'showMediaQueryInspector',
      'showNativeFunctionsInJSProfile',
      'showUAShadowDOM',
      'showWhitespacesInEditor',
      'sidebarPosition',
      'skipContentScripts',
      'skipStackFramesPattern',
      'sourceMapInfobarDisabled',
      'sourcesPanelDebuggerSidebarSplitViewState',
      'sourcesPanelNavigatorSplitViewState',
      'sourcesPanelSplitSidebarRatio',
      'sourcesPanelSplitViewState',
      'sourcesSidebarWidth',
      'standardEmulatedDeviceList',
      'StylesPaneSplitRatio',
      'stylesPaneSplitViewState',
      'textEditorAutocompletion',
      'textEditorAutoDetectIndent',
      'textEditorBracketMatching',
      'textEditorIndent',
      'timelineCaptureFilmStrip',
      'timelineCaptureLayersAndPictures',
      'timelineCaptureMemory',
      'timelineCaptureNetwork',
      'timeline-details',
      'timelineEnableJSSampling',
      'timelineOverviewMode',
      'timelinePanelDetailsSplitViewState',
      'timelinePanelRecorsSplitViewState',
      'timelinePanelTimelineStackSplitViewState',
      'timelinePerspective',
      'timeline-split',
      'timelineTreeGroupBy',
      'timeline-view',
      'timelineViewMode',
      'uiTheme',
      'watchExpressions',
      'WebInspector.Drawer.lastSelectedView',
      'WebInspector.Drawer.showOnLoad',
      'workspaceExcludedFolders',
      'workspaceFolderExcludePattern',
      'workspaceInfobarDisabled',
      'workspaceMappingInfobarDisabled',
      'xhrBreakpoints'
    ];

    /**
     * @this {!{_storage: Object, _name: string}}
     */
    function settingRemove() {
      this._storage[this._name] = undefined;
    }

    /**
     * @param {!Object} object
     * @param {function(!Array<!{name: string}>)} observer
     */
    function objectObserve(object, observer) {
      if (window['WebInspector']) {
        var settingPrototype = /** @type {!Object} */ (window['WebInspector']['Setting']['prototype']);
        if (typeof settingPrototype['remove'] === 'function')
          settingPrototype['remove'] = settingRemove;
      }
      /** @type {!Set<string>} */
      var changedProperties = new Set();
      var scheduled = false;

      function scheduleObserver() {
        if (scheduled)
          return;
        scheduled = true;
        setImmediate(callObserver);
      }

      function callObserver() {
        scheduled = false;
        var changes = /** @type {!Array<!{name: string}>} */ ([]);
        changedProperties.forEach(function(name) {
          changes.push({name: name});
        });
        changedProperties.clear();
        observer.call(null, changes);
      }

      /** @type {!Map<string, *>} */
      var storage = new Map();

      /**
       * @param {string} property
       */
      function defineProperty(property) {
        if (property in object) {
          storage.set(property, object[property]);
          delete object[property];
        }

        Object.defineProperty(object, property, {
          /**
           * @return {*}
           */
          get: function() {
            return storage.get(property);
          },

          /**
           * @param {*} value
           */
          set: function(value) {
            storage.set(property, value);
            changedProperties.add(property);
            scheduleObserver();
          }
        });
      }

      for (var i = 0; i < properties.length; ++i)
        defineProperty(properties[i]);
    }

    window.Object.observe = objectObserve;
  }

  /** @type {!Map<number, string>} */
  var staticKeyIdentifiers = new Map([
    [0x12, 'Alt'],
    [0x11, 'Control'],
    [0x10, 'Shift'],
    [0x14, 'CapsLock'],
    [0x5b, 'Win'],
    [0x5c, 'Win'],
    [0x0c, 'Clear'],
    [0x28, 'Down'],
    [0x23, 'End'],
    [0x0a, 'Enter'],
    [0x0d, 'Enter'],
    [0x2b, 'Execute'],
    [0x70, 'F1'],
    [0x71, 'F2'],
    [0x72, 'F3'],
    [0x73, 'F4'],
    [0x74, 'F5'],
    [0x75, 'F6'],
    [0x76, 'F7'],
    [0x77, 'F8'],
    [0x78, 'F9'],
    [0x79, 'F10'],
    [0x7a, 'F11'],
    [0x7b, 'F12'],
    [0x7c, 'F13'],
    [0x7d, 'F14'],
    [0x7e, 'F15'],
    [0x7f, 'F16'],
    [0x80, 'F17'],
    [0x81, 'F18'],
    [0x82, 'F19'],
    [0x83, 'F20'],
    [0x84, 'F21'],
    [0x85, 'F22'],
    [0x86, 'F23'],
    [0x87, 'F24'],
    [0x2f, 'Help'],
    [0x24, 'Home'],
    [0x2d, 'Insert'],
    [0x25, 'Left'],
    [0x22, 'PageDown'],
    [0x21, 'PageUp'],
    [0x13, 'Pause'],
    [0x2c, 'PrintScreen'],
    [0x27, 'Right'],
    [0x91, 'Scroll'],
    [0x29, 'Select'],
    [0x26, 'Up'],
    [0x2e, 'U+007F'],  // Standard says that DEL becomes U+007F.
    [0xb0, 'MediaNextTrack'],
    [0xb1, 'MediaPreviousTrack'],
    [0xb2, 'MediaStop'],
    [0xb3, 'MediaPlayPause'],
    [0xad, 'VolumeMute'],
    [0xae, 'VolumeDown'],
    [0xaf, 'VolumeUp'],
  ]);

  /**
   * @param {number} keyCode
   * @return {string}
   */
  function keyCodeToKeyIdentifier(keyCode) {
    var result = staticKeyIdentifiers.get(keyCode);
    if (result !== undefined)
      return result;
    result = 'U+';
    var hexString = keyCode.toString(16).toUpperCase();
    for (var i = hexString.length; i < 4; ++i)
      result += '0';
    result += hexString;
    return result;
  }

  function installBackwardsCompatibility() {
    if (window.location.search.indexOf('remoteFrontend') === -1)
      return;

    // Support for legacy (<M53) frontends.
    if (!window.KeyboardEvent.prototype.hasOwnProperty('keyIdentifier')) {
      Object.defineProperty(window.KeyboardEvent.prototype, 'keyIdentifier', {
        /**
         * @return {string}
         * @this {KeyboardEvent}
         */
        get: function() {
          return keyCodeToKeyIdentifier(this.keyCode);
        }
      });
    }

    // Support for legacy (<M50) frontends.
    installObjectObserve();

    /**
     * @param {string} property
     * @return {!CSSValue|null}
     * @this {CSSStyleDeclaration}
     */
    function getValue(property) {
      // Note that |property| comes from another context, so we can't use === here.
      // eslint-disable-next-line eqeqeq
      if (property == 'padding-left') {
        return /** @type {!CSSValue} */ ({
          /**
           * @return {number}
           * @this {!{__paddingLeft: number}}
           */
          getFloatValue: function() {
            return this.__paddingLeft;
          },
          __paddingLeft: parseFloat(this.paddingLeft)
        });
      }
      throw new Error('getPropertyCSSValue is undefined');
    }

    // Support for legacy (<M41) frontends.
    window.CSSStyleDeclaration.prototype.getPropertyCSSValue = getValue;

    function CSSPrimitiveValue() {
    }
    CSSPrimitiveValue.CSS_PX = 5;
    window.CSSPrimitiveValue = CSSPrimitiveValue;

    // Support for legacy (<M44) frontends.
    var styleElement = window.document.createElement('style');
    styleElement.type = 'text/css';
    styleElement.textContent = 'html /deep/ * { min-width: 0; min-height: 0; }';

    // Support for quirky border-image behavior (<M51), see:
    // https://bugs.chromium.org/p/chromium/issues/detail?id=559258
    styleElement.textContent +=
        '\nhtml /deep/ .cm-breakpoint .CodeMirror-linenumber { border-style: solid !important; }';
    styleElement.textContent +=
        '\nhtml /deep/ .cm-breakpoint.cm-breakpoint-conditional .CodeMirror-linenumber { border-style: solid !important; }';
    window.document.head.appendChild(styleElement);

    // Support for legacy (<M49) frontends.
    Event.prototype.deepPath = undefined;

    // Support for legacy (<53) frontends.
    window.FileError = /** @type {!function (new: FileError) : ?} */ ({
      NOT_FOUND_ERR: DOMException.NOT_FOUND_ERR,
      ABORT_ERR: DOMException.ABORT_ERR,
      INVALID_MODIFICATION_ERR: DOMException.INVALID_MODIFICATION_ERR,
      NOT_READABLE_ERR: 0  // No matching DOMException, so code will be 0.
    });
  }

  function windowLoaded() {
    window.removeEventListener('DOMContentLoaded', windowLoaded, false);
    installBackwardsCompatibility();
  }

  if (window.document.head &&
      (window.document.readyState === 'complete' || window.document.readyState === 'interactive'))
    installBackwardsCompatibility();
  else
    window.addEventListener('DOMContentLoaded', windowLoaded, false);

  /** @type {(!function(string, boolean=):boolean)|undefined} */
  DOMTokenList.prototype.__originalDOMTokenListToggle;

  if (!DOMTokenList.prototype.__originalDOMTokenListToggle) {
    DOMTokenList.prototype.__originalDOMTokenListToggle = DOMTokenList.prototype.toggle;
    /**
     * @param {string} token
     * @param {boolean=} force
     * @return {boolean}
     */
    DOMTokenList.prototype.toggle = function(token, force) {
      if (arguments.length === 1)
        force = !this.contains(token);
      return this.__originalDOMTokenListToggle(token, !!force);
    };
  }

})(window);
