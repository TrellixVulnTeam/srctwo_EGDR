// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var USER_MEDIA_TAB_ID = 'user-media-tab-id';

var tabView = null;
var ssrcInfoManager = null;
var peerConnectionUpdateTable = null;
var statsTable = null;
var dumpCreator = null;
/** A map from peer connection id to the PeerConnectionRecord. */
var peerConnectionDataStore = {};
/** A list of getUserMedia requests. */
var userMediaRequests = [];

/** A simple class to store the updates and stats data for a peer connection. */
var PeerConnectionRecord = (function() {
  /** @constructor */
  function PeerConnectionRecord() {
    /** @private */
    this.record_ = {
      constraints: {},
      rtcConfiguration: [],
      stats: {},
      updateLog: [],
      url: '',
    };
  };

  PeerConnectionRecord.prototype = {
    /** @override */
    toJSON: function() {
      return this.record_;
    },

    /**
     * Adds the initilization info of the peer connection.
     * @param {string} url The URL of the web page owning the peer connection.
     * @param {Array} rtcConfiguration
     * @param {!Object} constraints Media constraints.
     */
    initialize: function(url, rtcConfiguration, constraints) {
      this.record_.url = url;
      this.record_.rtcConfiguration = rtcConfiguration;
      this.record_.constraints = constraints;
    },

    /**
     * @param {string} dataSeriesId The TimelineDataSeries identifier.
     * @return {!TimelineDataSeries}
     */
    getDataSeries: function(dataSeriesId) {
      return this.record_.stats[dataSeriesId];
    },

    /**
     * @param {string} dataSeriesId The TimelineDataSeries identifier.
     * @param {!TimelineDataSeries} dataSeries The TimelineDataSeries to set to.
     */
    setDataSeries: function(dataSeriesId, dataSeries) {
      this.record_.stats[dataSeriesId] = dataSeries;
    },

    /**
     * @param {!Object} update The object contains keys "time", "type", and
     *   "value".
     */
    addUpdate: function(update) {
      var time = new Date(parseFloat(update.time));
      this.record_.updateLog.push({
        time: time.toLocaleString(),
        type: update.type,
        value: update.value,
      });
    },
  };

  return PeerConnectionRecord;
})();

// The maximum number of data points bufferred for each stats. Old data points
// will be shifted out when the buffer is full.
var MAX_STATS_DATA_POINT_BUFFER_SIZE = 1000;

// <include src="tab_view.js">
// <include src="data_series.js">
// <include src="ssrc_info_manager.js">
// <include src="stats_graph_helper.js">
// <include src="stats_table.js">
// <include src="peer_connection_update_table.js">
// <include src="dump_creator.js">


function initialize() {
  dumpCreator = new DumpCreator($('content-root'));
  tabView = new TabView($('content-root'));
  ssrcInfoManager = new SsrcInfoManager();
  peerConnectionUpdateTable = new PeerConnectionUpdateTable();
  statsTable = new StatsTable(ssrcInfoManager);

  chrome.send('finishedDOMLoad');

  // Requests stats from all peer connections every second.
  window.setInterval(requestStats, 1000);
}
document.addEventListener('DOMContentLoaded', initialize);


/** Sends a request to the browser to get peer connection statistics. */
function requestStats() {
  if (Object.keys(peerConnectionDataStore).length > 0)
    chrome.send('getAllStats');
}


/**
 * A helper function for getting a peer connection element id.
 *
 * @param {!Object<number>} data The object containing the pid and lid of the
 *     peer connection.
 * @return {string} The peer connection element id.
 */
function getPeerConnectionId(data) {
  return data.pid + '-' + data.lid;
}


/**
 * Extracts ssrc info from a setLocal/setRemoteDescription update.
 *
 * @param {!PeerConnectionUpdateEntry} data The peer connection update data.
 */
function extractSsrcInfo(data) {
  if (data.type == 'setLocalDescription' ||
      data.type == 'setRemoteDescription') {
    ssrcInfoManager.addSsrcStreamInfo(data.value);
  }
}


/**
 * A helper function for appending a child element to |parent|.
 *
 * @param {!Element} parent The parent element.
 * @param {string} tag The child element tag.
 * @param {string} text The textContent of the new DIV.
 * @return {!Element} the new DIV element.
 */
function appendChildWithText(parent, tag, text) {
  var child = document.createElement(tag);
  child.textContent = text;
  parent.appendChild(child);
  return child;
}

/**
 * Helper for adding a peer connection update.
 *
 * @param {Element} peerConnectionElement
 * @param {!PeerConnectionUpdateEntry} update The peer connection update data.
 */
function addPeerConnectionUpdate(peerConnectionElement, update) {
  peerConnectionUpdateTable.addPeerConnectionUpdate(peerConnectionElement,
                                                    update);
  extractSsrcInfo(update);
  peerConnectionDataStore[peerConnectionElement.id].addUpdate(update);
}


/** Browser message handlers. */


/**
 * Removes all information about a peer connection.
 *
 * @param {!Object<number>} data The object containing the pid and lid of a peer
 *     connection.
 */
function removePeerConnection(data) {
  var element = $(getPeerConnectionId(data));
  if (element) {
    delete peerConnectionDataStore[element.id];
    tabView.removeTab(element.id);
  }
}


/**
 * Adds a peer connection.
 *
 * @param {!Object} data The object containing the pid, lid, url,
 *     rtcConfiguration, and constraints of a peer connection.
 */
function addPeerConnection(data) {
  var id = getPeerConnectionId(data);

  if (!peerConnectionDataStore[id]) {
    peerConnectionDataStore[id] = new PeerConnectionRecord();
  }
  peerConnectionDataStore[id].initialize(
      data.url, data.rtcConfiguration, data.constraints);

  var peerConnectionElement = $(id);
  if (!peerConnectionElement) {
    peerConnectionElement = tabView.addTab(id, data.url + ' [' + id + ']');
  }

  var p = document.createElement('p');
  p.textContent = data.url + ', ' + data.rtcConfiguration + ', ' +
      data.constraints;
  peerConnectionElement.appendChild(p);

  return peerConnectionElement;
}


/**
 * Adds a peer connection update.
 *
 * @param {!PeerConnectionUpdateEntry} data The peer connection update data.
 */
function updatePeerConnection(data) {
  var peerConnectionElement = $(getPeerConnectionId(data));
  addPeerConnectionUpdate(peerConnectionElement, data);
}


/**
 * Adds the information of all peer connections created so far.
 *
 * @param {Array<!Object>} data An array of the information of all peer
 *     connections. Each array item contains pid, lid, url, rtcConfiguration,
 *     constraints, and an array of updates as the log.
 */
function updateAllPeerConnections(data) {
  for (var i = 0; i < data.length; ++i) {
    var peerConnection = addPeerConnection(data[i]);

    var log = data[i].log;
    if (!log)
      continue;
    for (var j = 0; j < log.length; ++j) {
      addPeerConnectionUpdate(peerConnection, log[j]);
    }
  }
  requestStats();
}


/**
 * Handles the report of stats.
 *
 * @param {!Object} data The object containing pid, lid, and reports, where
 *     reports is an array of stats reports. Each report contains id, type,
 *     and stats, where stats is the object containing timestamp and values,
 *     which is an array of strings, whose even index entry is the name of the
 *     stat, and the odd index entry is the value.
 */
function addStats(data) {
  var peerConnectionElement = $(getPeerConnectionId(data));
  if (!peerConnectionElement)
    return;

  for (var i = 0; i < data.reports.length; ++i) {
    var report = data.reports[i];
    statsTable.addStatsReport(peerConnectionElement, report);
    drawSingleReport(peerConnectionElement, report);
  }
}


/**
 * Adds a getUserMedia request.
 *
 * @param {!Object} data The object containing rid {number}, pid {number},
 *     origin {string}, audio {string}, video {string}.
 */
function addGetUserMedia(data) {
  userMediaRequests.push(data);

  if (!$(USER_MEDIA_TAB_ID)) {
    tabView.addTab(USER_MEDIA_TAB_ID, 'GetUserMedia Requests');
  }

  var requestDiv = document.createElement('div');
  requestDiv.className = 'user-media-request-div-class';
  requestDiv.rid = data.rid;
  $(USER_MEDIA_TAB_ID).appendChild(requestDiv);

  appendChildWithText(requestDiv, 'div', 'Caller origin: ' + data.origin);
  appendChildWithText(requestDiv, 'div', 'Caller process id: ' + data.pid);
  appendChildWithText(requestDiv, 'span', 'Audio Constraints').style.fontWeight
      = 'bold';
  appendChildWithText(requestDiv, 'div', data.audio);

  appendChildWithText(requestDiv, 'span', 'Video Constraints').style.fontWeight
      = 'bold';
  appendChildWithText(requestDiv, 'div', data.video);
}


/**
 * Removes the getUserMedia requests from the specified |rid|.
 *
 * @param {!Object} data The object containing rid {number}, the render id.
 */
function removeGetUserMediaForRenderer(data) {
  for (var i = userMediaRequests.length - 1; i >= 0; --i) {
    if (userMediaRequests[i].rid == data.rid)
      userMediaRequests.splice(i, 1);
  }

  var requests = $(USER_MEDIA_TAB_ID).childNodes;
  for (var i = 0; i < requests.length; ++i) {
    if (requests[i].rid == data.rid)
      $(USER_MEDIA_TAB_ID).removeChild(requests[i]);

  }
  if ($(USER_MEDIA_TAB_ID).childNodes.length == 0)
    tabView.removeTab(USER_MEDIA_TAB_ID);
}


/**
 * Notification that the audio debug recordings file selection dialog was
 * cancelled, i.e. recordings have not been enabled.
 */
function audioDebugRecordingsFileSelectionCancelled() {
  dumpCreator.clearAudioDebugRecordingsCheckbox();
}


/**
 * Notification that the event log recordings file selection dialog was
 * cancelled, i.e. recordings have not been enabled.
 */
function eventLogRecordingsFileSelectionCancelled() {
  dumpCreator.clearEventLogRecordingsCheckbox();
}


/**
 * Notification that audio debug recordings are enabled. Used e.g. on page load
 * to update the UI to reflect the recording state.
 */
function setAudioDebugRecordingsEnabled() {
  dumpCreator.setAudioDebugRecordingsCheckbox();
}


/**
 * Notification that event log recordings are enabled. Used e.g. on page load
 * to update the UI to reflect the recording state.
 */
function setEventLogRecordingsEnabled() {
  dumpCreator.setEventLogRecordingsCheckbox();
}
