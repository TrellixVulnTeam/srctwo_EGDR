/*
 * Copyright (C) 2007 Apple Inc.  All rights reserved.
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
 * @unrestricted
 */
UI.TreeOutline = class extends Common.Object {
  constructor() {
    super();
    this._createRootElement();

    /** @type {?UI.TreeElement} */
    this.selectedTreeElement = null;
    this.expandTreeElementsWhenArrowing = false;
    /** @type {?function(!UI.TreeElement, !UI.TreeElement):number} */
    this._comparator = null;

    this.contentElement = this._rootElement._childrenListNode;
    this.contentElement.addEventListener('keydown', this._treeKeyDown.bind(this), true);

    this._focusable = true;
    this.setFocusable(this._focusable);
    if (this._focusable)
      this.contentElement.setAttribute('tabIndex', -1);
    this.element = this.contentElement;
    UI.ARIAUtils.markAsTree(this.element);

    // Adjust to allow computing margin-left for the selection element.
    // Check the padding-left for the li element for correct value.
    this._paddingSize = 0;
  }

  _createRootElement() {
    this._rootElement = new UI.TreeElement();
    this._rootElement.treeOutline = this;
    this._rootElement.root = true;
    this._rootElement.selectable = false;
    this._rootElement.expanded = true;
    this._rootElement._childrenListNode.classList.remove('children');
  }

  /**
   * @return {!UI.TreeElement}
   */
  rootElement() {
    return this._rootElement;
  }

  /**
   * @return {?UI.TreeElement}
   */
  firstChild() {
    return this._rootElement.firstChild();
  }

  /**
   * @param {!UI.TreeElement} child
   */
  appendChild(child) {
    this._rootElement.appendChild(child);
  }

  /**
   * @param {!UI.TreeElement} child
   * @param {number} index
   */
  insertChild(child, index) {
    this._rootElement.insertChild(child, index);
  }

  /**
   * @param {!UI.TreeElement} child
   */
  removeChild(child) {
    this._rootElement.removeChild(child);
  }

  removeChildren() {
    this._rootElement.removeChildren();
  }

  /**
   * @param {number} x
   * @param {number} y
   * @return {?UI.TreeElement}
   */
  treeElementFromPoint(x, y) {
    var node = this.contentElement.ownerDocument.deepElementFromPoint(x, y);
    if (!node)
      return null;

    var listNode = node.enclosingNodeOrSelfWithNodeNameInArray(['ol', 'li']);
    if (listNode)
      return listNode.parentTreeElement || listNode.treeElement;
    return null;
  }

  /**
   * @param {?Event} event
   * @return {?UI.TreeElement}
   */
  treeElementFromEvent(event) {
    return event ? this.treeElementFromPoint(event.pageX, event.pageY) : null;
  }

  /**
   * @param {?function(!UI.TreeElement, !UI.TreeElement):number} comparator
   */
  setComparator(comparator) {
    this._comparator = comparator;
  }

  /**
   * @param {boolean} focusable
   */
  setFocusable(focusable) {
    if (focusable) {
      this._focusable = true;
      this.contentElement.setAttribute('tabIndex', -1);
      if (this.selectedTreeElement)
        this.selectedTreeElement._setFocusable(true);
    } else {
      this._focusable = false;
      this.contentElement.removeAttribute('tabIndex');
      if (this.selectedTreeElement)
        this.selectedTreeElement._setFocusable(false);
    }
  }

  focus() {
    if (this.selectedTreeElement)
      this.selectedTreeElement.listItemElement.focus();
    else
      this.contentElement.focus();
  }

  /**
   * @param {!UI.TreeElement} element
   */
  _bindTreeElement(element) {
    if (element.treeOutline)
      console.error('Binding element for the second time: ' + new Error().stack);
    element.treeOutline = this;
    element.onbind();
  }

  /**
   * @param {!UI.TreeElement} element
   */
  _unbindTreeElement(element) {
    if (!element.treeOutline)
      console.error('Unbinding element that was not bound: ' + new Error().stack);

    element.deselect();
    element.onunbind();
    element.treeOutline = null;
  }

  /**
   * @return {boolean}
   */
  selectPrevious() {
    var nextSelectedElement = this.selectedTreeElement.traversePreviousTreeElement(true);
    while (nextSelectedElement && !nextSelectedElement.selectable)
      nextSelectedElement = nextSelectedElement.traversePreviousTreeElement(!this.expandTreeElementsWhenArrowing);
    if (!nextSelectedElement)
      return false;
    nextSelectedElement.select(false, true);
    return true;
  }

  /**
   * @return {boolean}
   */
  selectNext() {
    var nextSelectedElement = this.selectedTreeElement.traverseNextTreeElement(true);
    while (nextSelectedElement && !nextSelectedElement.selectable)
      nextSelectedElement = nextSelectedElement.traverseNextTreeElement(!this.expandTreeElementsWhenArrowing);
    if (!nextSelectedElement)
      return false;
    nextSelectedElement.select(false, true);
    return true;
  }

  /**
   * @param {number} paddingSize
   */
  setPaddingSize(paddingSize) {
    this._paddingSize = paddingSize;
  }

  /**
   * @param {!Event} event
   */
  _treeKeyDown(event) {
    if (!this.selectedTreeElement || event.target !== this.selectedTreeElement.listItemElement || event.shiftKey ||
        event.metaKey || event.ctrlKey)
      return;

    var handled = false;
    if (event.key === 'ArrowUp' && !event.altKey) {
      handled = this.selectPrevious();
    } else if (event.key === 'ArrowDown' && !event.altKey) {
      handled = this.selectNext();
    } else if (event.key === 'ArrowLeft') {
      handled = this.selectedTreeElement.collapseOrAscend(event.altKey);
    } else if (event.key === 'ArrowRight') {
      if (!this.selectedTreeElement.revealed()) {
        this.selectedTreeElement.reveal();
        handled = true;
      } else {
        handled = this.selectedTreeElement.descendOrExpand(event.altKey);
      }
    } else if (event.keyCode === 8 /* Backspace */ || event.keyCode === 46 /* Delete */) {
      handled = this.selectedTreeElement.ondelete();
    } else if (isEnterKey(event)) {
      handled = this.selectedTreeElement.onenter();
    } else if (event.keyCode === UI.KeyboardShortcut.Keys.Space.code) {
      handled = this.selectedTreeElement.onspace();
    }

    if (handled)
      event.consume(true);
  }

  /**
   * @param {!UI.TreeElement} treeElement
   * @param {boolean} center
   */
  _deferredScrollIntoView(treeElement, center) {
    if (!this._treeElementToScrollIntoView)
      this.element.window().requestAnimationFrame(deferredScrollIntoView.bind(this));
    this._treeElementToScrollIntoView = treeElement;
    this._centerUponScrollIntoView = center;
    /**
     * @this {UI.TreeOutline}
     */
    function deferredScrollIntoView() {
      this._treeElementToScrollIntoView.listItemElement.scrollIntoViewIfNeeded(this._centerUponScrollIntoView);
      delete this._treeElementToScrollIntoView;
      delete this._centerUponScrollIntoView;
    }
  }
};

/** @enum {symbol} */
UI.TreeOutline.Events = {
  ElementAttached: Symbol('ElementAttached'),
  ElementExpanded: Symbol('ElementExpanded'),
  ElementCollapsed: Symbol('ElementCollapsed'),
  ElementSelected: Symbol('ElementSelected')
};

/**
 * @unrestricted
 */
UI.TreeOutlineInShadow = class extends UI.TreeOutline {
  constructor() {
    super();
    this.contentElement.classList.add('tree-outline');

    // Redefine element to the external one.
    this.element = createElement('div');
    this._shadowRoot = UI.createShadowRootWithCoreStyles(this.element, 'ui/treeoutline.css');
    this._disclosureElement = this._shadowRoot.createChild('div', 'tree-outline-disclosure');
    this._disclosureElement.appendChild(this.contentElement);
    this._renderSelection = true;
  }

  /**
   * @param {string} cssFile
   */
  registerRequiredCSS(cssFile) {
    UI.appendStyle(this._shadowRoot, cssFile);
  }

  hideOverflow() {
    this._disclosureElement.classList.add('tree-outline-disclosure-hide-overflow');
  }

  makeDense() {
    this.contentElement.classList.add('tree-outline-dense');
  }
};

/**
 * @unrestricted
 */
UI.TreeElement = class {
  /**
   * @param {(string|!Node)=} title
   * @param {boolean=} expandable
   */
  constructor(title, expandable) {
    /** @type {?UI.TreeOutline} */
    this.treeOutline = null;
    this.parent = null;
    this.previousSibling = null;
    this.nextSibling = null;
    this._boundOnFocus = this._onFocus.bind(this);
    this._boundOnBlur = this._onBlur.bind(this);

    this._listItemNode = createElement('li');
    this._titleElement = this._listItemNode.createChild('span', 'tree-element-title');
    this._listItemNode.treeElement = this;
    if (title)
      this.title = title;
    this._listItemNode.addEventListener('mousedown', this._handleMouseDown.bind(this), false);
    this._listItemNode.addEventListener('click', this._treeElementToggled.bind(this), false);
    this._listItemNode.addEventListener('dblclick', this._handleDoubleClick.bind(this), false);
    UI.ARIAUtils.markAsTreeitem(this._listItemNode);

    this._childrenListNode = createElement('ol');
    this._childrenListNode.parentTreeElement = this;
    this._childrenListNode.classList.add('children');
    UI.ARIAUtils.markAsGroup(this._childrenListNode);

    this._hidden = false;
    this._selectable = true;
    this.expanded = false;
    this.selected = false;
    this.setExpandable(expandable || false);
    this._collapsible = true;
  }

  /**
   * @param {?UI.TreeElement} ancestor
   * @return {boolean}
   */
  hasAncestor(ancestor) {
    if (!ancestor)
      return false;

    var currentNode = this.parent;
    while (currentNode) {
      if (ancestor === currentNode)
        return true;
      currentNode = currentNode.parent;
    }

    return false;
  }

  /**
   * @param {?UI.TreeElement} ancestor
   * @return {boolean}
   */
  hasAncestorOrSelf(ancestor) {
    return this === ancestor || this.hasAncestor(ancestor);
  }

  /**
   * @return {!Array.<!UI.TreeElement>}
   */
  children() {
    return this._children || [];
  }

  /**
   * @return {number}
   */
  childCount() {
    return this._children ? this._children.length : 0;
  }

  /**
   * @return {?UI.TreeElement}
   */
  firstChild() {
    return this._children ? this._children[0] : null;
  }

  /**
   * @return {?UI.TreeElement}
   */
  lastChild() {
    return this._children ? this._children[this._children.length - 1] : null;
  }

  /**
   * @param {number} index
   * @return {?UI.TreeElement}
   */
  childAt(index) {
    return this._children ? this._children[index] : null;
  }

  /**
   * @param {!UI.TreeElement} child
   * @return {number}
   */
  indexOfChild(child) {
    return this._children ? this._children.indexOf(child) : -1;
  }

  /**
   * @param {!UI.TreeElement} child
   */
  appendChild(child) {
    if (!this._children)
      this._children = [];

    var insertionIndex;
    if (this.treeOutline && this.treeOutline._comparator)
      insertionIndex = this._children.lowerBound(child, this.treeOutline._comparator);
    else
      insertionIndex = this._children.length;
    this.insertChild(child, insertionIndex);
  }

  /**
   * @param {!UI.TreeElement} child
   * @param {number} index
   */
  insertChild(child, index) {
    if (!this._children)
      this._children = [];

    if (!child)
      throw 'child can\'t be undefined or null';

    console.assert(
        !child.parent, 'Attempting to insert a child that is already in the tree, reparenting is not supported.');

    var previousChild = (index > 0 ? this._children[index - 1] : null);
    if (previousChild) {
      previousChild.nextSibling = child;
      child.previousSibling = previousChild;
    } else {
      child.previousSibling = null;
    }

    var nextChild = this._children[index];
    if (nextChild) {
      nextChild.previousSibling = child;
      child.nextSibling = nextChild;
    } else {
      child.nextSibling = null;
    }

    this._children.splice(index, 0, child);

    this.setExpandable(true);
    child.parent = this;

    if (this.treeOutline)
      this.treeOutline._bindTreeElement(child);
    for (var current = child.firstChild(); this.treeOutline && current;
         current = current.traverseNextTreeElement(false, child, true))
      this.treeOutline._bindTreeElement(current);
    child.onattach();
    child._ensureSelection();
    if (this.treeOutline)
      this.treeOutline.dispatchEventToListeners(UI.TreeOutline.Events.ElementAttached, child);
    var nextSibling = child.nextSibling ? child.nextSibling._listItemNode : null;
    this._childrenListNode.insertBefore(child._listItemNode, nextSibling);
    this._childrenListNode.insertBefore(child._childrenListNode, nextSibling);
    if (child.selected)
      child.select();
    if (child.expanded)
      child.expand();
  }

  /**
   * @param {number} childIndex
   */
  removeChildAtIndex(childIndex) {
    if (childIndex < 0 || childIndex >= this._children.length)
      throw 'childIndex out of range';

    var child = this._children[childIndex];
    this._children.splice(childIndex, 1);

    var parent = child.parent;
    if (this.treeOutline && this.treeOutline.selectedTreeElement &&
        this.treeOutline.selectedTreeElement.hasAncestorOrSelf(child)) {
      if (child.nextSibling)
        child.nextSibling.select(true);
      else if (child.previousSibling)
        child.previousSibling.select(true);
      else if (parent)
        parent.select(true);
    }

    if (child.previousSibling)
      child.previousSibling.nextSibling = child.nextSibling;
    if (child.nextSibling)
      child.nextSibling.previousSibling = child.previousSibling;
    child.parent = null;

    if (this.treeOutline)
      this.treeOutline._unbindTreeElement(child);
    for (var current = child.firstChild(); this.treeOutline && current;
         current = current.traverseNextTreeElement(false, child, true))
      this.treeOutline._unbindTreeElement(current);

    child._detach();
  }

  /**
   * @param {!UI.TreeElement} child
   */
  removeChild(child) {
    if (!child)
      throw 'child can\'t be undefined or null';
    if (child.parent !== this)
      return;

    var childIndex = this._children.indexOf(child);
    if (childIndex === -1)
      throw 'child not found in this node\'s children';

    this.removeChildAtIndex(childIndex);
  }

  removeChildren() {
    if (!this.root && this.treeOutline && this.treeOutline.selectedTreeElement &&
        this.treeOutline.selectedTreeElement.hasAncestorOrSelf(this))
      this.select(true);

    for (var i = 0; this._children && i < this._children.length; ++i) {
      var child = this._children[i];
      child.previousSibling = null;
      child.nextSibling = null;
      child.parent = null;

      if (this.treeOutline)
        this.treeOutline._unbindTreeElement(child);
      for (var current = child.firstChild(); this.treeOutline && current;
           current = current.traverseNextTreeElement(false, child, true))
        this.treeOutline._unbindTreeElement(current);
      child._detach();
    }
    this._children = [];
  }

  get selectable() {
    if (this._hidden)
      return false;
    return this._selectable;
  }

  set selectable(x) {
    this._selectable = x;
  }

  get listItemElement() {
    return this._listItemNode;
  }

  /**
   * @return {!Element}
   */
  titleElement() {
    return this._titleElement;
  }

  get childrenListElement() {
    return this._childrenListNode;
  }

  /**
   * @return {string|!Node}
   */
  get title() {
    return this._title;
  }

  /**
   * @param {string|!Node} x
   */
  set title(x) {
    if (this._title === x)
      return;
    this._title = x;

    if (typeof x === 'string') {
      this._titleElement.textContent = x;
      this.tooltip = x;
    } else {
      this._titleElement = x;
      this.tooltip = '';
    }

    this._listItemNode.removeChildren();
    if (this._leadingIconsElement)
      this._listItemNode.appendChild(this._leadingIconsElement);
    this._listItemNode.appendChild(this._titleElement);
    if (this._trailingIconsElement)
      this._listItemNode.appendChild(this._trailingIconsElement);
    this._ensureSelection();
  }

  /**
   * @return {string}
   */
  titleAsText() {
    if (!this._title)
      return '';
    if (typeof this._title === 'string')
      return this._title;
    return this._title.textContent;
  }

  /**
   * @param {!UI.InplaceEditor.Config} editingConfig
   */
  startEditingTitle(editingConfig) {
    UI.InplaceEditor.startEditing(this._titleElement, editingConfig);
    this.treeOutline._shadowRoot.getSelection().selectAllChildren(this._titleElement);
  }

  /**
   * @param {!Array<!UI.Icon>} icons
   */
  setLeadingIcons(icons) {
    if (!this._leadingIconsElement && !icons.length)
      return;
    if (!this._leadingIconsElement) {
      this._leadingIconsElement = createElementWithClass('div', 'leading-icons');
      this._leadingIconsElement.classList.add('icons-container');
      this._listItemNode.insertBefore(this._leadingIconsElement, this._titleElement);
      this._ensureSelection();
    }
    this._leadingIconsElement.removeChildren();
    for (var icon of icons)
      this._leadingIconsElement.appendChild(icon);
  }

  /**
   * @param {!Array<!UI.Icon>} icons
   */
  setTrailingIcons(icons) {
    if (!this._trailingIconsElement && !icons.length)
      return;
    if (!this._trailingIconsElement) {
      this._trailingIconsElement = createElementWithClass('div', 'trailing-icons');
      this._trailingIconsElement.classList.add('icons-container');
      this._listItemNode.appendChild(this._trailingIconsElement);
      this._ensureSelection();
    }
    this._trailingIconsElement.removeChildren();
    for (var icon of icons)
      this._trailingIconsElement.appendChild(icon);
  }


  /**
   * @return {string}
   */
  get tooltip() {
    return this._tooltip || '';
  }

  /**
   * @param {string} x
   */
  set tooltip(x) {
    if (this._tooltip === x)
      return;
    this._tooltip = x;
    this._listItemNode.title = x;
  }

  /**
   * @return {boolean}
   */
  isExpandable() {
    return this._expandable;
  }

  /**
   * @param {boolean} expandable
   */
  setExpandable(expandable) {
    if (this._expandable === expandable)
      return;

    this._expandable = expandable;

    this._listItemNode.classList.toggle('parent', expandable);
    if (!expandable) {
      this.collapse();
      UI.ARIAUtils.unsetExpanded(this._listItemNode);
    } else {
      UI.ARIAUtils.setExpanded(this._listItemNode, false);
    }
  }

  /**
   * @param {boolean} collapsible
   */
  setCollapsible(collapsible) {
    if (this._collapsible === collapsible)
      return;

    this._collapsible = collapsible;

    this._listItemNode.classList.toggle('always-parent', !collapsible);
    if (!collapsible)
      this.expand();
  }

  get hidden() {
    return this._hidden;
  }

  set hidden(x) {
    if (this._hidden === x)
      return;

    this._hidden = x;

    this._listItemNode.classList.toggle('hidden', x);
    this._childrenListNode.classList.toggle('hidden', x);
  }

  invalidateChildren() {
    if (this._children) {
      this.removeChildren();
      this._children = null;
    }
  }

  /**
   * @return {number}
   */
  computeLeftMargin() {
    var treeElement = this.parent;
    var depth = 0;
    while (treeElement !== null) {
      depth++;
      treeElement = treeElement.parent;
    }

    return -(this.treeOutline._paddingSize * (depth - 1) + 4);
  }

  _ensureSelection() {
    if (!this.treeOutline || !this.treeOutline._renderSelection)
      return;
    if (!this._selectionElement)
      this._selectionElement = createElementWithClass('div', 'selection fill');
    if (this.treeOutline._paddingSize)
      this._selectionElement.style.setProperty('margin-left', this.computeLeftMargin() + 'px');
    this._listItemNode.insertBefore(this._selectionElement, this.listItemElement.firstChild);
  }

  /**
   * @param {!Event} event
   */
  _treeElementToggled(event) {
    var element = event.currentTarget;
    if (element.treeElement !== this || element.hasSelection())
      return;

    var toggleOnClick = this.toggleOnClick && !this.selectable;
    var isInTriangle = this.isEventWithinDisclosureTriangle(event);
    if (!toggleOnClick && !isInTriangle)
      return;

    if (this.expanded) {
      if (event.altKey)
        this.collapseRecursively();
      else
        this.collapse();
    } else {
      if (event.altKey)
        this.expandRecursively();
      else
        this.expand();
    }
    event.consume();
  }

  /**
   * @param {!Event} event
   */
  _handleMouseDown(event) {
    var element = event.currentTarget;
    if (!element)
      return;
    if (!this.selectable)
      return;
    if (element.treeElement !== this)
      return;

    if (this.isEventWithinDisclosureTriangle(event))
      return;

    this.selectOnMouseDown(event);
  }

  /**
   * @param {!Event} event
   */
  _handleDoubleClick(event) {
    var element = event.currentTarget;
    if (!element || element.treeElement !== this)
      return;

    var handled = this.ondblclick(event);
    if (handled)
      return;
    if (this._expandable && !this.expanded)
      this.expand();
  }

  _detach() {
    this._listItemNode.remove();
    this._childrenListNode.remove();
  }

  collapse() {
    if (!this.expanded || !this._collapsible)
      return;
    this._listItemNode.classList.remove('expanded');
    this._childrenListNode.classList.remove('expanded');
    UI.ARIAUtils.setExpanded(this._listItemNode, false);
    this.expanded = false;
    this.oncollapse();
    if (this.treeOutline)
      this.treeOutline.dispatchEventToListeners(UI.TreeOutline.Events.ElementCollapsed, this);
  }

  collapseRecursively() {
    var item = this;
    while (item) {
      if (item.expanded)
        item.collapse();
      item = item.traverseNextTreeElement(false, this, true);
    }
  }

  expand() {
    if (!this._expandable || (this.expanded && this._children))
      return;

    // Set this before onpopulate. Since onpopulate can add elements, this makes
    // sure the expanded flag is true before calling those functions. This prevents the possibility
    // of an infinite loop if onpopulate were to call expand.

    this.expanded = true;

    this._populateIfNeeded();
    this._listItemNode.classList.add('expanded');
    this._childrenListNode.classList.add('expanded');
    UI.ARIAUtils.setExpanded(this._listItemNode, true);

    if (this.treeOutline) {
      this.onexpand();
      this.treeOutline.dispatchEventToListeners(UI.TreeOutline.Events.ElementExpanded, this);
    }
  }

  /**
   * @param {number=} maxDepth
   */
  expandRecursively(maxDepth) {
    var item = this;
    var info = {};
    var depth = 0;

    // The Inspector uses TreeOutlines to represents object properties, so recursive expansion
    // in some case can be infinite, since JavaScript objects can hold circular references.
    // So default to a recursion cap of 3 levels, since that gives fairly good results.
    if (isNaN(maxDepth))
      maxDepth = 3;

    while (item) {
      if (depth < maxDepth)
        item.expand();
      item = item.traverseNextTreeElement(false, this, (depth >= maxDepth), info);
      depth += info.depthChange;
    }
  }

  /**
   * @param {boolean} altKey
   * @return {boolean}
   */
  collapseOrAscend(altKey) {
    if (this.expanded) {
      if (altKey)
        this.collapseRecursively();
      else
        this.collapse();
      return true;
    }

    if (!this.parent || this.parent.root)
      return false;

    if (!this.parent.selectable) {
      this.parent.collapse();
      return true;
    }

    var nextSelectedElement = this.parent;
    while (nextSelectedElement && !nextSelectedElement.selectable)
      nextSelectedElement = nextSelectedElement.parent;

    if (!nextSelectedElement)
      return false;
    nextSelectedElement.select(false, true);
    return true;
  }

  /**
   * @param {boolean} altKey
   * @return {boolean}
   */
  descendOrExpand(altKey) {
    if (!this._expandable)
      return false;

    if (!this.expanded) {
      if (altKey)
        this.expandRecursively();
      else
        this.expand();
      return true;
    }

    var nextSelectedElement = this.firstChild();
    while (nextSelectedElement && !nextSelectedElement.selectable)
      nextSelectedElement = nextSelectedElement.nextSibling;

    if (!nextSelectedElement)
      return false;
    nextSelectedElement.select(false, true);
    return true;
  }

  /**
   * @param {boolean=} center
   */
  reveal(center) {
    var currentAncestor = this.parent;
    while (currentAncestor && !currentAncestor.root) {
      if (!currentAncestor.expanded)
        currentAncestor.expand();
      currentAncestor = currentAncestor.parent;
    }

    this.treeOutline._deferredScrollIntoView(this, !!center);
  }

  /**
   * @return {boolean}
   */
  revealed() {
    var currentAncestor = this.parent;
    while (currentAncestor && !currentAncestor.root) {
      if (!currentAncestor.expanded)
        return false;
      currentAncestor = currentAncestor.parent;
    }

    return true;
  }

  selectOnMouseDown(event) {
    if (this.select(false, true))
      event.consume(true);
  }

  /**
   * @param {boolean=} omitFocus
   * @param {boolean=} selectedByUser
   * @return {boolean}
   */
  select(omitFocus, selectedByUser) {
    if (!this.treeOutline || !this.selectable || this.selected)
      return false;
    // Wait to deselect this element so that focus only changes once
    var lastSelected = this.treeOutline.selectedTreeElement;
    this.treeOutline.selectedTreeElement = null;

    if (this.treeOutline._rootElement === this) {
      if (lastSelected)
        lastSelected.deselect();
      return false;
    }

    this.selected = true;

    this.treeOutline.selectedTreeElement = this;
    if (this.treeOutline._focusable)
      this._setFocusable(true);
    if (!omitFocus || this.treeOutline.contentElement.hasFocus())
      this.listItemElement.focus();

    this._listItemNode.classList.add('selected');
    this.treeOutline.dispatchEventToListeners(UI.TreeOutline.Events.ElementSelected, this);
    if (lastSelected)
      lastSelected.deselect();
    return this.onselect(selectedByUser);
  }

  /**
   * @param {boolean} focusable
   */
  _setFocusable(focusable) {
    if (focusable) {
      this._listItemNode.setAttribute('tabIndex', 0);
      this._listItemNode.addEventListener('focus', this._boundOnFocus, false);
      this._listItemNode.addEventListener('blur', this._boundOnBlur, false);
    } else {
      this._listItemNode.removeAttribute('tabIndex');
      this._listItemNode.removeEventListener('focus', this._boundOnFocus, false);
      this._listItemNode.removeEventListener('blur', this._boundOnBlur, false);
    }
  }

  _onFocus() {
    this._listItemNode.classList.add('force-white-icons');
  }

  _onBlur() {
    this._listItemNode.classList.remove('force-white-icons');
  }

  /**
   * @param {boolean=} omitFocus
   */
  revealAndSelect(omitFocus) {
    this.reveal(true);
    this.select(omitFocus);
  }

  deselect() {
    var hadFocus = this._listItemNode.hasFocus();
    this.selected = false;
    this._listItemNode.classList.remove('selected');
    this._setFocusable(false);

    if (this.treeOutline && this.treeOutline.selectedTreeElement === this) {
      this.treeOutline.selectedTreeElement = null;
      if (hadFocus)
        this.treeOutline.focus();
    }
  }

  _populateIfNeeded() {
    if (this.treeOutline && this._expandable && !this._children) {
      this._children = [];
      this.onpopulate();
    }
  }

  onpopulate() {
    // Overridden by subclasses.
  }

  /**
   * @return {boolean}
   */
  onenter() {
    return false;
  }

  /**
   * @return {boolean}
   */
  ondelete() {
    return false;
  }

  /**
   * @return {boolean}
   */
  onspace() {
    return false;
  }

  onbind() {
  }

  onunbind() {
  }

  onattach() {
  }

  onexpand() {
  }

  oncollapse() {
  }

  /**
   * @param {!Event} e
   * @return {boolean}
   */
  ondblclick(e) {
    return false;
  }

  /**
   * @param {boolean=} selectedByUser
   * @return {boolean}
   */
  onselect(selectedByUser) {
    return false;
  }

  /**
   * @param {boolean} skipUnrevealed
   * @param {?UI.TreeElement=} stayWithin
   * @param {boolean=} dontPopulate
   * @param {!Object=} info
   * @return {?UI.TreeElement}
   */
  traverseNextTreeElement(skipUnrevealed, stayWithin, dontPopulate, info) {
    if (!dontPopulate)
      this._populateIfNeeded();

    if (info)
      info.depthChange = 0;

    var element = skipUnrevealed ? (this.revealed() ? this.firstChild() : null) : this.firstChild();
    if (element && (!skipUnrevealed || (skipUnrevealed && this.expanded))) {
      if (info)
        info.depthChange = 1;
      return element;
    }

    if (this === stayWithin)
      return null;

    element = skipUnrevealed ? (this.revealed() ? this.nextSibling : null) : this.nextSibling;
    if (element)
      return element;

    element = this;
    while (element && !element.root &&
           !(skipUnrevealed ? (element.revealed() ? element.nextSibling : null) : element.nextSibling) &&
           element.parent !== stayWithin) {
      if (info)
        info.depthChange -= 1;
      element = element.parent;
    }

    if (!element || element.root)
      return null;

    return (skipUnrevealed ? (element.revealed() ? element.nextSibling : null) : element.nextSibling);
  }

  /**
   * @param {boolean} skipUnrevealed
   * @param {boolean=} dontPopulate
   * @return {?UI.TreeElement}
   */
  traversePreviousTreeElement(skipUnrevealed, dontPopulate) {
    var element = skipUnrevealed ? (this.revealed() ? this.previousSibling : null) : this.previousSibling;
    if (!dontPopulate && element)
      element._populateIfNeeded();

    while (element && (skipUnrevealed ? (element.revealed() && element.expanded ? element.lastChild() : null) :
                                        element.lastChild())) {
      if (!dontPopulate)
        element._populateIfNeeded();
      element =
          (skipUnrevealed ? (element.revealed() && element.expanded ? element.lastChild() : null) :
                            element.lastChild());
    }

    if (element)
      return element;

    if (!this.parent || this.parent.root)
      return null;

    return this.parent;
  }

  /**
   * @return {boolean}
   */
  isEventWithinDisclosureTriangle(event) {
    // FIXME: We should not use getComputedStyle(). For that we need to get rid of using ::before for disclosure triangle. (http://webk.it/74446)
    var paddingLeftValue = window.getComputedStyle(this._listItemNode).paddingLeft;
    console.assert(paddingLeftValue.endsWith('px'));
    var computedLeftPadding = parseFloat(paddingLeftValue);
    var left = this._listItemNode.totalOffsetLeft() + computedLeftPadding;
    return event.pageX >= left && event.pageX <= left + UI.TreeElement._ArrowToggleWidth && this._expandable;
  }
};

/** @const */
UI.TreeElement._ArrowToggleWidth = 10;

(function() {
  var img = new Image();
  if (window.devicePixelRatio > 1)
    img.src = 'Images/treeoutlineTriangles_2x.png';
  else
    img.src = 'Images/treeoutlineTriangles.png';
  UI.TreeElement._imagePreload = img;
})();
