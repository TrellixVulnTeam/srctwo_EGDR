// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
/**
 * @implements {Workspace.ProjectSearchConfig}
 * @unrestricted
 */
Workspace.SearchConfig = class {
  /**
   * @param {string} query
   * @param {boolean} ignoreCase
   * @param {boolean} isRegex
   */
  constructor(query, ignoreCase, isRegex) {
    this._query = query;
    this._ignoreCase = ignoreCase;
    this._isRegex = isRegex;
    this._parse();
  }

  /**
   * @param {{query: string, ignoreCase: boolean, isRegex: boolean}} object
   * @return {!Workspace.SearchConfig}
   */
  static fromPlainObject(object) {
    return new Workspace.SearchConfig(object.query, object.ignoreCase, object.isRegex);
  }

  /**
   * @override
   * @return {string}
   */
  query() {
    return this._query;
  }

  /**
   * @override
   * @return {boolean}
   */
  ignoreCase() {
    return this._ignoreCase;
  }

  /**
   * @override
   * @return {boolean}
   */
  isRegex() {
    return this._isRegex;
  }

  /**
   * @return {{query: string, ignoreCase: boolean, isRegex: boolean}}
   */
  toPlainObject() {
    return {query: this.query(), ignoreCase: this.ignoreCase(), isRegex: this.isRegex()};
  }

  _parse() {
    // Inside double quotes: any symbol except double quote and backslash or any symbol escaped with a backslash.
    var quotedPattern = /"([^\\"]|\\.)+"/;
    // A word is a sequence of any symbols except space and backslash or any symbols escaped with a backslash, that does not start with file:.
    var unquotedWordPattern = /(\s*(?!-?f(ile)?:)[^\\ ]|\\.)+/;
    var unquotedPattern = unquotedWordPattern.source + '(\\s+' + unquotedWordPattern.source + ')*';


    var pattern = [
      '(\\s*' + Workspace.SearchConfig.FilePatternRegex.source + '\\s*)',
      '(' + quotedPattern.source + ')',
      '(' + unquotedPattern + ')',
    ].join('|');
    var regexp = new RegExp(pattern, 'g');
    var queryParts = this._query.match(regexp) || [];
    /**
     * @type {!Array.<!Workspace.SearchConfig.QueryTerm>}
     */
    this._fileQueries = [];

    /**
     * @type {!Array.<string>}
     */
    this._queries = [];

    for (var i = 0; i < queryParts.length; ++i) {
      var queryPart = queryParts[i];
      if (!queryPart)
        continue;
      var fileQuery = this._parseFileQuery(queryPart);
      if (fileQuery) {
        this._fileQueries.push(fileQuery);
        /** @type {!Array.<!Workspace.SearchConfig.RegexQuery>} */
        this._fileRegexQueries = this._fileRegexQueries || [];
        this._fileRegexQueries.push(
            {regex: new RegExp(fileQuery.text, this.ignoreCase ? 'i' : ''), isNegative: fileQuery.isNegative});
        continue;
      }
      if (this._isRegex) {
        this._queries.push(queryPart);
        continue;
      }
      if (queryPart.startsWith('"')) {
        if (!queryPart.endsWith('"'))
          continue;
        this._queries.push(this._parseQuotedQuery(queryPart));
        continue;
      }
      this._queries.push(this._parseUnquotedQuery(queryPart));
    }
  }

  /**
   * @override
   * @param {string} filePath
   * @return {boolean}
   */
  filePathMatchesFileQuery(filePath) {
    if (!this._fileRegexQueries)
      return true;
    for (var i = 0; i < this._fileRegexQueries.length; ++i) {
      if (!!filePath.match(this._fileRegexQueries[i].regex) === this._fileRegexQueries[i].isNegative)
        return false;
    }
    return true;
  }

  /**
   * @override
   * @return {!Array.<string>}
   */
  queries() {
    return this._queries;
  }

  _parseUnquotedQuery(query) {
    return query.replace(/\\(.)/g, '$1');
  }

  _parseQuotedQuery(query) {
    return query.substring(1, query.length - 1).replace(/\\(.)/g, '$1');
  }

  /**
   * @param {string} query
   * @return {?Workspace.SearchConfig.QueryTerm}
   */
  _parseFileQuery(query) {
    var match = query.match(Workspace.SearchConfig.FilePatternRegex);
    if (!match)
      return null;
    var isNegative = !!match[1];
    query = match[3];
    var result = '';
    for (var i = 0; i < query.length; ++i) {
      var char = query[i];
      if (char === '*') {
        result += '.*';
      } else if (char === '\\') {
        ++i;
        var nextChar = query[i];
        if (nextChar === ' ')
          result += ' ';
      } else {
        if (String.regexSpecialCharacters().indexOf(query.charAt(i)) !== -1)
          result += '\\';
        result += query.charAt(i);
      }
    }
    return new Workspace.SearchConfig.QueryTerm(result, isNegative);
  }
};

// After file: prefix: any symbol except space and backslash or any symbol escaped with a backslash.
Workspace.SearchConfig.FilePatternRegex = /(-)?f(ile)?:((?:[^\\ ]|\\.)+)/;

/** @typedef {!{regex: !RegExp, isNegative: boolean}} */
Workspace.SearchConfig.RegexQuery;


/**
 * @unrestricted
 */
Workspace.SearchConfig.QueryTerm = class {
  /**
   * @param {string} text
   * @param {boolean} isNegative
   */
  constructor(text, isNegative) {
    this.text = text;
    this.isNegative = isNegative;
  }
};
