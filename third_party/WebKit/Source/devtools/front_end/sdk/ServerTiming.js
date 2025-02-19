// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
/**
 * @unrestricted
 */
SDK.ServerTiming = class {
  /**
   * @param {string} metric
   * @param {?number} value
   * @param {?string} description
   */
  constructor(metric, value, description) {
    this.metric = metric;
    this.value = value;
    this.description = description;
  }

  /**
   * @param {!Array<!SDK.NetworkRequest.NameValue>} headers
   * @return {?Array<!SDK.ServerTiming>}
   */
  static parseHeaders(headers) {
    var rawServerTimingHeaders = headers.filter(item => item.name.toLowerCase() === 'server-timing');
    if (!rawServerTimingHeaders.length)
      return null;

    /**
     * @param {?string} valueString
     * @return {?Array<!SDK.ServerTiming>}
     */
    function createFromHeaderValue(valueString) {
      // https://www.w3.org/TR/server-timing/
      var serverTimingMetricRegExp =
          /[ \t]*([\!\#\$\%\&\'\*\+\-\.\^\_\`\|\~0-9A-Za-z]+)[ \t]*(?:=[ \t]*(\d+(?:\.\d+)?))?[ \t]*(?:;[ \t]*(?:"([^"]+)"|([\!\#\$\%\&\'\*\+\-\.\^\_\`\|\~0-9A-Za-z]+)))?[ \t]*(?:,(.*))?/;
      var metricMatch;
      var result = [];
      while (valueString && (metricMatch = serverTimingMetricRegExp.exec(valueString))) {
        var metric = metricMatch[1];
        var value = metricMatch[2];
        var description = metricMatch[3] || metricMatch[4];
        if (value !== undefined)
          value = Math.abs(parseFloat(metricMatch[2]));
        valueString = metricMatch[5];  // comma delimited headers
        if (value === undefined || isNaN(value))
          value = null;
        result.push(new SDK.ServerTiming(metric, value, description));
      }
      return result;
    }

    var serverTimings = rawServerTimingHeaders.reduce((memo, header) => {
      var timing = createFromHeaderValue(header.value);
      Array.prototype.push.apply(memo, timing);
      return memo;
    }, []);
    serverTimings.sort((a, b) => a.metric.toLowerCase().compareTo(b.metric.toLowerCase()));
    return serverTimings;
  }
};
