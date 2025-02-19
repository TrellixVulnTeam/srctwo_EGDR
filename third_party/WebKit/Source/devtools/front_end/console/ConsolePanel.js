/*
 * Copyright (C) 2009 Joseph Pecoraro
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
Console.ConsolePanel = class extends UI.Panel {
  constructor() {
    super('console');
    this._view = Console.ConsoleView.instance();
  }

  /**
   * @return {!Console.ConsolePanel}
   */
  static instance() {
    return /** @type {!Console.ConsolePanel} */ (self.runtime.sharedInstance(Console.ConsolePanel));
  }

  /**
   * @override
   */
  wasShown() {
    super.wasShown();
    var wrapper = Console.ConsolePanel.WrapperView._instance;
    if (wrapper && wrapper.isShowing())
      UI.inspectorView.setDrawerMinimized(true);
    this._view.show(this.element);
  }

  /**
   * @override
   */
  willHide() {
    super.willHide();
    if (Console.ConsolePanel.WrapperView._instance)
      Console.ConsolePanel.WrapperView._instance._showViewInWrapper();
    UI.inspectorView.setDrawerMinimized(false);
  }

  /**
   * @override
   * @return {?UI.SearchableView}
   */
  searchableView() {
    return Console.ConsoleView.instance().searchableView();
  }
};

/**
 * @unrestricted
 */
Console.ConsolePanel.WrapperView = class extends UI.VBox {
  constructor() {
    super();
    this.element.classList.add('console-view-wrapper');

    Console.ConsolePanel.WrapperView._instance = this;

    this._view = Console.ConsoleView.instance();
  }

  /**
   * @override
   */
  wasShown() {
    if (!Console.ConsolePanel.instance().isShowing())
      this._showViewInWrapper();
    else
      UI.inspectorView.setDrawerMinimized(true);
  }

  /**
   * @override
   */
  willHide() {
    UI.inspectorView.setDrawerMinimized(false);
  }

  _showViewInWrapper() {
    this._view.show(this.element);
  }
};

/**
 * @implements {Common.Revealer}
 * @unrestricted
 */
Console.ConsolePanel.ConsoleRevealer = class {
  /**
   * @override
   * @param {!Object} object
   * @return {!Promise}
   */
  reveal(object) {
    var consoleView = Console.ConsoleView.instance();
    if (consoleView.isShowing()) {
      consoleView.focus();
      return Promise.resolve();
    }
    UI.viewManager.showView('console-view');
    return Promise.resolve();
  }
};
