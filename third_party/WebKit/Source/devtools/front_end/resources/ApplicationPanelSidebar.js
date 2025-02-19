/*
 * Copyright (C) 2007, 2008, 2010 Apple Inc.  All rights reserved.
 * Copyright (C) 2009 Joseph Pecoraro
 * Copyright (C) 2013 Samsung Electronics. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * @implements {SDK.TargetManager.Observer}
 * @unrestricted
 */
Resources.ApplicationPanelSidebar = class extends UI.VBox {
  /**
   * @param {!Resources.ResourcesPanel} panel
   */
  constructor(panel) {
    super();

    this._panel = panel;

    this._sidebarTree = new UI.TreeOutlineInShadow();
    this._sidebarTree.element.classList.add('resources-sidebar');
    this._sidebarTree.registerRequiredCSS('resources/resourcesSidebar.css');
    this._sidebarTree.element.classList.add('filter-all');
    // Listener needs to have been set up before the elements are added
    this._sidebarTree.addEventListener(UI.TreeOutline.Events.ElementAttached, this._treeElementAdded, this);

    this.contentElement.appendChild(this._sidebarTree.element);
    this._applicationTreeElement = this._addSidebarSection(Common.UIString('Application'));
    var manifestTreeElement = new Resources.AppManifestTreeElement(panel);
    this._applicationTreeElement.appendChild(manifestTreeElement);
    this.serviceWorkersTreeElement = new Resources.ServiceWorkersTreeElement(panel);
    this._applicationTreeElement.appendChild(this.serviceWorkersTreeElement);
    var clearStorageTreeElement = new Resources.ClearStorageTreeElement(panel);
    this._applicationTreeElement.appendChild(clearStorageTreeElement);

    var storageTreeElement = this._addSidebarSection(Common.UIString('Storage'));
    this.localStorageListTreeElement =
        new Resources.StorageCategoryTreeElement(panel, Common.UIString('Local Storage'), 'LocalStorage');
    var localStorageIcon = UI.Icon.create('mediumicon-table', 'resource-tree-item');
    this.localStorageListTreeElement.setLeadingIcons([localStorageIcon]);

    storageTreeElement.appendChild(this.localStorageListTreeElement);
    this.sessionStorageListTreeElement =
        new Resources.StorageCategoryTreeElement(panel, Common.UIString('Session Storage'), 'SessionStorage');
    var sessionStorageIcon = UI.Icon.create('mediumicon-table', 'resource-tree-item');
    this.sessionStorageListTreeElement.setLeadingIcons([sessionStorageIcon]);

    storageTreeElement.appendChild(this.sessionStorageListTreeElement);
    this.indexedDBListTreeElement = new Resources.IndexedDBTreeElement(panel);
    storageTreeElement.appendChild(this.indexedDBListTreeElement);
    this.databasesListTreeElement =
        new Resources.StorageCategoryTreeElement(panel, Common.UIString('Web SQL'), 'Databases');
    var databaseIcon = UI.Icon.create('mediumicon-database', 'resource-tree-item');
    this.databasesListTreeElement.setLeadingIcons([databaseIcon]);

    storageTreeElement.appendChild(this.databasesListTreeElement);
    this.cookieListTreeElement = new Resources.StorageCategoryTreeElement(panel, Common.UIString('Cookies'), 'Cookies');
    var cookieIcon = UI.Icon.create('mediumicon-cookie', 'resource-tree-item');
    this.cookieListTreeElement.setLeadingIcons([cookieIcon]);
    storageTreeElement.appendChild(this.cookieListTreeElement);

    var cacheTreeElement = this._addSidebarSection(Common.UIString('Cache'));
    this.cacheStorageListTreeElement = new Resources.ServiceWorkerCacheTreeElement(panel);
    cacheTreeElement.appendChild(this.cacheStorageListTreeElement);
    this.applicationCacheListTreeElement =
        new Resources.StorageCategoryTreeElement(panel, Common.UIString('Application Cache'), 'ApplicationCache');
    var applicationCacheIcon = UI.Icon.create('mediumicon-table', 'resource-tree-item');
    this.applicationCacheListTreeElement.setLeadingIcons([applicationCacheIcon]);

    cacheTreeElement.appendChild(this.applicationCacheListTreeElement);

    this._resourcesSection = new Resources.ResourcesSection(panel, this._addSidebarSection(Common.UIString('Frames')));

    /** @type {!Map.<!Resources.Database, !Object.<string, !Resources.DatabaseTableView>>} */
    this._databaseTableViews = new Map();
    /** @type {!Map.<!Resources.Database, !Resources.DatabaseQueryView>} */
    this._databaseQueryViews = new Map();
    /** @type {!Map.<!Resources.Database, !Resources.DatabaseTreeElement>} */
    this._databaseTreeElements = new Map();
    /** @type {!Map.<!Resources.DOMStorage, !Resources.DOMStorageTreeElement>} */
    this._domStorageTreeElements = new Map();
    /** @type {!Object.<string, boolean>} */
    this._domains = {};

    this._sidebarTree.contentElement.addEventListener('mousemove', this._onmousemove.bind(this), false);
    this._sidebarTree.contentElement.addEventListener('mouseleave', this._onmouseleave.bind(this), false);

    SDK.targetManager.observeTargets(this);
    SDK.targetManager.addModelListener(
        SDK.ResourceTreeModel, SDK.ResourceTreeModel.Events.FrameNavigated, this._frameNavigated, this);

    var selection = this._panel.lastSelectedItemPath();
    if (!selection.length)
      manifestTreeElement.select();
  }

  /**
   * @param {string} title
   * @return {!UI.TreeElement}
   */
  _addSidebarSection(title) {
    var treeElement = new UI.TreeElement(title, true);
    treeElement.listItemElement.classList.add('storage-group-list-item');
    treeElement.setCollapsible(false);
    treeElement.selectable = false;
    this._sidebarTree.appendChild(treeElement);
    return treeElement;
  }

  /**
   * @override
   * @param {!SDK.Target} target
   */
  targetAdded(target) {
    if (this._target)
      return;
    this._target = target;
    this._databaseModel = target.model(Resources.DatabaseModel);
    this._databaseModel.addEventListener(Resources.DatabaseModel.Events.DatabaseAdded, this._databaseAdded, this);
    this._databaseModel.addEventListener(Resources.DatabaseModel.Events.DatabasesRemoved, this._resetWebSQL, this);

    var resourceTreeModel = target.model(SDK.ResourceTreeModel);
    if (!resourceTreeModel)
      return;

    if (resourceTreeModel.cachedResourcesLoaded())
      this._initialize();

    resourceTreeModel.addEventListener(SDK.ResourceTreeModel.Events.CachedResourcesLoaded, this._initialize, this);
    resourceTreeModel.addEventListener(
        SDK.ResourceTreeModel.Events.WillLoadCachedResources, this._resetWithFrames, this);
  }

  /**
   * @override
   * @param {!SDK.Target} target
   */
  targetRemoved(target) {
    if (target !== this._target)
      return;
    delete this._target;

    var resourceTreeModel = target.model(SDK.ResourceTreeModel);
    if (resourceTreeModel) {
      resourceTreeModel.removeEventListener(SDK.ResourceTreeModel.Events.CachedResourcesLoaded, this._initialize, this);
      resourceTreeModel.removeEventListener(
          SDK.ResourceTreeModel.Events.WillLoadCachedResources, this._resetWithFrames, this);
    }
    this._databaseModel.removeEventListener(Resources.DatabaseModel.Events.DatabaseAdded, this._databaseAdded, this);
    this._databaseModel.removeEventListener(Resources.DatabaseModel.Events.DatabasesRemoved, this._resetWebSQL, this);

    this._resetWithFrames();
  }

  /**
   * @override
   */
  focus() {
    this._sidebarTree.focus();
  }

  _initialize() {
    for (var frame of SDK.ResourceTreeModel.frames())
      this._addCookieDocument(frame);
    this._databaseModel.enable();

    var indexedDBModel = this._target.model(Resources.IndexedDBModel);
    if (indexedDBModel)
      indexedDBModel.enable();

    var cacheStorageModel = this._target.model(SDK.ServiceWorkerCacheModel);
    if (cacheStorageModel)
      cacheStorageModel.enable();
    var resourceTreeModel = this._target.model(SDK.ResourceTreeModel);
    if (resourceTreeModel)
      this._populateApplicationCacheTree(resourceTreeModel);
    var domStorageModel = this._target.model(Resources.DOMStorageModel);
    if (domStorageModel)
      this._populateDOMStorageTree(domStorageModel);
    this.indexedDBListTreeElement._initialize();
    var serviceWorkerCacheModel = this._target.model(SDK.ServiceWorkerCacheModel);
    this.cacheStorageListTreeElement._initialize(serviceWorkerCacheModel);
  }

  _resetWithFrames() {
    this._resourcesSection.reset();
    this._reset();
  }

  _resetWebSQL() {
    var queryViews = this._databaseQueryViews.valuesArray();
    for (var i = 0; i < queryViews.length; ++i) {
      queryViews[i].removeEventListener(
          Resources.DatabaseQueryView.Events.SchemaUpdated, this._updateDatabaseTables, this);
    }
    this._databaseTableViews.clear();
    this._databaseQueryViews.clear();
    this._databaseTreeElements.clear();
    this.databasesListTreeElement.removeChildren();
    this.databasesListTreeElement.setExpandable(false);
  }

  _resetAppCache() {
    for (var frameId of Object.keys(this._applicationCacheFrameElements))
      this._applicationCacheFrameManifestRemoved({data: frameId});
    this.applicationCacheListTreeElement.setExpandable(false);
  }

  /**
   * @param {!Common.Event} event
   */
  _treeElementAdded(event) {
    var selection = this._panel.lastSelectedItemPath();
    if (!selection.length)
      return;
    var element = event.data;
    var index = selection.indexOf(element.itemURL);
    if (index < 0)
      return;
    for (var parent = element.parent; parent; parent = parent.parent)
      parent.expand();
    if (index > 0)
      element.expand();
    element.select();
  }

  _reset() {
    this._domains = {};
    this._resetWebSQL();
    this.cookieListTreeElement.removeChildren();
  }

  _frameNavigated(event) {
    var frame = event.data;

    if (!frame.parentFrame)
      this._reset();

    var applicationCacheFrameTreeElement = this._applicationCacheFrameElements[frame.id];
    if (applicationCacheFrameTreeElement)
      applicationCacheFrameTreeElement.frameNavigated(frame);
    this._addCookieDocument(frame);
  }

  /**
   * @param {!Common.Event} event
   */
  _databaseAdded(event) {
    var database = /** @type {!Resources.Database} */ (event.data);
    var databaseTreeElement = new Resources.DatabaseTreeElement(this, database);
    this._databaseTreeElements.set(database, databaseTreeElement);
    this.databasesListTreeElement.appendChild(databaseTreeElement);
  }

  /**
   * @param {!SDK.ResourceTreeFrame} frame
   */
  _addCookieDocument(frame) {
    var parsedURL = frame.url.asParsedURL();
    if (!parsedURL || (parsedURL.scheme !== 'http' && parsedURL.scheme !== 'https' && parsedURL.scheme !== 'file'))
      return;

    var domain = parsedURL.securityOrigin();
    if (!this._domains[domain]) {
      this._domains[domain] = true;
      var cookieDomainTreeElement = new Resources.CookieTreeElement(this._panel, frame, domain);
      this.cookieListTreeElement.appendChild(cookieDomainTreeElement);
    }
  }

  /**
   * @param {!Common.Event} event
   */
  _domStorageAdded(event) {
    var domStorage = /** @type {!Resources.DOMStorage} */ (event.data);
    this._addDOMStorage(domStorage);
  }

  /**
   * @param {!Resources.DOMStorage} domStorage
   */
  _addDOMStorage(domStorage) {
    console.assert(!this._domStorageTreeElements.get(domStorage));

    var domStorageTreeElement = new Resources.DOMStorageTreeElement(this._panel, domStorage);
    this._domStorageTreeElements.set(domStorage, domStorageTreeElement);
    if (domStorage.isLocalStorage)
      this.localStorageListTreeElement.appendChild(domStorageTreeElement);
    else
      this.sessionStorageListTreeElement.appendChild(domStorageTreeElement);
  }

  /**
   * @param {!Common.Event} event
   */
  _domStorageRemoved(event) {
    var domStorage = /** @type {!Resources.DOMStorage} */ (event.data);
    this._removeDOMStorage(domStorage);
  }

  /**
   * @param {!Resources.DOMStorage} domStorage
   */
  _removeDOMStorage(domStorage) {
    var treeElement = this._domStorageTreeElements.get(domStorage);
    if (!treeElement)
      return;
    var wasSelected = treeElement.selected;
    var parentListTreeElement = treeElement.parent;
    parentListTreeElement.removeChild(treeElement);
    if (wasSelected)
      parentListTreeElement.select();
    this._domStorageTreeElements.remove(domStorage);
  }

  /**
   * @param {!Resources.Database} database
   */
  selectDatabase(database) {
    if (database) {
      this._showDatabase(database);
      this._databaseTreeElements.get(database).select();
    }
  }

  /**
   * @param {!SDK.Resource} resource
   * @param {number=} line
   * @param {number=} column
   * @return {!Promise}
   */
  async showResource(resource, line, column) {
    var resourceTreeElement = Resources.FrameResourceTreeElement.forResource(resource);
    if (resourceTreeElement)
      await resourceTreeElement.revealResource(line, column);
  }

  /**
   * @param {!Resources.Database} database
   * @param {string=} tableName
   */
  _showDatabase(database, tableName) {
    if (!database)
      return;

    var view;
    if (tableName) {
      var tableViews = this._databaseTableViews.get(database);
      if (!tableViews) {
        tableViews = /** @type {!Object.<string, !Resources.DatabaseTableView>} */ ({});
        this._databaseTableViews.set(database, tableViews);
      }
      view = tableViews[tableName];
      if (!view) {
        view = new Resources.DatabaseTableView(database, tableName);
        tableViews[tableName] = view;
      }
    } else {
      view = this._databaseQueryViews.get(database);
      if (!view) {
        view = new Resources.DatabaseQueryView(database);
        this._databaseQueryViews.set(database, view);
        view.addEventListener(Resources.DatabaseQueryView.Events.SchemaUpdated, this._updateDatabaseTables, this);
      }
    }

    this._innerShowView(view);
  }

  _showApplicationCache(frameId) {
    if (!this._applicationCacheViews[frameId]) {
      this._applicationCacheViews[frameId] =
          new Resources.ApplicationCacheItemsView(this._applicationCacheModel, frameId);
    }

    this._innerShowView(this._applicationCacheViews[frameId]);
  }

  /**
   *  @param {!UI.Widget} view
   */
  showFileSystem(view) {
    this._innerShowView(view);
  }

  _innerShowView(view) {
    this._panel.showView(view);
  }

  _updateDatabaseTables(event) {
    var database = event.data;

    if (!database)
      return;

    var databasesTreeElement = this._databaseTreeElements.get(database);
    if (!databasesTreeElement)
      return;

    databasesTreeElement.invalidateChildren();
    var tableViews = this._databaseTableViews.get(database);

    if (!tableViews)
      return;

    var tableNamesHash = {};
    var panel = this._panel;
    function tableNamesCallback(tableNames) {
      var tableNamesLength = tableNames.length;
      for (var i = 0; i < tableNamesLength; ++i)
        tableNamesHash[tableNames[i]] = true;

      for (var tableName in tableViews) {
        if (!(tableName in tableNamesHash)) {
          if (panel.visibleView === tableViews[tableName])
            panel.showView(null);
          delete tableViews[tableName];
        }
      }
    }
    database.getTableNames(tableNamesCallback);
  }

  /**
   * @param {!Resources.DOMStorageModel} domStorageModel
   */
  _populateDOMStorageTree(domStorageModel) {
    domStorageModel.enable();
    domStorageModel.storages().forEach(this._addDOMStorage.bind(this));
    domStorageModel.addEventListener(Resources.DOMStorageModel.Events.DOMStorageAdded, this._domStorageAdded, this);
    domStorageModel.addEventListener(Resources.DOMStorageModel.Events.DOMStorageRemoved, this._domStorageRemoved, this);
  }

  /**
   * @param {!SDK.ResourceTreeModel} resourceTreeModel
   */
  _populateApplicationCacheTree(resourceTreeModel) {
    this._applicationCacheModel = this._target.model(Resources.ApplicationCacheModel);

    this._applicationCacheViews = {};
    this._applicationCacheFrameElements = {};
    this._applicationCacheManifestElements = {};

    this._applicationCacheModel.addEventListener(
        Resources.ApplicationCacheModel.Events.FrameManifestAdded, this._applicationCacheFrameManifestAdded, this);
    this._applicationCacheModel.addEventListener(
        Resources.ApplicationCacheModel.Events.FrameManifestRemoved, this._applicationCacheFrameManifestRemoved, this);
    this._applicationCacheModel.addEventListener(
        Resources.ApplicationCacheModel.Events.FrameManifestsReset, this._resetAppCache, this);

    this._applicationCacheModel.addEventListener(
        Resources.ApplicationCacheModel.Events.FrameManifestStatusUpdated,
        this._applicationCacheFrameManifestStatusChanged, this);
    this._applicationCacheModel.addEventListener(
        Resources.ApplicationCacheModel.Events.NetworkStateChanged, this._applicationCacheNetworkStateChanged, this);
  }

  _applicationCacheFrameManifestAdded(event) {
    var frameId = event.data;
    var manifestURL = this._applicationCacheModel.frameManifestURL(frameId);

    var manifestTreeElement = this._applicationCacheManifestElements[manifestURL];
    if (!manifestTreeElement) {
      manifestTreeElement = new Resources.ApplicationCacheManifestTreeElement(this._panel, manifestURL);
      this.applicationCacheListTreeElement.appendChild(manifestTreeElement);
      this._applicationCacheManifestElements[manifestURL] = manifestTreeElement;
    }

    var model = this._target.model(SDK.ResourceTreeModel);
    var frameTreeElement = new Resources.ApplicationCacheFrameTreeElement(this, model.frameForId(frameId), manifestURL);
    manifestTreeElement.appendChild(frameTreeElement);
    manifestTreeElement.expand();
    this._applicationCacheFrameElements[frameId] = frameTreeElement;
  }

  _applicationCacheFrameManifestRemoved(event) {
    var frameId = event.data;
    var frameTreeElement = this._applicationCacheFrameElements[frameId];
    if (!frameTreeElement)
      return;

    var manifestURL = frameTreeElement.manifestURL;
    delete this._applicationCacheFrameElements[frameId];
    delete this._applicationCacheViews[frameId];
    frameTreeElement.parent.removeChild(frameTreeElement);

    var manifestTreeElement = this._applicationCacheManifestElements[manifestURL];
    if (manifestTreeElement.childCount())
      return;

    delete this._applicationCacheManifestElements[manifestURL];
    manifestTreeElement.parent.removeChild(manifestTreeElement);
  }

  _applicationCacheFrameManifestStatusChanged(event) {
    var frameId = event.data;
    var status = this._applicationCacheModel.frameManifestStatus(frameId);

    if (this._applicationCacheViews[frameId])
      this._applicationCacheViews[frameId].updateStatus(status);
  }

  _applicationCacheNetworkStateChanged(event) {
    var isNowOnline = event.data;

    for (var manifestURL in this._applicationCacheViews)
      this._applicationCacheViews[manifestURL].updateNetworkState(isNowOnline);
  }

  showView(view) {
    if (view)
      this.showResource(view.resource);
  }

  _onmousemove(event) {
    var nodeUnderMouse = event.target;
    if (!nodeUnderMouse)
      return;

    var listNode = nodeUnderMouse.enclosingNodeOrSelfWithNodeName('li');
    if (!listNode)
      return;

    var element = listNode.treeElement;
    if (this._previousHoveredElement === element)
      return;

    if (this._previousHoveredElement) {
      this._previousHoveredElement.hovered = false;
      delete this._previousHoveredElement;
    }

    if (element instanceof Resources.FrameTreeElement) {
      this._previousHoveredElement = element;
      element.hovered = true;
    }
  }

  _onmouseleave(event) {
    if (this._previousHoveredElement) {
      this._previousHoveredElement.hovered = false;
      delete this._previousHoveredElement;
    }
  }
};

/**
 * @unrestricted
 */
Resources.BaseStorageTreeElement = class extends UI.TreeElement {
  /**
   * @param {!Resources.ResourcesPanel} storagePanel
   * @param {string} title
   * @param {boolean} expandable
   */
  constructor(storagePanel, title, expandable) {
    super(title, expandable);
    this._storagePanel = storagePanel;
  }

  /**
   * @override
   * @return {boolean}
   */
  onselect(selectedByUser) {
    if (!selectedByUser)
      return false;

    var path = [];
    for (var el = this; el; el = el.parent) {
      var url = el.itemURL;
      if (!url)
        break;
      path.push(url);
    }
    this._storagePanel.setLastSelectedItemPath(path);

    return false;
  }

  /**
   * @protected
   * @param {?UI.Widget} view
   */
  showView(view) {
    this._storagePanel.showView(view);
  }
};

Resources.StorageCategoryTreeElement = class extends Resources.BaseStorageTreeElement {
  /**
   * @param {!Resources.ResourcesPanel} storagePanel
   * @param {string} categoryName
   * @param {string} settingsKey
   */
  constructor(storagePanel, categoryName, settingsKey) {
    super(storagePanel, categoryName, false);
    this._expandedSetting =
        Common.settings.createSetting('resources' + settingsKey + 'Expanded', settingsKey === 'Frames');
    this._categoryName = categoryName;
  }


  get itemURL() {
    return 'category://' + this._categoryName;
  }

  /**
   * @override
   * @return {boolean}
   */
  onselect(selectedByUser) {
    super.onselect(selectedByUser);
    this._storagePanel.showCategoryView(this._categoryName);
    return false;
  }

  /**
   * @override
   */
  onattach() {
    super.onattach();
    if (this._expandedSetting.get())
      this.expand();
  }

  /**
   * @override
   */
  onexpand() {
    this._expandedSetting.set(true);
  }

  /**
   * @override
   */
  oncollapse() {
    this._expandedSetting.set(false);
  }
};

/**
 * @unrestricted
 */
Resources.DatabaseTreeElement = class extends Resources.BaseStorageTreeElement {
  /**
   * @param {!Resources.ApplicationPanelSidebar} sidebar
   * @param {!Resources.Database} database
   */
  constructor(sidebar, database) {
    super(sidebar._panel, database.name, true);
    this._sidebar = sidebar;
    this._database = database;

    var icon = UI.Icon.create('mediumicon-database', 'resource-tree-item');
    this.setLeadingIcons([icon]);
  }

  get itemURL() {
    return 'database://' + encodeURI(this._database.name);
  }

  /**
   * @override
   * @return {boolean}
   */
  onselect(selectedByUser) {
    super.onselect(selectedByUser);
    this._sidebar._showDatabase(this._database);
    return false;
  }

  /**
   * @override
   */
  onexpand() {
    this._updateChildren();
  }

  async _updateChildren() {
    var tableNames = await this._database.tableNames();
    for (var tableName of tableNames)
      this.appendChild(new Resources.DatabaseTableTreeElement(this._sidebar, this._database, tableName));
  }
};

/**
 * @unrestricted
 */
Resources.DatabaseTableTreeElement = class extends Resources.BaseStorageTreeElement {
  /**
   * @param {!Resources.ApplicationPanelSidebar} sidebar
   * @param {!Resources.Database} database
   * @param {string} tableName
   */
  constructor(sidebar, database, tableName) {
    super(sidebar._panel, tableName, false);
    this._sidebar = sidebar;
    this._database = database;
    this._tableName = tableName;
    var icon = UI.Icon.create('mediumicon-table', 'resource-tree-item');
    this.setLeadingIcons([icon]);
  }

  get itemURL() {
    return 'database://' + encodeURI(this._database.name) + '/' + encodeURI(this._tableName);
  }

  /**
   * @override
   * @return {boolean}
   */
  onselect(selectedByUser) {
    super.onselect(selectedByUser);
    this._sidebar._showDatabase(this._database, this._tableName);
    return false;
  }
};

/**
 * @unrestricted
 */
Resources.ServiceWorkerCacheTreeElement = class extends Resources.StorageCategoryTreeElement {
  /**
   * @param {!Resources.ResourcesPanel} storagePanel
   */
  constructor(storagePanel) {
    super(storagePanel, Common.UIString('Cache Storage'), 'CacheStorage');
    var icon = UI.Icon.create('mediumicon-database', 'resource-tree-item');
    this.setLeadingIcons([icon]);
    /** @type {?SDK.ServiceWorkerCacheModel} */
    this._swCacheModel = null;
  }

  /**
   * @param {?SDK.ServiceWorkerCacheModel} model
   */
  _initialize(model) {
    /** @type {!Array.<!Resources.SWCacheTreeElement>} */
    this._swCacheTreeElements = [];
    this._swCacheModel = model;
    if (model) {
      for (var cache of model.caches())
        this._addCache(model, cache);
    }
    SDK.targetManager.addModelListener(
        SDK.ServiceWorkerCacheModel, SDK.ServiceWorkerCacheModel.Events.CacheAdded, this._cacheAdded, this);
    SDK.targetManager.addModelListener(
        SDK.ServiceWorkerCacheModel, SDK.ServiceWorkerCacheModel.Events.CacheRemoved, this._cacheRemoved, this);
  }

  /**
   * @override
   */
  onattach() {
    super.onattach();
    this.listItemElement.addEventListener('contextmenu', this._handleContextMenuEvent.bind(this), true);
  }

  _handleContextMenuEvent(event) {
    var contextMenu = new UI.ContextMenu(event);
    contextMenu.appendItem(Common.UIString('Refresh Caches'), this._refreshCaches.bind(this));
    contextMenu.show();
  }

  _refreshCaches() {
    if (this._swCacheModel)
      this._swCacheModel.refreshCacheNames();
  }

  /**
   * @param {!Common.Event} event
   */
  _cacheAdded(event) {
    var cache = /** @type {!SDK.ServiceWorkerCacheModel.Cache} */ (event.data.cache);
    var model = /** @type {!SDK.ServiceWorkerCacheModel} */ (event.data.model);
    this._addCache(model, cache);
  }

  /**
   * @param {!SDK.ServiceWorkerCacheModel} model
   * @param {!SDK.ServiceWorkerCacheModel.Cache} cache
   */
  _addCache(model, cache) {
    var swCacheTreeElement = new Resources.SWCacheTreeElement(this._storagePanel, model, cache);
    this._swCacheTreeElements.push(swCacheTreeElement);
    this.appendChild(swCacheTreeElement);
  }

  /**
   * @param {!Common.Event} event
   */
  _cacheRemoved(event) {
    var cache = /** @type {!SDK.ServiceWorkerCacheModel.Cache} */ (event.data.cache);
    var model = /** @type {!SDK.ServiceWorkerCacheModel} */ (event.data.model);

    var swCacheTreeElement = this._cacheTreeElement(model, cache);
    if (!swCacheTreeElement)
      return;

    this.removeChild(swCacheTreeElement);
    this._swCacheTreeElements.remove(swCacheTreeElement);
    this.setExpandable(this.childCount() > 0);
  }

  /**
   * @param {!SDK.ServiceWorkerCacheModel} model
   * @param {!SDK.ServiceWorkerCacheModel.Cache} cache
   * @return {?Resources.SWCacheTreeElement}
   */
  _cacheTreeElement(model, cache) {
    var index = -1;
    for (var i = 0; i < this._swCacheTreeElements.length; ++i) {
      if (this._swCacheTreeElements[i]._cache.equals(cache) && this._swCacheTreeElements[i]._model === model) {
        index = i;
        break;
      }
    }
    if (index !== -1)
      return this._swCacheTreeElements[i];
    return null;
  }
};

Resources.SWCacheTreeElement = class extends Resources.BaseStorageTreeElement {
  /**
   * @param {!Resources.ResourcesPanel} storagePanel
   * @param {!SDK.ServiceWorkerCacheModel} model
   * @param {!SDK.ServiceWorkerCacheModel.Cache} cache
   */
  constructor(storagePanel, model, cache) {
    super(storagePanel, cache.cacheName + ' - ' + cache.securityOrigin, false);
    this._model = model;
    this._cache = cache;
    /** @type {?Resources.ServiceWorkerCacheView} */
    this._view = null;
    var icon = UI.Icon.create('mediumicon-table', 'resource-tree-item');
    this.setLeadingIcons([icon]);
  }

  get itemURL() {
    // I don't think this will work at all.
    return 'cache://' + this._cache.cacheId;
  }

  /**
   * @override
   */
  onattach() {
    super.onattach();
    this.listItemElement.addEventListener('contextmenu', this._handleContextMenuEvent.bind(this), true);
  }

  _handleContextMenuEvent(event) {
    var contextMenu = new UI.ContextMenu(event);
    contextMenu.appendItem(Common.UIString('Delete'), this._clearCache.bind(this));
    contextMenu.show();
  }

  _clearCache() {
    this._model.deleteCache(this._cache);
  }

  /**
   * @param {!SDK.ServiceWorkerCacheModel.Cache} cache
   */
  update(cache) {
    this._cache = cache;
    if (this._view)
      this._view.update(cache);
  }

  /**
   * @override
   * @return {boolean}
   */
  onselect(selectedByUser) {
    super.onselect(selectedByUser);
    if (!this._view)
      this._view = new Resources.ServiceWorkerCacheView(this._model, this._cache);

    this.showView(this._view);
    return false;
  }
};

/**
 * @unrestricted
 */
Resources.ServiceWorkersTreeElement = class extends Resources.BaseStorageTreeElement {
  /**
   * @param {!Resources.ResourcesPanel} storagePanel
   */
  constructor(storagePanel) {
    super(storagePanel, Common.UIString('Service Workers'), false);
    var icon = UI.Icon.create('mediumicon-service-worker', 'resource-tree-item');
    this.setLeadingIcons([icon]);
  }

  /**
   * @return {string}
   */
  get itemURL() {
    return 'service-workers://';
  }

  /**
   * @override
   * @return {boolean}
   */
  onselect(selectedByUser) {
    super.onselect(selectedByUser);
    if (!this._view)
      this._view = new Resources.ServiceWorkersView();
    this.showView(this._view);
    return false;
  }
};

/**
 * @unrestricted
 */
Resources.AppManifestTreeElement = class extends Resources.BaseStorageTreeElement {
  /**
   * @param {!Resources.ResourcesPanel} storagePanel
   */
  constructor(storagePanel) {
    super(storagePanel, Common.UIString('Manifest'), false);
    var icon = UI.Icon.create('mediumicon-manifest', 'resource-tree-item');
    this.setLeadingIcons([icon]);
  }

  /**
   * @return {string}
   */
  get itemURL() {
    return 'manifest://';
  }

  /**
   * @override
   * @return {boolean}
   */
  onselect(selectedByUser) {
    super.onselect(selectedByUser);
    if (!this._view)
      this._view = new Resources.AppManifestView();
    this.showView(this._view);
    return false;
  }
};

/**
 * @unrestricted
 */
Resources.ClearStorageTreeElement = class extends Resources.BaseStorageTreeElement {
  /**
   * @param {!Resources.ResourcesPanel} storagePanel
   */
  constructor(storagePanel) {
    super(storagePanel, Common.UIString('Clear storage'), false);
    var icon = UI.Icon.create('mediumicon-clear-storage', 'resource-tree-item');
    this.setLeadingIcons([icon]);
  }

  /**
   * @return {string}
   */
  get itemURL() {
    return 'clear-storage://';
  }

  /**
   * @override
   * @return {boolean}
   */
  onselect(selectedByUser) {
    super.onselect(selectedByUser);
    if (!this._view)
      this._view = new Resources.ClearStorageView();
    this.showView(this._view);
    return false;
  }
};

/**
 * @unrestricted
 */
Resources.IndexedDBTreeElement = class extends Resources.StorageCategoryTreeElement {
  /**
   * @param {!Resources.ResourcesPanel} storagePanel
   */
  constructor(storagePanel) {
    super(storagePanel, Common.UIString('IndexedDB'), 'IndexedDB');
    var icon = UI.Icon.create('mediumicon-database', 'resource-tree-item');
    this.setLeadingIcons([icon]);
  }

  _initialize() {
    SDK.targetManager.addModelListener(
        Resources.IndexedDBModel, Resources.IndexedDBModel.Events.DatabaseAdded, this._indexedDBAdded, this);
    SDK.targetManager.addModelListener(
        Resources.IndexedDBModel, Resources.IndexedDBModel.Events.DatabaseRemoved, this._indexedDBRemoved, this);
    SDK.targetManager.addModelListener(
        Resources.IndexedDBModel, Resources.IndexedDBModel.Events.DatabaseLoaded, this._indexedDBLoaded, this);
    /** @type {!Array.<!Resources.IDBDatabaseTreeElement>} */
    this._idbDatabaseTreeElements = [];

    for (var indexedDBModel of SDK.targetManager.models(Resources.IndexedDBModel)) {
      var databases = indexedDBModel.databases();
      for (var j = 0; j < databases.length; ++j)
        this._addIndexedDB(indexedDBModel, databases[j]);
    }
  }

  /**
   * @override
   */
  onattach() {
    super.onattach();
    this.listItemElement.addEventListener('contextmenu', this._handleContextMenuEvent.bind(this), true);
  }

  _handleContextMenuEvent(event) {
    var contextMenu = new UI.ContextMenu(event);
    contextMenu.appendItem(Common.UIString('Refresh IndexedDB'), this.refreshIndexedDB.bind(this));
    contextMenu.show();
  }

  refreshIndexedDB() {
    for (var indexedDBModel of SDK.targetManager.models(Resources.IndexedDBModel))
      indexedDBModel.refreshDatabaseNames();
  }

  /**
   * @param {!Common.Event} event
   */
  _indexedDBAdded(event) {
    var databaseId = /** @type {!Resources.IndexedDBModel.DatabaseId} */ (event.data.databaseId);
    var model = /** @type {!Resources.IndexedDBModel} */ (event.data.model);
    this._addIndexedDB(model, databaseId);
  }

  /**
   * @param {!Resources.IndexedDBModel} model
   * @param {!Resources.IndexedDBModel.DatabaseId} databaseId
   */
  _addIndexedDB(model, databaseId) {
    var idbDatabaseTreeElement = new Resources.IDBDatabaseTreeElement(this._storagePanel, model, databaseId);
    this._idbDatabaseTreeElements.push(idbDatabaseTreeElement);
    this.appendChild(idbDatabaseTreeElement);
    model.refreshDatabase(databaseId);
  }

  /**
   * @param {!Common.Event} event
   */
  _indexedDBRemoved(event) {
    var databaseId = /** @type {!Resources.IndexedDBModel.DatabaseId} */ (event.data.databaseId);
    var model = /** @type {!Resources.IndexedDBModel} */ (event.data.model);

    var idbDatabaseTreeElement = this._idbDatabaseTreeElement(model, databaseId);
    if (!idbDatabaseTreeElement)
      return;

    idbDatabaseTreeElement.clear();
    this.removeChild(idbDatabaseTreeElement);
    this._idbDatabaseTreeElements.remove(idbDatabaseTreeElement);
    this.setExpandable(this.childCount() > 0);
  }

  /**
   * @param {!Common.Event} event
   */
  _indexedDBLoaded(event) {
    var database = /** @type {!Resources.IndexedDBModel.Database} */ (event.data.database);
    var model = /** @type {!Resources.IndexedDBModel} */ (event.data.model);

    var idbDatabaseTreeElement = this._idbDatabaseTreeElement(model, database.databaseId);
    if (!idbDatabaseTreeElement)
      return;

    idbDatabaseTreeElement.update(database);
  }

  /**
   * @param {!Resources.IndexedDBModel.DatabaseId} databaseId
   * @param {!Resources.IndexedDBModel} model
   * @return {?Resources.IDBDatabaseTreeElement}
   */
  _idbDatabaseTreeElement(model, databaseId) {
    var index = -1;
    for (var i = 0; i < this._idbDatabaseTreeElements.length; ++i) {
      if (this._idbDatabaseTreeElements[i]._databaseId.equals(databaseId) &&
          this._idbDatabaseTreeElements[i]._model === model) {
        index = i;
        break;
      }
    }
    if (index !== -1)
      return this._idbDatabaseTreeElements[i];
    return null;
  }
};

/**
 * @unrestricted
 */
Resources.IDBDatabaseTreeElement = class extends Resources.BaseStorageTreeElement {
  /**
   * @param {!Resources.ResourcesPanel} storagePanel
   * @param {!Resources.IndexedDBModel} model
   * @param {!Resources.IndexedDBModel.DatabaseId} databaseId
   */
  constructor(storagePanel, model, databaseId) {
    super(storagePanel, databaseId.name + ' - ' + databaseId.securityOrigin, false);
    this._model = model;
    this._databaseId = databaseId;
    this._idbObjectStoreTreeElements = {};
    var icon = UI.Icon.create('mediumicon-database', 'resource-tree-item');
    this.setLeadingIcons([icon]);
    this._model.addEventListener(Resources.IndexedDBModel.Events.DatabaseNamesRefreshed, this._refreshIndexedDB, this);
  }

  get itemURL() {
    return 'indexedDB://' + this._databaseId.securityOrigin + '/' + this._databaseId.name;
  }

  /**
   * @override
   */
  onattach() {
    super.onattach();
    this.listItemElement.addEventListener('contextmenu', this._handleContextMenuEvent.bind(this), true);
  }

  _handleContextMenuEvent(event) {
    var contextMenu = new UI.ContextMenu(event);
    contextMenu.appendItem(Common.UIString('Refresh IndexedDB'), this._refreshIndexedDB.bind(this));
    contextMenu.show();
  }

  _refreshIndexedDB() {
    this._model.refreshDatabase(this._databaseId);
  }

  /**
   * @param {!Resources.IndexedDBModel.Database} database
   */
  update(database) {
    this._database = database;
    var objectStoreNames = {};
    for (var objectStoreName in this._database.objectStores) {
      var objectStore = this._database.objectStores[objectStoreName];
      objectStoreNames[objectStore.name] = true;
      if (!this._idbObjectStoreTreeElements[objectStore.name]) {
        var idbObjectStoreTreeElement =
            new Resources.IDBObjectStoreTreeElement(this._storagePanel, this._model, this._databaseId, objectStore);
        this._idbObjectStoreTreeElements[objectStore.name] = idbObjectStoreTreeElement;
        this.appendChild(idbObjectStoreTreeElement);
      }
      this._idbObjectStoreTreeElements[objectStore.name].update(objectStore);
    }
    for (var objectStoreName in this._idbObjectStoreTreeElements) {
      if (!objectStoreNames[objectStoreName])
        this._objectStoreRemoved(objectStoreName);
    }

    if (this._view)
      this._view.update(database);

    this._updateTooltip();
  }

  _updateTooltip() {
    this.tooltip = Common.UIString('Version') + ': ' + this._database.version;
  }

  /**
   * @override
   * @return {boolean}
   */
  onselect(selectedByUser) {
    super.onselect(selectedByUser);
    if (!this._view)
      this._view = new Resources.IDBDatabaseView(this._model, this._database);

    this.showView(this._view);
    return false;
  }

  /**
   * @param {string} objectStoreName
   */
  _objectStoreRemoved(objectStoreName) {
    var objectStoreTreeElement = this._idbObjectStoreTreeElements[objectStoreName];
    objectStoreTreeElement.clear();
    this.removeChild(objectStoreTreeElement);
    delete this._idbObjectStoreTreeElements[objectStoreName];
  }

  clear() {
    for (var objectStoreName in this._idbObjectStoreTreeElements)
      this._objectStoreRemoved(objectStoreName);
  }
};

Resources.IDBObjectStoreTreeElement = class extends Resources.BaseStorageTreeElement {
  /**
   * @param {!Resources.ResourcesPanel} storagePanel
   * @param {!Resources.IndexedDBModel} model
   * @param {!Resources.IndexedDBModel.DatabaseId} databaseId
   * @param {!Resources.IndexedDBModel.ObjectStore} objectStore
   */
  constructor(storagePanel, model, databaseId, objectStore) {
    super(storagePanel, objectStore.name, false);
    this._model = model;
    this._databaseId = databaseId;
    this._idbIndexTreeElements = {};
    this._objectStore = objectStore;
    /** @type {?Resources.IDBDataView} */
    this._view = null;
    var icon = UI.Icon.create('mediumicon-table', 'resource-tree-item');
    this.setLeadingIcons([icon]);
  }

  get itemURL() {
    return 'indexedDB://' + this._databaseId.securityOrigin + '/' + this._databaseId.name + '/' +
        this._objectStore.name;
  }

  /**
   * @override
   */
  onattach() {
    super.onattach();
    this.listItemElement.addEventListener('contextmenu', this._handleContextMenuEvent.bind(this), true);
  }

  _handleContextMenuEvent(event) {
    var contextMenu = new UI.ContextMenu(event);
    contextMenu.appendItem(Common.UIString('Clear'), this._clearObjectStore.bind(this));
    contextMenu.show();
  }

  async _clearObjectStore() {
    await this._model.clearObjectStore(this._databaseId, this._objectStore.name);
    this.update(this._objectStore);
  }

  /**
   * @param {!Resources.IndexedDBModel.ObjectStore} objectStore
   */
  update(objectStore) {
    this._objectStore = objectStore;

    var indexNames = {};
    for (var indexName in this._objectStore.indexes) {
      var index = this._objectStore.indexes[indexName];
      indexNames[index.name] = true;
      if (!this._idbIndexTreeElements[index.name]) {
        var idbIndexTreeElement = new Resources.IDBIndexTreeElement(
            this._storagePanel, this._model, this._databaseId, this._objectStore, index);
        this._idbIndexTreeElements[index.name] = idbIndexTreeElement;
        this.appendChild(idbIndexTreeElement);
      }
      this._idbIndexTreeElements[index.name].update(index);
    }
    for (var indexName in this._idbIndexTreeElements) {
      if (!indexNames[indexName])
        this._indexRemoved(indexName);
    }
    for (var indexName in this._idbIndexTreeElements) {
      if (!indexNames[indexName]) {
        this.removeChild(this._idbIndexTreeElements[indexName]);
        delete this._idbIndexTreeElements[indexName];
      }
    }

    if (this.childCount())
      this.expand();

    if (this._view)
      this._view.update(this._objectStore, null);

    this._updateTooltip();
  }

  _updateTooltip() {
    var keyPathString = this._objectStore.keyPathString;
    var tooltipString = keyPathString !== null ? (Common.UIString('Key path: ') + keyPathString) : '';
    if (this._objectStore.autoIncrement)
      tooltipString += '\n' + Common.UIString('autoIncrement');
    this.tooltip = tooltipString;
  }

  /**
   * @override
   * @return {boolean}
   */
  onselect(selectedByUser) {
    super.onselect(selectedByUser);
    if (!this._view)
      this._view = new Resources.IDBDataView(this._model, this._databaseId, this._objectStore, null);

    this.showView(this._view);
    return false;
  }

  /**
   * @param {string} indexName
   */
  _indexRemoved(indexName) {
    var indexTreeElement = this._idbIndexTreeElements[indexName];
    indexTreeElement.clear();
    this.removeChild(indexTreeElement);
    delete this._idbIndexTreeElements[indexName];
  }

  clear() {
    for (var indexName in this._idbIndexTreeElements)
      this._indexRemoved(indexName);
    if (this._view)
      this._view.clear();
  }
};

/**
 * @unrestricted
 */
Resources.IDBIndexTreeElement = class extends Resources.BaseStorageTreeElement {
  /**
   * @param {!Resources.ResourcesPanel} storagePanel
   * @param {!Resources.IndexedDBModel} model
   * @param {!Resources.IndexedDBModel.DatabaseId} databaseId
   * @param {!Resources.IndexedDBModel.ObjectStore} objectStore
   * @param {!Resources.IndexedDBModel.Index} index
   */
  constructor(storagePanel, model, databaseId, objectStore, index) {
    super(storagePanel, index.name, false);
    this._model = model;
    this._databaseId = databaseId;
    this._objectStore = objectStore;
    this._index = index;
  }

  get itemURL() {
    return 'indexedDB://' + this._databaseId.securityOrigin + '/' + this._databaseId.name + '/' +
        this._objectStore.name + '/' + this._index.name;
  }

  /**
   * @param {!Resources.IndexedDBModel.Index} index
   */
  update(index) {
    this._index = index;

    if (this._view)
      this._view.update(this._index);

    this._updateTooltip();
  }

  _updateTooltip() {
    var tooltipLines = [];
    var keyPathString = this._index.keyPathString;
    tooltipLines.push(Common.UIString('Key path: ') + keyPathString);
    if (this._index.unique)
      tooltipLines.push(Common.UIString('unique'));
    if (this._index.multiEntry)
      tooltipLines.push(Common.UIString('multiEntry'));
    this.tooltip = tooltipLines.join('\n');
  }

  /**
   * @override
   * @return {boolean}
   */
  onselect(selectedByUser) {
    super.onselect(selectedByUser);
    if (!this._view)
      this._view = new Resources.IDBDataView(this._model, this._databaseId, this._objectStore, this._index);

    this.showView(this._view);
    return false;
  }

  clear() {
    if (this._view)
      this._view.clear();
  }
};

/**
 * @unrestricted
 */
Resources.DOMStorageTreeElement = class extends Resources.BaseStorageTreeElement {
  /**
   * @param {!Resources.ResourcesPanel} storagePanel
   * @param {!Resources.DOMStorage} domStorage
   */
  constructor(storagePanel, domStorage) {
    super(storagePanel, domStorage.securityOrigin ? domStorage.securityOrigin : Common.UIString('Local Files'), false);
    this._domStorage = domStorage;
    var icon = UI.Icon.create('mediumicon-table', 'resource-tree-item');
    this.setLeadingIcons([icon]);
  }

  get itemURL() {
    return 'storage://' + this._domStorage.securityOrigin + '/' +
        (this._domStorage.isLocalStorage ? 'local' : 'session');
  }

  /**
   * @override
   * @return {boolean}
   */
  onselect(selectedByUser) {
    super.onselect(selectedByUser);
    this._storagePanel.showDOMStorage(this._domStorage);
    return false;
  }

  /**
   * @override
   */
  onattach() {
    super.onattach();
    this.listItemElement.addEventListener('contextmenu', this._handleContextMenuEvent.bind(this), true);
  }

  _handleContextMenuEvent(event) {
    var contextMenu = new UI.ContextMenu(event);
    contextMenu.appendItem(Common.UIString('Clear'), () => this._domStorage.clear());
    contextMenu.show();
  }
};

Resources.CookieTreeElement = class extends Resources.BaseStorageTreeElement {
  /**
   * @param {!Resources.ResourcesPanel} storagePanel
   * @param {!SDK.ResourceTreeFrame} frame
   * @param {string} cookieDomain
   */
  constructor(storagePanel, frame, cookieDomain) {
    super(storagePanel, cookieDomain ? cookieDomain : Common.UIString('Local Files'), false);
    this._target = frame.resourceTreeModel().target();
    this._cookieDomain = cookieDomain;
    var icon = UI.Icon.create('mediumicon-cookie', 'resource-tree-item');
    this.setLeadingIcons([icon]);
  }

  get itemURL() {
    return 'cookies://' + this._cookieDomain;
  }

  /**
   * @override
   */
  onattach() {
    super.onattach();
    this.listItemElement.addEventListener('contextmenu', this._handleContextMenuEvent.bind(this), true);
  }

  /**
   * @param {!Event} event
   */
  _handleContextMenuEvent(event) {
    var contextMenu = new UI.ContextMenu(event);
    contextMenu.appendItem(
        Common.UIString('Clear'), () => this._storagePanel.clearCookies(this._target, this._cookieDomain));
    contextMenu.show();
  }

  /**
   * @override
   * @return {boolean}
   */
  onselect(selectedByUser) {
    super.onselect(selectedByUser);
    this._storagePanel.showCookies(this._target, this._cookieDomain);
    return false;
  }
};

/**
 * @unrestricted
 */
Resources.ApplicationCacheManifestTreeElement = class extends Resources.BaseStorageTreeElement {
  constructor(storagePanel, manifestURL) {
    var title = new Common.ParsedURL(manifestURL).displayName;
    super(storagePanel, title, false);
    this.tooltip = manifestURL;
    this._manifestURL = manifestURL;
  }

  get itemURL() {
    return 'appcache://' + this._manifestURL;
  }

  get manifestURL() {
    return this._manifestURL;
  }

  /**
   * @override
   * @return {boolean}
   */
  onselect(selectedByUser) {
    super.onselect(selectedByUser);
    this._storagePanel.showCategoryView(this._manifestURL);
    return false;
  }
};

/**
 * @unrestricted
 */
Resources.ApplicationCacheFrameTreeElement = class extends Resources.BaseStorageTreeElement {
  /**
   * @param {!Resources.ApplicationPanelSidebar} sidebar
   * @param {!SDK.ResourceTreeFrame} frame
   * @param {string} manifestURL
   */
  constructor(sidebar, frame, manifestURL) {
    super(sidebar._panel, '', false);
    this._sidebar = sidebar;
    this._frameId = frame.id;
    this._manifestURL = manifestURL;
    this._refreshTitles(frame);

    var icon = UI.Icon.create('largeicon-navigator-folder', 'navigator-tree-item');
    icon.classList.add('navigator-folder-tree-item');
    this.setLeadingIcons([icon]);
  }

  get itemURL() {
    return 'appcache://' + this._manifestURL + '/' + encodeURI(this.titleAsText());
  }

  get frameId() {
    return this._frameId;
  }

  get manifestURL() {
    return this._manifestURL;
  }

  /**
   * @param {!SDK.ResourceTreeFrame} frame
   */
  _refreshTitles(frame) {
    this.title = frame.displayName();
  }

  /**
   * @param {!SDK.ResourceTreeFrame} frame
   */
  frameNavigated(frame) {
    this._refreshTitles(frame);
  }

  /**
   * @override
   * @return {boolean}
   */
  onselect(selectedByUser) {
    super.onselect(selectedByUser);
    this._sidebar._showApplicationCache(this._frameId);
    return false;
  }
};

/**
 * @unrestricted
 */
Resources.StorageCategoryView = class extends UI.VBox {
  constructor() {
    super();

    this.element.classList.add('storage-view');
    this._emptyWidget = new UI.EmptyWidget('');
    this._emptyWidget.show(this.element);
  }

  setText(text) {
    this._emptyWidget.text = text;
  }
};
