/*
 * Copyright (C) 2011 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY GOOGLE INC. AND ITS CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL GOOGLE INC.
 * OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * @interface
 */
Sources.TabbedEditorContainerDelegate = function() {};

Sources.TabbedEditorContainerDelegate.prototype = {
  /**
   * @param {!Workspace.UISourceCode} uiSourceCode
   * @return {!UI.Widget}
   */
  viewForFile(uiSourceCode) {},
};

/**
 * @unrestricted
 */
Sources.TabbedEditorContainer = class extends Common.Object {
  /**
   * @param {!Sources.TabbedEditorContainerDelegate} delegate
   * @param {!Common.Setting} setting
   * @param {!Element} placeholderElement
   */
  constructor(delegate, setting, placeholderElement) {
    super();
    this._delegate = delegate;

    this._tabbedPane = new UI.TabbedPane();
    this._tabbedPane.setPlaceholderElement(placeholderElement);
    this._tabbedPane.setTabDelegate(new Sources.EditorContainerTabDelegate(this));

    this._tabbedPane.setCloseableTabs(true);
    this._tabbedPane.setAllowTabReorder(true, true);

    this._tabbedPane.addEventListener(UI.TabbedPane.Events.TabClosed, this._tabClosed, this);
    this._tabbedPane.addEventListener(UI.TabbedPane.Events.TabSelected, this._tabSelected, this);

    Persistence.persistence.addEventListener(
        Persistence.Persistence.Events.BindingCreated, this._onBindingCreated, this);
    Persistence.persistence.addEventListener(
        Persistence.Persistence.Events.BindingRemoved, this._onBindingRemoved, this);

    this._tabIds = new Map();
    this._files = {};

    this._previouslyViewedFilesSetting = setting;
    this._history = Sources.TabbedEditorContainer.History.fromObject(this._previouslyViewedFilesSetting.get());
  }

  /**
   * @param {!Common.Event} event
   */
  _onBindingCreated(event) {
    var binding = /** @type {!Persistence.PersistenceBinding} */ (event.data);
    this._updateFileTitle(binding.fileSystem);

    var networkTabId = this._tabIds.get(binding.network);
    var fileSystemTabId = this._tabIds.get(binding.fileSystem);

    var wasSelectedInNetwork = this._currentFile === binding.network;
    var currentSelectionRange = this._history.selectionRange(binding.network.url());
    var currentScrollLineNumber = this._history.scrollLineNumber(binding.network.url());
    this._history.remove(binding.network.url());

    if (!networkTabId)
      return;

    if (!fileSystemTabId) {
      var tabIndex = this._tabbedPane.tabIndex(networkTabId);
      fileSystemTabId = this._appendFileTab(binding.fileSystem, false, tabIndex);
      var fileSystemTabView = /** @type {!UI.Widget} */ (this._tabbedPane.tabView(fileSystemTabId));
      this._restoreEditorProperties(fileSystemTabView, currentSelectionRange, currentScrollLineNumber);
    }

    this._closeTabs([networkTabId], true);
    if (wasSelectedInNetwork)
      this._tabbedPane.selectTab(fileSystemTabId, false);

    this._updateHistory();
  }

  /**
   * @param {!Common.Event} event
   */
  _onBindingRemoved(event) {
    var binding = /** @type {!Persistence.PersistenceBinding} */ (event.data);
    this._updateFileTitle(binding.fileSystem);
  }

  /**
   * @return {!UI.Widget}
   */
  get view() {
    return this._tabbedPane;
  }

  /**
   * @return {?UI.Widget}
   */
  get visibleView() {
    return this._tabbedPane.visibleView;
  }

  /**
   * @return {!Array.<!UI.Widget>}
   */
  fileViews() {
    return /** @type {!Array.<!UI.Widget>} */ (this._tabbedPane.tabViews());
  }

  /**
   * @return {!UI.Toolbar}
   */
  leftToolbar() {
    return this._tabbedPane.leftToolbar();
  }

  /**
   * @return {!UI.Toolbar}
   */
  rightToolbar() {
    return this._tabbedPane.rightToolbar();
  }

  /**
   * @param {!Element} parentElement
   */
  show(parentElement) {
    this._tabbedPane.show(parentElement);
  }

  /**
   * @param {!Workspace.UISourceCode} uiSourceCode
   */
  showFile(uiSourceCode) {
    this._innerShowFile(uiSourceCode, true);
  }

  /**
   * @param {!Workspace.UISourceCode} uiSourceCode
   */
  closeFile(uiSourceCode) {
    var tabId = this._tabIds.get(uiSourceCode);
    if (!tabId)
      return;
    this._closeTabs([tabId]);
  }

  closeAllFiles() {
    this._closeTabs(this._tabbedPane.allTabs());
  }

  /**
   * @return {!Array.<!Workspace.UISourceCode>}
   */
  historyUISourceCodes() {
    // FIXME: there should be a way to fetch UISourceCode for its uri.
    var uriToUISourceCode = {};
    for (var id in this._files) {
      var uiSourceCode = this._files[id];
      uriToUISourceCode[uiSourceCode.url()] = uiSourceCode;
    }

    var result = [];
    var uris = this._history._urls();
    for (var i = 0; i < uris.length; ++i) {
      var uiSourceCode = uriToUISourceCode[uris[i]];
      if (uiSourceCode)
        result.push(uiSourceCode);
    }
    return result;
  }

  _addViewListeners() {
    if (!this._currentView || !this._currentView.textEditor)
      return;
    this._currentView.textEditor.addEventListener(
        SourceFrame.SourcesTextEditor.Events.ScrollChanged, this._scrollChanged, this);
    this._currentView.textEditor.addEventListener(
        SourceFrame.SourcesTextEditor.Events.SelectionChanged, this._selectionChanged, this);
  }

  _removeViewListeners() {
    if (!this._currentView || !this._currentView.textEditor)
      return;
    this._currentView.textEditor.removeEventListener(
        SourceFrame.SourcesTextEditor.Events.ScrollChanged, this._scrollChanged, this);
    this._currentView.textEditor.removeEventListener(
        SourceFrame.SourcesTextEditor.Events.SelectionChanged, this._selectionChanged, this);
  }

  /**
   * @param {!Common.Event} event
   */
  _scrollChanged(event) {
    if (this._scrollTimer)
      clearTimeout(this._scrollTimer);
    var lineNumber = /** @type {number} */ (event.data);
    this._scrollTimer = setTimeout(saveHistory.bind(this), 100);
    this._history.updateScrollLineNumber(this._currentFile.url(), lineNumber);

    /**
     * @this {Sources.TabbedEditorContainer}
     */
    function saveHistory() {
      this._history.save(this._previouslyViewedFilesSetting);
    }
  }

  /**
   * @param {!Common.Event} event
   */
  _selectionChanged(event) {
    var range = /** @type {!TextUtils.TextRange} */ (event.data);
    this._history.updateSelectionRange(this._currentFile.url(), range);
    this._history.save(this._previouslyViewedFilesSetting);

    Extensions.extensionServer.sourceSelectionChanged(this._currentFile.url(), range);
  }

  /**
   * @param {!Workspace.UISourceCode} uiSourceCode
   * @param {boolean=} userGesture
   */
  _innerShowFile(uiSourceCode, userGesture) {
    var binding = Persistence.persistence.binding(uiSourceCode);
    uiSourceCode = binding ? binding.fileSystem : uiSourceCode;
    if (this._currentFile === uiSourceCode)
      return;

    this._removeViewListeners();
    this._currentFile = uiSourceCode;

    var tabId = this._tabIds.get(uiSourceCode) || this._appendFileTab(uiSourceCode, userGesture);

    this._tabbedPane.selectTab(tabId, userGesture);
    if (userGesture)
      this._editorSelectedByUserAction();

    var previousView = this._currentView;
    this._currentView = this.visibleView;
    this._addViewListeners();

    var eventData = {
      currentFile: this._currentFile,
      currentView: this._currentView,
      previousView: previousView,
      userGesture: userGesture
    };
    this.dispatchEventToListeners(Sources.TabbedEditorContainer.Events.EditorSelected, eventData);
  }

  /**
   * @param {!Workspace.UISourceCode} uiSourceCode
   * @return {string}
   */
  _titleForFile(uiSourceCode) {
    var maxDisplayNameLength = 30;
    var title = uiSourceCode.displayName(true).trimMiddle(maxDisplayNameLength);
    if (uiSourceCode.isDirty() || Persistence.persistence.hasUnsavedCommittedChanges(uiSourceCode))
      title += '*';
    return title;
  }

  /**
   * @param {string} id
   * @param {string} nextTabId
   */
  _maybeCloseTab(id, nextTabId) {
    var uiSourceCode = this._files[id];
    var shouldPrompt = uiSourceCode.isDirty() && uiSourceCode.project().canSetFileContent();
    // FIXME: this should be replaced with common Save/Discard/Cancel dialog.
    if (!shouldPrompt ||
        confirm(Common.UIString('Are you sure you want to close unsaved file: %s?', uiSourceCode.name()))) {
      uiSourceCode.resetWorkingCopy();
      if (nextTabId)
        this._tabbedPane.selectTab(nextTabId, true);
      this._tabbedPane.closeTab(id, true);
      return true;
    }
    return false;
  }

  /**
   * @param {!Array.<string>} ids
   * @param {boolean=} forceCloseDirtyTabs
   */
  _closeTabs(ids, forceCloseDirtyTabs) {
    var dirtyTabs = [];
    var cleanTabs = [];
    for (var i = 0; i < ids.length; ++i) {
      var id = ids[i];
      var uiSourceCode = this._files[id];
      if (!forceCloseDirtyTabs && uiSourceCode.isDirty())
        dirtyTabs.push(id);
      else
        cleanTabs.push(id);
    }
    if (dirtyTabs.length)
      this._tabbedPane.selectTab(dirtyTabs[0], true);
    this._tabbedPane.closeTabs(cleanTabs, true);
    for (var i = 0; i < dirtyTabs.length; ++i) {
      var nextTabId = i + 1 < dirtyTabs.length ? dirtyTabs[i + 1] : null;
      if (!this._maybeCloseTab(dirtyTabs[i], nextTabId))
        break;
    }
  }

  /**
   * @param {string} tabId
   * @param {!UI.ContextMenu} contextMenu
   */
  _onContextMenu(tabId, contextMenu) {
    var uiSourceCode = this._files[tabId];
    if (uiSourceCode)
      contextMenu.appendApplicableItems(uiSourceCode);
  }

  /**
   * @param {!Workspace.UISourceCode} uiSourceCode
   */
  addUISourceCode(uiSourceCode) {
    var uri = uiSourceCode.url();
    var index = this._history.index(uri);
    if (index === -1)
      return;

    if (!this._tabIds.has(uiSourceCode))
      this._appendFileTab(uiSourceCode, false);

    // Select tab if this file was the last to be shown.
    if (!index) {
      this._innerShowFile(uiSourceCode, false);
      return;
    }

    if (!this._currentFile)
      return;
    var currentProjectType = this._currentFile.project().type();
    var addedProjectType = uiSourceCode.project().type();
    var snippetsProjectType = Workspace.projectTypes.Snippets;
    if (this._history.index(this._currentFile.url()) && currentProjectType === snippetsProjectType &&
        addedProjectType !== snippetsProjectType)
      this._innerShowFile(uiSourceCode, false);
  }

  /**
   * @param {!Workspace.UISourceCode} uiSourceCode
   */
  removeUISourceCode(uiSourceCode) {
    this.removeUISourceCodes([uiSourceCode]);
  }

  /**
   * @param {!Array.<!Workspace.UISourceCode>} uiSourceCodes
   */
  removeUISourceCodes(uiSourceCodes) {
    var tabIds = [];
    for (var i = 0; i < uiSourceCodes.length; ++i) {
      var uiSourceCode = uiSourceCodes[i];
      var tabId = this._tabIds.get(uiSourceCode);
      if (tabId)
        tabIds.push(tabId);
    }
    this._tabbedPane.closeTabs(tabIds);
  }

  /**
   * @param {!Workspace.UISourceCode} uiSourceCode
   */
  _editorClosedByUserAction(uiSourceCode) {
    this._history.remove(uiSourceCode.url());
    this._updateHistory();
  }

  _editorSelectedByUserAction() {
    this._updateHistory();
  }

  _updateHistory() {
    var tabIds = this._tabbedPane.lastOpenedTabIds(Sources.TabbedEditorContainer.maximalPreviouslyViewedFilesCount);

    /**
     * @param {string} tabId
     * @this {Sources.TabbedEditorContainer}
     */
    function tabIdToURI(tabId) {
      return this._files[tabId].url();
    }

    this._history.update(tabIds.map(tabIdToURI.bind(this)));
    this._history.save(this._previouslyViewedFilesSetting);
  }

  /**
   * @param {!Workspace.UISourceCode} uiSourceCode
   * @return {string}
   */
  _tooltipForFile(uiSourceCode) {
    uiSourceCode = Persistence.persistence.fileSystem(uiSourceCode) || uiSourceCode;
    return uiSourceCode.url();
  }

  /**
   * @param {!Workspace.UISourceCode} uiSourceCode
   * @param {boolean=} userGesture
   * @param {number=} index
   * @return {string}
   */
  _appendFileTab(uiSourceCode, userGesture, index) {
    var view = this._delegate.viewForFile(uiSourceCode);
    var title = this._titleForFile(uiSourceCode);
    var tooltip = this._tooltipForFile(uiSourceCode);

    var tabId = this._generateTabId();
    this._tabIds.set(uiSourceCode, tabId);
    this._files[tabId] = uiSourceCode;

    var savedSelectionRange = this._history.selectionRange(uiSourceCode.url());
    var savedScrollLineNumber = this._history.scrollLineNumber(uiSourceCode.url());
    this._restoreEditorProperties(view, savedSelectionRange, savedScrollLineNumber);

    this._tabbedPane.appendTab(tabId, title, view, tooltip, userGesture, undefined, index);

    this._updateFileTitle(uiSourceCode);
    this._addUISourceCodeListeners(uiSourceCode);
    return tabId;
  }

  /**
   * @param {!UI.Widget} editorView
   * @param {!TextUtils.TextRange=} selection
   * @param {number=} firstLineNumber
   */
  _restoreEditorProperties(editorView, selection, firstLineNumber) {
    var sourceFrame =
        editorView instanceof SourceFrame.SourceFrame ? /** @type {!SourceFrame.SourceFrame} */ (editorView) : null;
    if (!sourceFrame)
      return;
    if (selection)
      sourceFrame.setSelection(selection);
    if (typeof firstLineNumber === 'number')
      sourceFrame.scrollToLine(firstLineNumber);
  }

  /**
   * @param {!Common.Event} event
   */
  _tabClosed(event) {
    var tabId = /** @type {string} */ (event.data.tabId);
    var userGesture = /** @type {boolean} */ (event.data.isUserGesture);

    var uiSourceCode = this._files[tabId];
    if (this._currentFile === uiSourceCode) {
      this._removeViewListeners();
      delete this._currentView;
      delete this._currentFile;
    }
    this._tabIds.remove(uiSourceCode);
    delete this._files[tabId];

    this._removeUISourceCodeListeners(uiSourceCode);

    this.dispatchEventToListeners(Sources.TabbedEditorContainer.Events.EditorClosed, uiSourceCode);

    if (userGesture)
      this._editorClosedByUserAction(uiSourceCode);
  }

  /**
   * @param {!Common.Event} event
   */
  _tabSelected(event) {
    var tabId = /** @type {string} */ (event.data.tabId);
    var userGesture = /** @type {boolean} */ (event.data.isUserGesture);

    var uiSourceCode = this._files[tabId];
    this._innerShowFile(uiSourceCode, userGesture);
  }

  /**
   * @param {!Workspace.UISourceCode} uiSourceCode
   */
  _addUISourceCodeListeners(uiSourceCode) {
    uiSourceCode.addEventListener(Workspace.UISourceCode.Events.TitleChanged, this._uiSourceCodeTitleChanged, this);
    uiSourceCode.addEventListener(
        Workspace.UISourceCode.Events.WorkingCopyChanged, this._uiSourceCodeWorkingCopyChanged, this);
    uiSourceCode.addEventListener(
        Workspace.UISourceCode.Events.WorkingCopyCommitted, this._uiSourceCodeWorkingCopyCommitted, this);
  }

  /**
   * @param {!Workspace.UISourceCode} uiSourceCode
   */
  _removeUISourceCodeListeners(uiSourceCode) {
    uiSourceCode.removeEventListener(Workspace.UISourceCode.Events.TitleChanged, this._uiSourceCodeTitleChanged, this);
    uiSourceCode.removeEventListener(
        Workspace.UISourceCode.Events.WorkingCopyChanged, this._uiSourceCodeWorkingCopyChanged, this);
    uiSourceCode.removeEventListener(
        Workspace.UISourceCode.Events.WorkingCopyCommitted, this._uiSourceCodeWorkingCopyCommitted, this);
  }

  /**
   * @param {!Workspace.UISourceCode} uiSourceCode
   */
  _updateFileTitle(uiSourceCode) {
    var tabId = this._tabIds.get(uiSourceCode);
    if (tabId) {
      var title = this._titleForFile(uiSourceCode);
      this._tabbedPane.changeTabTitle(tabId, title);
      var icon = null;
      if (Persistence.persistence.hasUnsavedCommittedChanges(uiSourceCode)) {
        icon = UI.Icon.create('smallicon-warning');
        icon.title = Common.UIString('Changes to this file were not saved to file system.');
      } else {
        icon = Persistence.PersistenceUtils.iconForUISourceCode(uiSourceCode);
      }
      this._tabbedPane.setTabIcon(tabId, icon);
    }
  }

  /**
   * @param {!Common.Event} event
   */
  _uiSourceCodeTitleChanged(event) {
    var uiSourceCode = /** @type {!Workspace.UISourceCode} */ (event.data);
    this._updateFileTitle(uiSourceCode);
    this._updateHistory();
  }

  /**
   * @param {!Common.Event} event
   */
  _uiSourceCodeWorkingCopyChanged(event) {
    var uiSourceCode = /** @type {!Workspace.UISourceCode} */ (event.data);
    this._updateFileTitle(uiSourceCode);
  }

  /**
   * @param {!Common.Event} event
   */
  _uiSourceCodeWorkingCopyCommitted(event) {
    var uiSourceCode = /** @type {!Workspace.UISourceCode} */ (event.data.uiSourceCode);
    this._updateFileTitle(uiSourceCode);
  }

  /**
   * @return {string}
   */
  _generateTabId() {
    return 'tab_' + (Sources.TabbedEditorContainer._tabId++);
  }

  /**
   * @return {?Workspace.UISourceCode} uiSourceCode
   */
  currentFile() {
    return this._currentFile || null;
  }
};

/** @enum {symbol} */
Sources.TabbedEditorContainer.Events = {
  EditorSelected: Symbol('EditorSelected'),
  EditorClosed: Symbol('EditorClosed')
};

Sources.TabbedEditorContainer._tabId = 0;

Sources.TabbedEditorContainer.maximalPreviouslyViewedFilesCount = 30;

/**
 * @unrestricted
 */
Sources.TabbedEditorContainer.HistoryItem = class {
  /**
   * @param {string} url
   * @param {!TextUtils.TextRange=} selectionRange
   * @param {number=} scrollLineNumber
   */
  constructor(url, selectionRange, scrollLineNumber) {
    /** @const */ this.url = url;
    /** @const */ this._isSerializable =
        url.length < Sources.TabbedEditorContainer.HistoryItem.serializableUrlLengthLimit;
    this.selectionRange = selectionRange;
    this.scrollLineNumber = scrollLineNumber;
  }

  /**
   * @param {!Object} serializedHistoryItem
   * @return {!Sources.TabbedEditorContainer.HistoryItem}
   */
  static fromObject(serializedHistoryItem) {
    var selectionRange = serializedHistoryItem.selectionRange ?
        TextUtils.TextRange.fromObject(serializedHistoryItem.selectionRange) :
        undefined;
    return new Sources.TabbedEditorContainer.HistoryItem(
        serializedHistoryItem.url, selectionRange, serializedHistoryItem.scrollLineNumber);
  }

  /**
   * @return {?Object}
   */
  serializeToObject() {
    if (!this._isSerializable)
      return null;
    var serializedHistoryItem = {};
    serializedHistoryItem.url = this.url;
    serializedHistoryItem.selectionRange = this.selectionRange;
    serializedHistoryItem.scrollLineNumber = this.scrollLineNumber;
    return serializedHistoryItem;
  }
};

Sources.TabbedEditorContainer.HistoryItem.serializableUrlLengthLimit = 4096;


/**
 * @unrestricted
 */
Sources.TabbedEditorContainer.History = class {
  /**
   * @param {!Array.<!Sources.TabbedEditorContainer.HistoryItem>} items
   */
  constructor(items) {
    this._items = items;
    this._rebuildItemIndex();
  }

  /**
   * @param {!Array.<!Object>} serializedHistory
   * @return {!Sources.TabbedEditorContainer.History}
   */
  static fromObject(serializedHistory) {
    var items = [];
    for (var i = 0; i < serializedHistory.length; ++i)
      items.push(Sources.TabbedEditorContainer.HistoryItem.fromObject(serializedHistory[i]));
    return new Sources.TabbedEditorContainer.History(items);
  }

  /**
   * @param {string} url
   * @return {number}
   */
  index(url) {
    return this._itemsIndex.has(url) ? /** @type {number} */ (this._itemsIndex.get(url)) : -1;
  }

  _rebuildItemIndex() {
    /** @type {!Map<string, number>} */
    this._itemsIndex = new Map();
    for (var i = 0; i < this._items.length; ++i) {
      console.assert(!this._itemsIndex.has(this._items[i].url));
      this._itemsIndex.set(this._items[i].url, i);
    }
  }

  /**
   * @param {string} url
   * @return {!TextUtils.TextRange|undefined}
   */
  selectionRange(url) {
    var index = this.index(url);
    return index !== -1 ? this._items[index].selectionRange : undefined;
  }

  /**
   * @param {string} url
   * @param {!TextUtils.TextRange=} selectionRange
   */
  updateSelectionRange(url, selectionRange) {
    if (!selectionRange)
      return;
    var index = this.index(url);
    if (index === -1)
      return;
    this._items[index].selectionRange = selectionRange;
  }

  /**
   * @param {string} url
   * @return {number|undefined}
   */
  scrollLineNumber(url) {
    var index = this.index(url);
    return index !== -1 ? this._items[index].scrollLineNumber : undefined;
  }

  /**
   * @param {string} url
   * @param {number} scrollLineNumber
   */
  updateScrollLineNumber(url, scrollLineNumber) {
    var index = this.index(url);
    if (index === -1)
      return;
    this._items[index].scrollLineNumber = scrollLineNumber;
  }

  /**
   * @param {!Array.<string>} urls
   */
  update(urls) {
    for (var i = urls.length - 1; i >= 0; --i) {
      var index = this.index(urls[i]);
      var item;
      if (index !== -1) {
        item = this._items[index];
        this._items.splice(index, 1);
      } else {
        item = new Sources.TabbedEditorContainer.HistoryItem(urls[i]);
      }
      this._items.unshift(item);
      this._rebuildItemIndex();
    }
  }

  /**
   * @param {string} url
   */
  remove(url) {
    var index = this.index(url);
    if (index !== -1) {
      this._items.splice(index, 1);
      this._rebuildItemIndex();
    }
  }

  /**
   * @param {!Common.Setting} setting
   */
  save(setting) {
    setting.set(this._serializeToObject());
  }

  /**
   * @return {!Array.<!Object>}
   */
  _serializeToObject() {
    var serializedHistory = [];
    for (var i = 0; i < this._items.length; ++i) {
      var serializedItem = this._items[i].serializeToObject();
      if (serializedItem)
        serializedHistory.push(serializedItem);
      if (serializedHistory.length === Sources.TabbedEditorContainer.maximalPreviouslyViewedFilesCount)
        break;
    }
    return serializedHistory;
  }

  /**
   * @return {!Array.<string>}
   */
  _urls() {
    var result = [];
    for (var i = 0; i < this._items.length; ++i)
      result.push(this._items[i].url);
    return result;
  }
};


/**
 * @implements {UI.TabbedPaneTabDelegate}
 * @unrestricted
 */
Sources.EditorContainerTabDelegate = class {
  /**
   * @param {!Sources.TabbedEditorContainer} editorContainer
   */
  constructor(editorContainer) {
    this._editorContainer = editorContainer;
  }

  /**
   * @override
   * @param {!UI.TabbedPane} tabbedPane
   * @param {!Array.<string>} ids
   */
  closeTabs(tabbedPane, ids) {
    this._editorContainer._closeTabs(ids);
  }

  /**
   * @override
   * @param {string} tabId
   * @param {!UI.ContextMenu} contextMenu
   */
  onContextMenu(tabId, contextMenu) {
    this._editorContainer._onContextMenu(tabId, contextMenu);
  }
};
