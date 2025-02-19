/*
 * Copyright (C) 2011 Google Inc. All rights reserved.
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

/**
 * @unrestricted
 */
Profiler.HeapSnapshotGridNode = class extends DataGrid.DataGridNode {
  /**
   * @param {!Profiler.HeapSnapshotSortableDataGrid} tree
   * @param {boolean} hasChildren
   */
  constructor(tree, hasChildren) {
    super(null, hasChildren);
    this._dataGrid = tree;
    this._instanceCount = 0;

    this._savedChildren = null;
    /**
     * List of position ranges for all visible nodes: [startPos1, endPos1),...,[startPosN, endPosN)
     * Position is an item position in the provider.
     */
    this._retrievedChildrenRanges = [];

    /**
     * @type {?Profiler.HeapSnapshotGridNode.ChildrenProvider}
     */
    this._providerObject = null;
    this._reachableFromWindow = false;
  }

  /**
   * @param {!Array.<string>} fieldNames
   * @return {!HeapSnapshotModel.ComparatorConfig}
   */
  static createComparator(fieldNames) {
    return /** @type {!HeapSnapshotModel.ComparatorConfig} */ (
        {fieldName1: fieldNames[0], ascending1: fieldNames[1], fieldName2: fieldNames[2], ascending2: fieldNames[3]});
  }

  /**
   * @return {!Profiler.HeapSnapshotSortableDataGrid}
   */
  heapSnapshotDataGrid() {
    return this._dataGrid;
  }

  /**
   * @return {!Profiler.HeapSnapshotGridNode.ChildrenProvider}
   */
  createProvider() {
    throw new Error('Not implemented.');
  }

  /**
   * @return {?{snapshot:!Profiler.HeapSnapshotProxy, snapshotNodeIndex:number}}
   */
  retainersDataSource() {
    return null;
  }

  /**
   * @return {!Profiler.HeapSnapshotGridNode.ChildrenProvider}
   */
  _provider() {
    if (!this._providerObject)
      this._providerObject = this.createProvider();
    return this._providerObject;
  }

  /**
   * @override
   * @param {string} columnId
   * @return {!Element}
   */
  createCell(columnId) {
    var cell = super.createCell(columnId);
    if (this._searchMatched)
      cell.classList.add('highlight');
    return cell;
  }

  /**
   * @override
   */
  collapse() {
    super.collapse();
    this._dataGrid.updateVisibleNodes(true);
  }

  /**
   * @override
   */
  expand() {
    super.expand();
    this._dataGrid.updateVisibleNodes(true);
  }

  dispose() {
    if (this._providerObject)
      this._providerObject.dispose();
    for (var node = this.children[0]; node; node = node.traverseNextNode(true, this, true)) {
      if (node.dispose)
        node.dispose();
    }
  }

  /**
   * @param {!SDK.HeapProfilerModel} heapProfilerModel
   * @param {string} objectGroupName
   * @return {!Promise<!SDK.RemoteObject>}
   */
  queryObjectContent(heapProfilerModel, objectGroupName) {
  }

  /**
   * @param {number} num
   * @return {string}
   */
  _toPercentString(num) {
    return num.toFixed(0) + '\xa0%';  // \xa0 is a non-breaking space.
  }

  /**
   * @param {number} distance
   * @return {string}
   */
  _toUIDistance(distance) {
    var baseSystemDistance = HeapSnapshotModel.baseSystemDistance;
    return distance >= 0 && distance < baseSystemDistance ? Common.UIString('%d', distance) : Common.UIString('\u2212');
  }

  /**
   * @return {!Array.<!DataGrid.DataGridNode>}
   */
  allChildren() {
    return this._dataGrid.allChildren(this);
  }

  /**
   * @param {number} index
   */
  removeChildByIndex(index) {
    this._dataGrid.removeChildByIndex(this, index);
  }

  /**
   * @param {number} nodePosition
   * @return {?DataGrid.DataGridNode}
   */
  childForPosition(nodePosition) {
    var indexOfFirstChildInRange = 0;
    for (var i = 0; i < this._retrievedChildrenRanges.length; i++) {
      var range = this._retrievedChildrenRanges[i];
      if (range.from <= nodePosition && nodePosition < range.to) {
        var childIndex = indexOfFirstChildInRange + nodePosition - range.from;
        return this.allChildren()[childIndex];
      }
      indexOfFirstChildInRange += range.to - range.from + 1;
    }
    return null;
  }

  /**
   * @param {string} columnId
   * @return {!Element}
   */
  _createValueCell(columnId) {
    var cell = createElement('td');
    cell.className = 'numeric-column';
    if (this.dataGrid.snapshot.totalSize !== 0) {
      var div = createElement('div');
      var valueSpan = createElement('span');
      valueSpan.textContent = this.data[columnId];
      div.appendChild(valueSpan);
      var percentColumn = columnId + '-percent';
      if (percentColumn in this.data) {
        var percentSpan = createElement('span');
        percentSpan.className = 'percent-column';
        percentSpan.textContent = this.data[percentColumn];
        div.appendChild(percentSpan);
        div.classList.add('profile-multiple-values');
      }
      cell.appendChild(div);
    }
    return cell;
  }

  /**
   * @override
   */
  populate() {
    if (this._populated)
      return;
    this._populated = true;
    this._provider().sortAndRewind(this.comparator()).then(() => this._populateChildren());
  }

  /**
   * @return {!Promise}
   */
  expandWithoutPopulate() {
    // Make sure default populate won't take action.
    this._populated = true;
    this.expand();
    return this._provider().sortAndRewind(this.comparator());
  }

  /**
   * @param {?number=} fromPosition
   * @param {?number=} toPosition
   * @return {!Promise}
   */
  _populateChildren(fromPosition, toPosition) {
    var afterPopulate;
    var promise = new Promise(resolve => afterPopulate = resolve);
    fromPosition = fromPosition || 0;
    toPosition = toPosition || fromPosition + this._dataGrid.defaultPopulateCount();
    var firstNotSerializedPosition = fromPosition;
    serializeNextChunk.call(this);
    return promise;

    /**
     * @this {Profiler.HeapSnapshotGridNode}
     */
    function serializeNextChunk() {
      if (firstNotSerializedPosition >= toPosition)
        return;
      var end = Math.min(firstNotSerializedPosition + this._dataGrid.defaultPopulateCount(), toPosition);
      this._provider().serializeItemsRange(firstNotSerializedPosition, end).then(childrenRetrieved.bind(this));
      firstNotSerializedPosition = end;
    }

    /**
     * @this {Profiler.HeapSnapshotGridNode}
     */
    function insertRetrievedChild(item, insertionIndex) {
      if (this._savedChildren) {
        var hash = this._childHashForEntity(item);
        if (hash in this._savedChildren) {
          this._dataGrid.insertChild(this, this._savedChildren[hash], insertionIndex);
          return;
        }
      }
      this._dataGrid.insertChild(this, this._createChildNode(item), insertionIndex);
    }

    /**
     * @this {Profiler.HeapSnapshotGridNode}
     */
    function insertShowMoreButton(from, to, insertionIndex) {
      var button = new DataGrid.ShowMoreDataGridNode(
          this._populateChildren.bind(this), from, to, this._dataGrid.defaultPopulateCount());
      this._dataGrid.insertChild(this, button, insertionIndex);
    }

    /**
     * @param {!HeapSnapshotModel.ItemsRange} itemsRange
     * @this {Profiler.HeapSnapshotGridNode}
     */
    function childrenRetrieved(itemsRange) {
      var itemIndex = 0;
      var itemPosition = itemsRange.startPosition;
      var items = itemsRange.items;
      var insertionIndex = 0;

      if (!this._retrievedChildrenRanges.length) {
        if (itemsRange.startPosition > 0) {
          this._retrievedChildrenRanges.push({from: 0, to: 0});
          insertShowMoreButton.call(this, 0, itemsRange.startPosition, insertionIndex++);
        }
        this._retrievedChildrenRanges.push({from: itemsRange.startPosition, to: itemsRange.endPosition});
        for (var i = 0, l = items.length; i < l; ++i)
          insertRetrievedChild.call(this, items[i], insertionIndex++);
        if (itemsRange.endPosition < itemsRange.totalLength)
          insertShowMoreButton.call(this, itemsRange.endPosition, itemsRange.totalLength, insertionIndex++);
      } else {
        var rangeIndex = 0;
        var found = false;
        var range;
        while (rangeIndex < this._retrievedChildrenRanges.length) {
          range = this._retrievedChildrenRanges[rangeIndex];
          if (range.to >= itemPosition) {
            found = true;
            break;
          }
          insertionIndex += range.to - range.from;
          // Skip the button if there is one.
          if (range.to < itemsRange.totalLength)
            insertionIndex += 1;
          ++rangeIndex;
        }

        if (!found || itemsRange.startPosition < range.from) {
          // Update previous button.
          this.allChildren()[insertionIndex - 1].setEndPosition(itemsRange.startPosition);
          insertShowMoreButton.call(
              this, itemsRange.startPosition, found ? range.from : itemsRange.totalLength, insertionIndex);
          range = {from: itemsRange.startPosition, to: itemsRange.startPosition};
          if (!found)
            rangeIndex = this._retrievedChildrenRanges.length;
          this._retrievedChildrenRanges.splice(rangeIndex, 0, range);
        } else {
          insertionIndex += itemPosition - range.from;
        }
        // At this point insertionIndex is always an index before button or between nodes.
        // Also it is always true here that range.from <= itemPosition <= range.to

        // Stretch the range right bound to include all new items.
        while (range.to < itemsRange.endPosition) {
          // Skip already added nodes.
          var skipCount = range.to - itemPosition;
          insertionIndex += skipCount;
          itemIndex += skipCount;
          itemPosition = range.to;

          // We're at the position before button: ...<?node>x<button>
          var nextRange = this._retrievedChildrenRanges[rangeIndex + 1];
          var newEndOfRange = nextRange ? nextRange.from : itemsRange.totalLength;
          if (newEndOfRange > itemsRange.endPosition)
            newEndOfRange = itemsRange.endPosition;
          while (itemPosition < newEndOfRange) {
            insertRetrievedChild.call(this, items[itemIndex++], insertionIndex++);
            ++itemPosition;
          }

          // Merge with the next range.
          if (nextRange && newEndOfRange === nextRange.from) {
            range.to = nextRange.to;
            // Remove "show next" button if there is one.
            this.removeChildByIndex(insertionIndex);
            this._retrievedChildrenRanges.splice(rangeIndex + 1, 1);
          } else {
            range.to = newEndOfRange;
            // Remove or update next button.
            if (newEndOfRange === itemsRange.totalLength)
              this.removeChildByIndex(insertionIndex);
            else
              this.allChildren()[insertionIndex].setStartPosition(itemsRange.endPosition);
          }
        }
      }

      // TODO: fix this.
      this._instanceCount += items.length;
      if (firstNotSerializedPosition < toPosition) {
        serializeNextChunk.call(this);
        return;
      }

      if (this.expanded)
        this._dataGrid.updateVisibleNodes(true);
      afterPopulate();
      this.dispatchEventToListeners(Profiler.HeapSnapshotGridNode.Events.PopulateComplete);
    }
  }

  _saveChildren() {
    this._savedChildren = null;
    var children = this.allChildren();
    for (var i = 0, l = children.length; i < l; ++i) {
      var child = children[i];
      if (!child.expanded)
        continue;
      if (!this._savedChildren)
        this._savedChildren = {};
      this._savedChildren[this._childHashForNode(child)] = child;
    }
  }

  async sort() {
    this._dataGrid.recursiveSortingEnter();

    await this._provider().sortAndRewind(this.comparator());

    this._saveChildren();
    this._dataGrid.removeAllChildren(this);
    this._retrievedChildrenRanges = [];
    var instanceCount = this._instanceCount;
    this._instanceCount = 0;

    await this._populateChildren(0, instanceCount);

    for (var child of this.allChildren()) {
      if (child.expanded)
        child.sort();
    }
    this._dataGrid.recursiveSortingLeave();
  }
};

/** @enum {symbol} */
Profiler.HeapSnapshotGridNode.Events = {
  PopulateComplete: Symbol('PopulateComplete')
};

/**
 * @interface
 */
Profiler.HeapSnapshotGridNode.ChildrenProvider = function() {};

Profiler.HeapSnapshotGridNode.ChildrenProvider.prototype = {
  dispose() {},

  /**
   * @param {number} snapshotObjectId
   * @return {!Promise<number>}
   */
  nodePosition(snapshotObjectId) {},

  /**
   * @return {!Promise<boolean>}
   */
  isEmpty() {},

  /**
   * @param {number} startPosition
   * @param {number} endPosition
   * @return {!Promise<!HeapSnapshotModel.ItemsRange>}
   */
  serializeItemsRange(startPosition, endPosition) {},

  /**
   * @param {!HeapSnapshotModel.ComparatorConfig} comparator
   * @return {!Promise<?>}
   */
  sortAndRewind(comparator) {}
};

/**
 * @unrestricted
 */
Profiler.HeapSnapshotGenericObjectNode = class extends Profiler.HeapSnapshotGridNode {
  /**
   * @param {!Profiler.HeapSnapshotSortableDataGrid} dataGrid
   * @param {!HeapSnapshotModel.Node} node
   */
  constructor(dataGrid, node) {
    super(dataGrid, false);
    // node is null for DataGrid root nodes.
    if (!node)
      return;
    this._name = node.name;
    this._type = node.type;
    this._distance = node.distance;
    this._shallowSize = node.selfSize;
    this._retainedSize = node.retainedSize;
    this.snapshotNodeId = node.id;
    this.snapshotNodeIndex = node.nodeIndex;
    if (this._type === 'string') {
      this._reachableFromWindow = true;
    } else if (this._type === 'object' && this._name.startsWith('Window')) {
      this._name = this.shortenWindowURL(this._name, false);
      this._reachableFromWindow = true;
    } else if (node.canBeQueried) {
      this._reachableFromWindow = true;
    }
    if (node.detachedDOMTreeNode)
      this.detachedDOMTreeNode = true;

    var snapshot = dataGrid.snapshot;
    var shallowSizePercent = this._shallowSize / snapshot.totalSize * 100.0;
    var retainedSizePercent = this._retainedSize / snapshot.totalSize * 100.0;
    this.data = {
      'distance': this._toUIDistance(this._distance),
      'shallowSize': Number.withThousandsSeparator(this._shallowSize),
      'retainedSize': Number.withThousandsSeparator(this._retainedSize),
      'shallowSize-percent': this._toPercentString(shallowSizePercent),
      'retainedSize-percent': this._toPercentString(retainedSizePercent)
    };
  }

  /**
   * @override
   * @return {?{snapshot:!Profiler.HeapSnapshotProxy, snapshotNodeIndex:number}}
   */
  retainersDataSource() {
    return {snapshot: this._dataGrid.snapshot, snapshotNodeIndex: this.snapshotNodeIndex};
  }

  /**
   * @override
   * @param {string} columnId
   * @return {!Element}
   */
  createCell(columnId) {
    var cell = columnId !== 'object' ? this._createValueCell(columnId) : this._createObjectCell();
    if (this._searchMatched)
      cell.classList.add('highlight');
    return cell;
  }

  /**
   * @return {!Element}
   */
  _createObjectCell() {
    var value = this._name;
    var valueStyle = 'object';
    switch (this._type) {
      case 'concatenated string':
      case 'string':
        value = '"' + value + '"';
        valueStyle = 'string';
        break;
      case 'regexp':
        value = '/' + value + '/';
        valueStyle = 'string';
        break;
      case 'closure':
        value = value + '()';
        valueStyle = 'function';
        break;
      case 'number':
        valueStyle = 'number';
        break;
      case 'hidden':
        valueStyle = 'null';
        break;
      case 'array':
        value = (value || '') + '[]';
        break;
    }
    if (this._reachableFromWindow)
      valueStyle += ' highlight';
    if (value === 'Object')
      value = '';
    if (this.detachedDOMTreeNode)
      valueStyle += ' detached-dom-tree-node';
    return this._createObjectCellWithValue(valueStyle, value);
  }

  _createObjectCellWithValue(valueStyle, value) {
    var cell = createElement('td');
    cell.className = 'object-column';
    var div = createElement('div');
    div.className = 'source-code event-properties';
    div.style.overflow = 'visible';

    this._prefixObjectCell(div);

    var valueSpan = createElement('span');
    valueSpan.className = 'value object-value-' + valueStyle;
    valueSpan.textContent = value;
    div.appendChild(valueSpan);

    var idSpan = createElement('span');
    idSpan.className = 'object-value-id';
    idSpan.textContent = ' @' + this.snapshotNodeId;
    div.appendChild(idSpan);

    cell.appendChild(div);
    cell.classList.add('disclosure');
    if (this.depth)
      cell.style.setProperty('padding-left', (this.depth * this.dataGrid.indentWidth) + 'px');
    cell.heapSnapshotNode = this;
    return cell;
  }

  _prefixObjectCell(div) {
  }

  /**
   * @override
   * @param {!SDK.HeapProfilerModel} heapProfilerModel
   * @param {string} objectGroupName
   * @return {!Promise<!SDK.RemoteObject>}
   */
  queryObjectContent(heapProfilerModel, objectGroupName) {
    var fulfill;
    var promise = new Promise(x => fulfill = x);

    /**
     * @param {?SDK.RemoteObject} object
     */
    function onResult(object) {
      fulfill(object || runtimeModel.createRemoteObjectFromPrimitiveValue(Common.UIString('Preview is not available')));
    }

    var runtimeModel = heapProfilerModel.runtimeModel();
    if (this._type === 'string')
      onResult(runtimeModel.createRemoteObjectFromPrimitiveValue(this._name));
    else if (!heapProfilerModel || !runtimeModel)
      onResult(null);
    else
      heapProfilerModel.objectForSnapshotObjectId(String(this.snapshotNodeId), objectGroupName).then(onResult);
    return promise;
  }

  async updateHasChildren() {
    var isEmpty = await this._provider().isEmpty();
    this.setHasChildren(!isEmpty);
  }

  /**
   * @param {string} fullName
   * @param {boolean} hasObjectId
   * @return {string}
   */
  shortenWindowURL(fullName, hasObjectId) {
    var startPos = fullName.indexOf('/');
    var endPos = hasObjectId ? fullName.indexOf('@') : fullName.length;
    if (startPos === -1 || endPos === -1)
      return fullName;
    var fullURL = fullName.substring(startPos + 1, endPos).trimLeft();
    var url = fullURL.trimURL();
    if (url.length > 40)
      url = url.trimMiddle(40);
    return fullName.substr(0, startPos + 2) + url + fullName.substr(endPos);
  }
};

/**
 * @unrestricted
 */
Profiler.HeapSnapshotObjectNode = class extends Profiler.HeapSnapshotGenericObjectNode {
  /**
   * @param {!Profiler.HeapSnapshotSortableDataGrid} dataGrid
   * @param {!Profiler.HeapSnapshotProxy} snapshot
   * @param {!HeapSnapshotModel.Edge} edge
   * @param {?Profiler.HeapSnapshotObjectNode} parentObjectNode
   */
  constructor(dataGrid, snapshot, edge, parentObjectNode) {
    super(dataGrid, edge.node);
    this._referenceName = edge.name;
    this._referenceType = edge.type;
    this._edgeIndex = edge.edgeIndex;
    this._snapshot = snapshot;

    this._parentObjectNode = parentObjectNode;
    this._cycledWithAncestorGridNode = this._findAncestorWithSameSnapshotNodeId();
    if (!this._cycledWithAncestorGridNode)
      this.updateHasChildren();

    var data = this.data;
    data['count'] = '';
    data['addedCount'] = '';
    data['removedCount'] = '';
    data['countDelta'] = '';
    data['addedSize'] = '';
    data['removedSize'] = '';
    data['sizeDelta'] = '';
  }

  /**
   * @override
   * @return {?{snapshot:!Profiler.HeapSnapshotProxy, snapshotNodeIndex:number}}
   */
  retainersDataSource() {
    return {snapshot: this._snapshot, snapshotNodeIndex: this.snapshotNodeIndex};
  }

  /**
   * @override
   * @return {!Profiler.HeapSnapshotProviderProxy}
   */
  createProvider() {
    return this._snapshot.createEdgesProvider(this.snapshotNodeIndex);
  }

  _findAncestorWithSameSnapshotNodeId() {
    var ancestor = this._parentObjectNode;
    while (ancestor) {
      if (ancestor.snapshotNodeId === this.snapshotNodeId)
        return ancestor;
      ancestor = ancestor._parentObjectNode;
    }
    return null;
  }

  /**
   * @param {!HeapSnapshotModel.Edge} item
   * @return {!Profiler.HeapSnapshotObjectNode}
   */
  _createChildNode(item) {
    return new Profiler.HeapSnapshotObjectNode(this._dataGrid, this._snapshot, item, this);
  }

  /**
   * @param {!HeapSnapshotModel.Edge} edge
   * @return {number}
   */
  _childHashForEntity(edge) {
    return edge.edgeIndex;
  }

  /**
   * @param {!Profiler.HeapSnapshotObjectNode} childNode
   * @return {number}
   */
  _childHashForNode(childNode) {
    return childNode._edgeIndex;
  }

  /**
   * @return {!HeapSnapshotModel.ComparatorConfig}
   */
  comparator() {
    var sortAscending = this._dataGrid.isSortOrderAscending();
    var sortColumnId = this._dataGrid.sortColumnId();
    var sortFields = {
      object: ['!edgeName', sortAscending, 'retainedSize', false],
      count: ['!edgeName', true, 'retainedSize', false],
      shallowSize: ['selfSize', sortAscending, '!edgeName', true],
      retainedSize: ['retainedSize', sortAscending, '!edgeName', true],
      distance: ['distance', sortAscending, '_name', true]
    }[sortColumnId] ||
        ['!edgeName', true, 'retainedSize', false];
    return Profiler.HeapSnapshotGridNode.createComparator(sortFields);
  }

  /**
   * @override
   */
  _prefixObjectCell(div) {
    var name = this._referenceName || '(empty)';
    var nameClass = 'name';
    switch (this._referenceType) {
      case 'context':
        nameClass = 'object-value-number';
        break;
      case 'internal':
      case 'hidden':
      case 'weak':
        nameClass = 'object-value-null';
        break;
      case 'element':
        name = '[' + name + ']';
        break;
    }

    if (this._cycledWithAncestorGridNode)
      div.className += ' cycled-ancessor-node';

    var nameSpan = createElement('span');
    nameSpan.className = nameClass;
    nameSpan.textContent = name;
    div.appendChild(nameSpan);

    var separatorSpan = createElement('span');
    separatorSpan.className = 'grayed';
    separatorSpan.textContent = this._edgeNodeSeparator();
    div.appendChild(separatorSpan);
  }

  /**
   * @return {string}
   */
  _edgeNodeSeparator() {
    return ' :: ';
  }
};

/**
 * @unrestricted
 */
Profiler.HeapSnapshotRetainingObjectNode = class extends Profiler.HeapSnapshotObjectNode {
  /**
   * @param {!Profiler.HeapSnapshotSortableDataGrid} dataGrid
   * @param {!Profiler.HeapSnapshotProxy} snapshot
   * @param {!HeapSnapshotModel.Edge} edge
   * @param {?Profiler.HeapSnapshotRetainingObjectNode} parentRetainingObjectNode
   */
  constructor(dataGrid, snapshot, edge, parentRetainingObjectNode) {
    super(dataGrid, snapshot, edge, parentRetainingObjectNode);
  }

  /**
   * @override
   * @return {!Profiler.HeapSnapshotProviderProxy}
   */
  createProvider() {
    return this._snapshot.createRetainingEdgesProvider(this.snapshotNodeIndex);
  }

  /**
   * @override
   * @param {!HeapSnapshotModel.Edge} item
   * @return {!Profiler.HeapSnapshotRetainingObjectNode}
   */
  _createChildNode(item) {
    return new Profiler.HeapSnapshotRetainingObjectNode(this._dataGrid, this._snapshot, item, this);
  }

  /**
   * @override
   * @return {string}
   */
  _edgeNodeSeparator() {
    return ' in ';
  }

  /**
   * @override
   */
  expand() {
    this._expandRetainersChain(20);
  }

  /**
   * @param {number} maxExpandLevels
   */
  _expandRetainersChain(maxExpandLevels) {
    if (!this._populated) {
      this.once(Profiler.HeapSnapshotGridNode.Events.PopulateComplete)
          .then(() => this._expandRetainersChain(maxExpandLevels));
      this.populate();
      return;
    }
    super.expand();
    if (--maxExpandLevels > 0 && this.children.length > 0) {
      var retainer = this.children[0];
      if (retainer._distance > 1) {
        retainer._expandRetainersChain(maxExpandLevels);
        return;
      }
    }
    this._dataGrid.dispatchEventToListeners(Profiler.HeapSnapshotRetainmentDataGrid.Events.ExpandRetainersComplete);
  }
};

/**
 * @unrestricted
 */
Profiler.HeapSnapshotInstanceNode = class extends Profiler.HeapSnapshotGenericObjectNode {
  /**
   * @param {!Profiler.HeapSnapshotSortableDataGrid} dataGrid
   * @param {!Profiler.HeapSnapshotProxy} snapshot
   * @param {!HeapSnapshotModel.Node} node
   * @param {boolean} isDeletedNode
   */
  constructor(dataGrid, snapshot, node, isDeletedNode) {
    super(dataGrid, node);
    this._baseSnapshotOrSnapshot = snapshot;
    this._isDeletedNode = isDeletedNode;
    this.updateHasChildren();

    var data = this.data;
    data['count'] = '';
    data['countDelta'] = '';
    data['sizeDelta'] = '';
    if (this._isDeletedNode) {
      data['addedCount'] = '';
      data['addedSize'] = '';
      data['removedCount'] = '\u2022';
      data['removedSize'] = Number.withThousandsSeparator(this._shallowSize);
    } else {
      data['addedCount'] = '\u2022';
      data['addedSize'] = Number.withThousandsSeparator(this._shallowSize);
      data['removedCount'] = '';
      data['removedSize'] = '';
    }
  }

  /**
   * @override
   * @return {?{snapshot:!Profiler.HeapSnapshotProxy, snapshotNodeIndex:number}}
   */
  retainersDataSource() {
    return {snapshot: this._baseSnapshotOrSnapshot, snapshotNodeIndex: this.snapshotNodeIndex};
  }

  /**
   * @override
   * @return {!Profiler.HeapSnapshotProviderProxy}
   */
  createProvider() {
    return this._baseSnapshotOrSnapshot.createEdgesProvider(this.snapshotNodeIndex);
  }

  /**
   * @param {!HeapSnapshotModel.Edge} item
   * @return {!Profiler.HeapSnapshotObjectNode}
   */
  _createChildNode(item) {
    return new Profiler.HeapSnapshotObjectNode(this._dataGrid, this._baseSnapshotOrSnapshot, item, null);
  }

  /**
   * @param {!HeapSnapshotModel.Edge} edge
   * @return {number}
   */
  _childHashForEntity(edge) {
    return edge.edgeIndex;
  }

  /**
   * @param {!Profiler.HeapSnapshotObjectNode} childNode
   * @return {number}
   */
  _childHashForNode(childNode) {
    return childNode._edgeIndex;
  }

  /**
   * @return {!HeapSnapshotModel.ComparatorConfig}
   */
  comparator() {
    var sortAscending = this._dataGrid.isSortOrderAscending();
    var sortColumnId = this._dataGrid.sortColumnId();
    var sortFields = {
      object: ['!edgeName', sortAscending, 'retainedSize', false],
      distance: ['distance', sortAscending, 'retainedSize', false],
      count: ['!edgeName', true, 'retainedSize', false],
      addedSize: ['selfSize', sortAscending, '!edgeName', true],
      removedSize: ['selfSize', sortAscending, '!edgeName', true],
      shallowSize: ['selfSize', sortAscending, '!edgeName', true],
      retainedSize: ['retainedSize', sortAscending, '!edgeName', true]
    }[sortColumnId] ||
        ['!edgeName', true, 'retainedSize', false];
    return Profiler.HeapSnapshotGridNode.createComparator(sortFields);
  }
};

/**
 * @unrestricted
 */
Profiler.HeapSnapshotConstructorNode = class extends Profiler.HeapSnapshotGridNode {
  /**
   * @param {!Profiler.HeapSnapshotConstructorsDataGrid} dataGrid
   * @param {string} className
   * @param {!HeapSnapshotModel.Aggregate} aggregate
   * @param {!HeapSnapshotModel.NodeFilter} nodeFilter
   */
  constructor(dataGrid, className, aggregate, nodeFilter) {
    super(dataGrid, aggregate.count > 0);
    this._name = className;
    this._nodeFilter = nodeFilter;
    this._distance = aggregate.distance;
    this._count = aggregate.count;
    this._shallowSize = aggregate.self;
    this._retainedSize = aggregate.maxRet;

    var snapshot = dataGrid.snapshot;
    var countPercent = this._count / snapshot.nodeCount * 100.0;
    var retainedSizePercent = this._retainedSize / snapshot.totalSize * 100.0;
    var shallowSizePercent = this._shallowSize / snapshot.totalSize * 100.0;

    this.data = {
      'object': className,
      'count': Number.withThousandsSeparator(this._count),
      'distance': this._toUIDistance(this._distance),
      'shallowSize': Number.withThousandsSeparator(this._shallowSize),
      'retainedSize': Number.withThousandsSeparator(this._retainedSize),
      'count-percent': this._toPercentString(countPercent),
      'shallowSize-percent': this._toPercentString(shallowSizePercent),
      'retainedSize-percent': this._toPercentString(retainedSizePercent)
    };
  }

  /**
   * @override
   * @return {!Profiler.HeapSnapshotProviderProxy}
   */
  createProvider() {
    return this._dataGrid.snapshot.createNodesProviderForClass(this._name, this._nodeFilter);
  }

  /**
   * @param {number} snapshotObjectId
   * @return {!Promise<!Array<!Profiler.HeapSnapshotGridNode>>}
   */
  async populateNodeBySnapshotObjectId(snapshotObjectId) {
    this._dataGrid.resetNameFilter();
    await this.expandWithoutPopulate();

    var nodePosition = await this._provider().nodePosition(snapshotObjectId);
    if (nodePosition === -1) {
      this.collapse();
      return [];
    }

    await this._populateChildren(nodePosition, null);

    var node = /** @type {?Profiler.HeapSnapshotGridNode} */ (this.childForPosition(nodePosition));
    return node ? [this, node] : [];
  }

  /**
   * @param {string} filterValue
   * @return {boolean}
   */
  filteredOut(filterValue) {
    return this._name.toLowerCase().indexOf(filterValue) === -1;
  }

  /**
   * @override
   * @param {string} columnId
   * @return {!Element}
   */
  createCell(columnId) {
    var cell = columnId !== 'object' ? this._createValueCell(columnId) : super.createCell(columnId);
    if (this._searchMatched)
      cell.classList.add('highlight');
    return cell;
  }

  /**
   * @param {!HeapSnapshotModel.Node} item
   * @return {!Profiler.HeapSnapshotInstanceNode}
   */
  _createChildNode(item) {
    return new Profiler.HeapSnapshotInstanceNode(this._dataGrid, this._dataGrid.snapshot, item, false);
  }

  /**
   * @return {!HeapSnapshotModel.ComparatorConfig}
   */
  comparator() {
    var sortAscending = this._dataGrid.isSortOrderAscending();
    var sortColumnId = this._dataGrid.sortColumnId();
    var sortFields = {
      object: ['name', sortAscending, 'id', true],
      distance: ['distance', sortAscending, 'retainedSize', false],
      count: ['name', true, 'id', true],
      shallowSize: ['selfSize', sortAscending, 'id', true],
      retainedSize: ['retainedSize', sortAscending, 'id', true]
    }[sortColumnId];
    return Profiler.HeapSnapshotGridNode.createComparator(sortFields);
  }

  /**
   * @param {!HeapSnapshotModel.Node} node
   * @return {number}
   */
  _childHashForEntity(node) {
    return node.id;
  }

  /**
   * @param {!Profiler.HeapSnapshotInstanceNode} childNode
   * @return {number}
   */
  _childHashForNode(childNode) {
    return childNode.snapshotNodeId;
  }
};

/**
 * @implements {Profiler.HeapSnapshotGridNode.ChildrenProvider}
 * @unrestricted
 */
Profiler.HeapSnapshotDiffNodesProvider = class {
  /**
   * @param {!Profiler.HeapSnapshotProviderProxy} addedNodesProvider
   * @param {!Profiler.HeapSnapshotProviderProxy} deletedNodesProvider
   * @param {number} addedCount
   * @param {number} removedCount
   */
  constructor(addedNodesProvider, deletedNodesProvider, addedCount, removedCount) {
    this._addedNodesProvider = addedNodesProvider;
    this._deletedNodesProvider = deletedNodesProvider;
    this._addedCount = addedCount;
    this._removedCount = removedCount;
  }

  /**
   * @override
   */
  dispose() {
    this._addedNodesProvider.dispose();
    this._deletedNodesProvider.dispose();
  }

  /**
   * @override
   * @param {number} snapshotObjectId
   * @return {!Promise<number>}
   */
  nodePosition(snapshotObjectId) {
    throw new Error('Unreachable');
  }

  /**
   * @override
   * @return {!Promise<boolean>}
   */
  isEmpty() {
    return Promise.resolve(false);
  }

  /**
   * @override
   * @param {number} beginPosition
   * @param {number} endPosition
   * @return {!Promise<!HeapSnapshotModel.ItemsRange>}
   */
  async serializeItemsRange(beginPosition, endPosition) {
    var itemsRange;
    var addedItems;
    if (beginPosition < this._addedCount) {
      itemsRange = await this._addedNodesProvider.serializeItemsRange(beginPosition, endPosition);

      for (var item of itemsRange.items)
        item.isAddedNotRemoved = true;

      if (itemsRange.endPosition >= endPosition) {
        itemsRange.totalLength = this._addedCount + this._removedCount;
        return itemsRange;
      }

      addedItems = itemsRange;
      itemsRange = await this._deletedNodesProvider.serializeItemsRange(0, endPosition - itemsRange.endPosition);
    } else {
      addedItems = new HeapSnapshotModel.ItemsRange(0, 0, 0, []);
      itemsRange = await this._deletedNodesProvider.serializeItemsRange(
          beginPosition - this._addedCount, endPosition - this._addedCount);
    }

    if (!addedItems.items.length)
      addedItems.startPosition = this._addedCount + itemsRange.startPosition;
    for (var item of itemsRange.items)
      item.isAddedNotRemoved = false;
    addedItems.items.pushAll(itemsRange.items);
    addedItems.endPosition = this._addedCount + itemsRange.endPosition;
    addedItems.totalLength = this._addedCount + this._removedCount;
    return addedItems;
  }

  /**
   * @override
   * @param {!HeapSnapshotModel.ComparatorConfig} comparator
   * @return {!Promise}
   */
  async sortAndRewind(comparator) {
    await this._addedNodesProvider.sortAndRewind(comparator);
    await this._deletedNodesProvider.sortAndRewind(comparator);
  }
};

/**
 * @unrestricted
 */
Profiler.HeapSnapshotDiffNode = class extends Profiler.HeapSnapshotGridNode {
  /**
   * @param {!Profiler.HeapSnapshotDiffDataGrid} dataGrid
   * @param {string} className
   * @param {!HeapSnapshotModel.DiffForClass} diffForClass
   */
  constructor(dataGrid, className, diffForClass) {
    super(dataGrid, true);
    this._name = className;
    this._addedCount = diffForClass.addedCount;
    this._removedCount = diffForClass.removedCount;
    this._countDelta = diffForClass.countDelta;
    this._addedSize = diffForClass.addedSize;
    this._removedSize = diffForClass.removedSize;
    this._sizeDelta = diffForClass.sizeDelta;
    this._deletedIndexes = diffForClass.deletedIndexes;
    this.data = {
      'object': className,
      'addedCount': Number.withThousandsSeparator(this._addedCount),
      'removedCount': Number.withThousandsSeparator(this._removedCount),
      'countDelta': this._signForDelta(this._countDelta) + Number.withThousandsSeparator(Math.abs(this._countDelta)),
      'addedSize': Number.withThousandsSeparator(this._addedSize),
      'removedSize': Number.withThousandsSeparator(this._removedSize),
      'sizeDelta': this._signForDelta(this._sizeDelta) + Number.withThousandsSeparator(Math.abs(this._sizeDelta))
    };
  }

  /**
   * @override
   * @return {!Profiler.HeapSnapshotDiffNodesProvider}
   */
  createProvider() {
    var tree = this._dataGrid;
    return new Profiler.HeapSnapshotDiffNodesProvider(
        tree.snapshot.createAddedNodesProvider(tree.baseSnapshot.uid, this._name),
        tree.baseSnapshot.createDeletedNodesProvider(this._deletedIndexes), this._addedCount, this._removedCount);
  }

  /**
   * @override
   * @param {string} columnId
   * @return {!Element}
   */
  createCell(columnId) {
    var cell = super.createCell(columnId);
    if (columnId !== 'object')
      cell.classList.add('numeric-column');
    return cell;
  }

  /**
   * @param {!HeapSnapshotModel.Node} item
   * @return {!Profiler.HeapSnapshotInstanceNode}
   */
  _createChildNode(item) {
    if (item.isAddedNotRemoved)
      return new Profiler.HeapSnapshotInstanceNode(this._dataGrid, this._dataGrid.snapshot, item, false);
    else
      return new Profiler.HeapSnapshotInstanceNode(this._dataGrid, this._dataGrid.baseSnapshot, item, true);
  }

  /**
   * @param {!HeapSnapshotModel.Node} node
   * @return {number}
   */
  _childHashForEntity(node) {
    return node.id;
  }

  /**
   * @param {!Profiler.HeapSnapshotInstanceNode} childNode
   * @return {number}
   */
  _childHashForNode(childNode) {
    return childNode.snapshotNodeId;
  }

  /**
   * @return {!HeapSnapshotModel.ComparatorConfig}
   */
  comparator() {
    var sortAscending = this._dataGrid.isSortOrderAscending();
    var sortColumnId = this._dataGrid.sortColumnId();
    var sortFields = {
      object: ['name', sortAscending, 'id', true],
      addedCount: ['name', true, 'id', true],
      removedCount: ['name', true, 'id', true],
      countDelta: ['name', true, 'id', true],
      addedSize: ['selfSize', sortAscending, 'id', true],
      removedSize: ['selfSize', sortAscending, 'id', true],
      sizeDelta: ['selfSize', sortAscending, 'id', true]
    }[sortColumnId];
    return Profiler.HeapSnapshotGridNode.createComparator(sortFields);
  }

  /**
   * @param {string} filterValue
   * @return {boolean}
   */
  filteredOut(filterValue) {
    return this._name.toLowerCase().indexOf(filterValue) === -1;
  }

  _signForDelta(delta) {
    if (delta === 0)
      return '';
    if (delta > 0)
      return '+';
    else
      return '\u2212';  // Math minus sign, same width as plus.
  }
};

/**
 * @unrestricted
 */
Profiler.AllocationGridNode = class extends Profiler.HeapSnapshotGridNode {
  /**
   * @param {!Profiler.AllocationDataGrid} dataGrid
   * @param {!HeapSnapshotModel.SerializedAllocationNode} data
   */
  constructor(dataGrid, data) {
    super(dataGrid, data.hasChildren);
    this._populated = false;
    this._allocationNode = data;
    this.data = {
      'liveCount': Number.withThousandsSeparator(data.liveCount),
      'count': Number.withThousandsSeparator(data.count),
      'liveSize': Number.withThousandsSeparator(data.liveSize),
      'size': Number.withThousandsSeparator(data.size),
      'name': data.name
    };
  }

  /**
   * @override
   */
  populate() {
    if (this._populated)
      return;
    this._doPopulate();
  }

  async _doPopulate() {
    this._populated = true;

    var callers = await this._dataGrid.snapshot.allocationNodeCallers(this._allocationNode.id);

    var callersChain = callers.nodesWithSingleCaller;
    var parentNode = this;
    var dataGrid = /** @type {!Profiler.AllocationDataGrid} */ (this._dataGrid);
    for (var caller of callersChain) {
      var child = new Profiler.AllocationGridNode(dataGrid, caller);
      dataGrid.appendNode(parentNode, child);
      parentNode = child;
      parentNode._populated = true;
      if (this.expanded)
        parentNode.expand();
    }

    var callersBranch = callers.branchingCallers;
    callersBranch.sort(this._dataGrid._createComparator());
    for (var caller of callersBranch)
      dataGrid.appendNode(parentNode, new Profiler.AllocationGridNode(dataGrid, caller));
    dataGrid.updateVisibleNodes(true);
  }

  /**
   * @override
   */
  expand() {
    super.expand();
    if (this.children.length === 1)
      this.children[0].expand();
  }

  /**
   * @override
   * @param {string} columnId
   * @return {!Element}
   */
  createCell(columnId) {
    if (columnId !== 'name')
      return this._createValueCell(columnId);

    var cell = super.createCell(columnId);
    var allocationNode = this._allocationNode;
    var heapProfilerModel = this._dataGrid.heapProfilerModel();
    if (allocationNode.scriptId) {
      var linkifier = this._dataGrid._linkifier;
      var urlElement = linkifier.linkifyScriptLocation(
          heapProfilerModel ? heapProfilerModel.target() : null, String(allocationNode.scriptId),
          allocationNode.scriptName, allocationNode.line - 1, allocationNode.column - 1, 'profile-node-file');
      urlElement.style.maxWidth = '75%';
      cell.insertBefore(urlElement, cell.firstChild);
    }
    return cell;
  }

  /**
   * @return {number}
   */
  allocationNodeId() {
    return this._allocationNode.id;
  }
};
