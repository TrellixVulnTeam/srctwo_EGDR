// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Include test fixture.
GEN_INCLUDE(['net_internals_test.js']);

// Anonymous namespace
(function() {

// @return {Array<object>} List of events for an abbreviated URL request.
function urlRequestEvents(id) {
  return [
    {
      'phase': EventPhase.PHASE_BEGIN,
      'source': {
        'id': id,
        'type': EventSourceType.URL_REQUEST
      },
      'time': '953534778',
      'type': EventType.REQUEST_ALIVE
    },
    {
      'params': {
        'load_flags': 68223104,
        'method': 'GET',
        'priority': 4,
        'url': 'http://www.google.com/'
      },
      'phase': EventPhase.PHASE_BEGIN,
      'source': {
        'id': id,
        'type': EventSourceType.URL_REQUEST
      },
      'time': '953534792',
      'type': EventType.URL_REQUEST_START_JOB
    },
    {
      'phase': EventPhase.PHASE_END,
      'source': {
        'id': id,
        'type': EventSourceType.URL_REQUEST
      },
      'time': '953534800',
      'type': EventType.URL_REQUEST_START_JOB
    },
    {
      'phase': EventPhase.PHASE_BEGIN,
      'source': {
        'id': id,
        'type': EventSourceType.URL_REQUEST
      },
      'time': '953534906',
      'type': EventType.HTTP_TRANSACTION_SEND_REQUEST
    },
    {
      'params': {
        'headers': [
          'Host: www.google.com',
          'Connection: keep-alive',
          'User-Agent: Mozilla/5.0',
          'Accept: text/html',
          'Accept-Encoding: gzip,deflate,sdch',
          'Accept-Language: en-US,en;q=0.8',
          'Accept-Charset: ISO-8859-1',
          'X-Random-Header-With-Quotes: "Quoted String"""',
        ],
        'line': 'GET / HTTP/1.1\r\n'
      },
      'phase': EventPhase.PHASE_NONE,
      'source': {
        'id': id,
        'type': EventSourceType.URL_REQUEST
      },
      'time': '953534910',
      'type': EventType.HTTP_TRANSACTION_SEND_REQUEST_HEADERS
    },
    {
      'phase': EventPhase.PHASE_END,
      'source': {
        'id': id,
        'type': EventSourceType.URL_REQUEST
      },
      'time': '953534915',
      'type': EventType.HTTP_TRANSACTION_SEND_REQUEST
    },
    {
      'phase': EventPhase.PHASE_END,
      'source': {
        'id': id,
        'type': EventSourceType.URL_REQUEST
      },
      'time': '953535567',
      'type': EventType.REQUEST_ALIVE
    }
  ];
}

/**
 * Tests the filters, both in terms of filtering correctly and UI.
 */
TEST_F('NetInternalsTest', 'netInternalsEventsViewFilter', function() {
  // Sets the filter and checks the results.
  // @param {string} filter Filter to use.
  // @param {Array<boolean>} matches Ordered list of whether or not each source
  //     matches |filter|.  Order must match display order after applying the
  //     filter.
  function checkFilter(filter, matches) {
    EventsView.getInstance().setFilterText_(filter);

    var postFilter = 0;
    for (var i = 0; i < matches.length; ++i) {
      if (matches[i])
        ++postFilter;
    }

    // Updating the display is normally done asynchronously, so have to manually
    // call update function to check displayed event count.
    EventsView.getInstance().repaintFilterCounter_();

    expectEquals(postFilter + ' of ' + matches.length,
                 $(EventsView.FILTER_COUNT_ID).innerText,
                 filter);

    var tbody = $(EventsView.TBODY_ID);
    assertEquals(matches.length, tbody.childElementCount, filter);

    var visibleCount = 0;
    for (var i = 0; i < tbody.childElementCount; ++i) {
      expectEquals(matches[i],
                   NetInternalsTest.nodeIsVisible(tbody.children[i]),
                   filter);
    }
  }

  EventsTracker.getInstance().deleteAllLogEntries();
  checkFilter('', [], 0);

  // A completed request.
  g_browser.receivedLogEntries(urlRequestEvents(31));
  checkFilter('', [true], 1);

  // An incomplete request.
  g_browser.receivedLogEntries(urlRequestEvents(56).slice(0, 3));
  checkFilter('', [true, true], 2);

  // Filters used alone and in all combinations.
  // |text| is the string to add to the filter.
  // |matches| is a 2-element array of booleans indicating which of the two
  //      requests passes the filter.
  var testFilters = [
    {text: 'http://www.google.com', matches: [true, true] },
    {text: 'MyMagicPony', matches: [false, false] },
    {text: 'type:URL_REQUEST', matches: [true, true] },
    {text: 'type:SOCKET,URL_REQUEST', matches: [true, true] },
    {text: 'type:SOCKET', matches: [false, false] },
    {text: '-type:PONY', matches: [true, true] },
    {text: '-type:URL_REQUEST', matches: [false, false] },
    {text: 'id:31,32', matches: [true, false] },
    {text: '-id:31,32', matches: [false, true] },
    {text: 'id:32,56,', matches: [false, true] },
    {text: '-is:active', matches: [true, false] },
    {text: 'is:active', matches: [false, true] },
    {text: '-is:error', matches: [true, true] },
    {text: 'is:error', matches: [false, false] },
    // Partial match of source type.
    {text: 'URL_REQ', matches: [true, true] },
    // Partial match of event type type.
    {text: 'SEND_REQUEST', matches: [true, false] },
    // Check that ":" works in strings.
    { text: 'Host:', matches: [true, false] },
    { text: '::', matches: [false, false] },
    // Test quotes.
    { text: '"Quoted String"', matches: [true, false] },
    { text: '"Quoted source"', matches: [false, false] },
    { text: '"\\"Quoted String\\""', matches: [true, false] },
    { text: '"\\"\\"Quoted String\\""', matches: [false, false] },
    { text: '\\"\\"\\"', matches: [true, false] },
    { text: '\\"\\"\\"\\"', matches: [false, false] },
    { text: '"Host: www.google.com"', matches: [true, false], },
    { text: 'Connection:" keep-alive"', matches: [true, false], },
    { text: '-Connection:" keep-alive"', matches: [false, true], },
    { text: '"Host: GET"', matches: [false, false] },
    // Make sure sorting has no effect on filters.  Sort by ID so order is
    // preserved.
    { text: 'sort:"id"', matches: [true, true] },
    { text: '-sort:"shoe size"', matches: [true, true] },
    { text: '"-sort:shoe size"', matches: [false, false] },
  ];

  for (var filter1 = 0; filter1 < testFilters.length; ++filter1) {
    checkFilter(testFilters[filter1].text, testFilters[filter1].matches);
    // Check |filter1| in combination with all the other filters.
    for (var filter2 = 0; filter2 < testFilters.length; ++filter2) {
      var matches = [];
      for (var i = 0; i < testFilters[filter1].matches.length; ++i) {
        // The merged filter matches an entry if both individual filters do.
        matches[i] = testFilters[filter1].matches[i] &&
                     testFilters[filter2].matches[i];
      }

      checkFilter(testFilters[filter1].text + ' ' + testFilters[filter2].text,
                  matches,
                  1);
    }
  }

  // Tests with unmatched quotes.  Unlike the strings above, combining them with
  // other filters is not the same as applying both filters independently.
  checkFilter('"Quoted String', [true, false]);
  checkFilter('"Quoted String source', [false, false]);
  checkFilter('Quoted" String', [true, false]);
  checkFilter('Quoted" source', [false, false]);
  checkFilter('Quoted "String', [true, false]);

  // Test toggling sort method, without any filters.
  var eventsView = EventsView.getInstance();
  eventsView.setFilterText_('');
  $(EventsView.SORT_BY_DESCRIPTION_ID).click();
  expectEquals('sort:desc', eventsView.getFilterText_());
  $(EventsView.SORT_BY_DESCRIPTION_ID).click();
  expectEquals('-sort:desc', eventsView.getFilterText_());
  $(EventsView.SORT_BY_ID_ID).click();
  expectEquals('sort:id', eventsView.getFilterText_());

  // Sort by default is by ID, so toggling ID when there's no filter results in
  // sort:-id.
  eventsView.setFilterText_('');
  $(EventsView.SORT_BY_ID_ID).click();
  expectEquals('-sort:id', eventsView.getFilterText_());
  $(EventsView.SORT_BY_ID_ID).click();
  expectEquals('sort:id', eventsView.getFilterText_());

  // Test toggling sort method with filters.
  eventsView.setFilterText_('text');
  $(EventsView.SORT_BY_ID_ID).click();
  expectEquals('-sort:id text', eventsView.getFilterText_());
  $(EventsView.SORT_BY_ID_ID).click();
  expectEquals('sort:id text', eventsView.getFilterText_());
  $(EventsView.SORT_BY_SOURCE_TYPE_ID).click();
  expectEquals('sort:source text', eventsView.getFilterText_());
  eventsView.setFilterText_('text sort:id "more text"');
  $(EventsView.SORT_BY_ID_ID).click();
  expectEquals('-sort:id text "more text"', eventsView.getFilterText_());

  testDone();
});

})();  // Anonymous namespace
