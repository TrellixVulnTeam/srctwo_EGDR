// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


/**
 * @fileoverview Tests the text module of Voice Search on the local NTP.
 */


/**
 * Voice Search Text module's object for test and setup functions.
 */
test.text = {};


/**
 * Utility to test code that uses timeouts.
 * @type {MockClock}
 */
test.text.clock = new MockClock();


/**
 * Utility to mock out object properties.
 * @type {Replacer}
 */
test.text.stubs = new Replacer();


/**
 * Set up the text DOM and test environment.
 */
test.text.setUp = function() {
  test.text.clock.reset();
  test.text.stubs.reset();

  setUpPage('voice-text-template');

  test.text.clock.install();
  test.text.stubs.replace(window, 'getChromeUILanguage', () => 'en-ZA');
  test.text.stubs.replace(speech, 'messages', {
    audioError: 'Audio error',
    details: 'Details',
    languageError: 'Language error',
    learnMore: 'Learn more',
    listening: 'Listening',
    networkError: 'Network error',
    noTranslation: 'No translation',
    noVoice: 'No voice',
    permissionError: 'Permission error',
    ready: 'Ready',
    tryAgain: 'Try again',
    waiting: 'Waiting'
  });

  text.init();
};


/**
 * Makes sure text sets up with the correct settings.
 */
test.text.testInit = function() {
  assertEquals('', text.interim_.textContent);
  assertEquals('', text.final_.textContent);
};


/**
 * Test updating the text values.
 */
test.text.testUpdateText = function() {
  const interimText = 'interim';
  const finalText = 'final';
  text.updateTextArea(interimText, finalText);
  assertEquals(interimText, text.interim_.textContent);
  assertEquals(finalText, text.final_.textContent);
};


/**
 * Test updating the text with an error message containing a link.
 */
test.text.testShowErrorMessageWithLink = function() {
  const noAudioError = RecognitionError.AUDIO_CAPTURE;
  text.showErrorMessage(noAudioError);
  const supportLink = text.SUPPORT_LINK_BASE_.replace(/&/, '&amp;') + 'en-ZA';
  assertEquals(
      'Audio error <a class="voice-text-link" id="voice-support-link" ' +
          `href="${supportLink}" target="_blank">Learn more</a>`,
      text.interim_.innerHTML);
  assertEquals('', text.final_.innerHTML);
};


/**
 * Test updating the text with an error message containing a callback.
 */
test.text.testShowErrorMessageWithCallback = function() {
  // Mock the restart callback.
  let restartCalled = false;
  test.text.stubs.replace(speech, 'restart', function() {
    restartCalled = true;
  });

  // Display the try again error.
  const tryAgainError = RecognitionError.NO_MATCH;
  text.showErrorMessage(tryAgainError);
  assertEquals(
      'No translation <a class="voice-text-link" id="voice-retry-link">' +
          'Try again</a>',
      text.interim_.innerHTML);
  assertEquals('', text.final_.innerHTML);

  // Assert the callback is called when the link element is clicked.
  assert(!restartCalled);
  assertEquals(1, text.interim_.children.length);
  text.interim_.children[0].click();
  assert(restartCalled);
};


/**
 * Test clearing the text elements.
 */
test.text.testClearText = function() {
  const interimText = 'interim '.repeat(100);
  const finalText = 'final '.repeat(100);

  assertEquals('voice-text', text.interim_.className);
  assertEquals('voice-text', text.final_.className);
  text.updateTextArea(interimText, finalText);
  assertEquals(interimText, text.interim_.textContent);
  assertEquals(finalText, text.final_.textContent);
  assertEquals('voice-text voice-text-3l', text.interim_.className);
  assertEquals('voice-text voice-text-3l', text.final_.className);

  text.clear();
  assertEquals('', text.interim_.textContent);
  assertEquals('', text.final_.textContent);
  assertEquals('voice-text', text.interim_.className);
  assertEquals('voice-text', text.final_.className);
};


/**
 * Test showing the initialization message after an initial timeout.
 */
test.text.testSetInitializationMessage = function() {
  text.interim_.textContent = 'interim text';
  text.final_.textContent = 'final text';

  test.text.clock.setTime(1);
  text.showInitializingMessage();
  assertEquals('', text.interim_.textContent);
  assertEquals('', text.final_.textContent);
  assertEquals(1, test.text.clock.pendingTimeouts.length);
  assertEquals(301, test.text.clock.pendingTimeouts[0].activationTime);

  test.text.clock.advanceTime(300);
  test.text.clock.pendingTimeouts.shift().callback();

  assertEquals('Waiting', text.interim_.textContent);
  assertEquals('', text.final_.textContent);
  assertEquals(0, test.text.clock.pendingTimeouts.length);
};


/**
 * Test showing the ready message.
 */
test.text.testReadyMessage = function() {
  text.interim_.textContent = 'interim text';
  text.final_.textContent = 'final text';

  test.text.clock.setTime(1);
  text.showReadyMessage();

  assertEquals('Ready', text.interim_.textContent);
  assertEquals('', text.final_.textContent);

  // Assert that the "Listening..." message will be shown after some time.
  assertEquals(1, test.text.clock.pendingTimeouts.length);
  assertEquals(2001, test.text.clock.pendingTimeouts[0].activationTime);
};


/**
 * Test showing the listening message when the ready message is shown.
 */
test.text.testListeningMessageWhenReady = function() {
  text.interim_.textContent = 'Ready';

  test.text.clock.setTime(1);
  text.startListeningMessageAnimation_();

  assertEquals(1, test.text.clock.pendingTimeouts.length);
  assertEquals(2001, test.text.clock.pendingTimeouts[0].activationTime);

  test.text.clock.advanceTime(2000);
  test.text.clock.pendingTimeouts.shift().callback();

  assertEquals('Listening', text.interim_.textContent);
  assertEquals('', text.final_.textContent);
  assertEquals(0, test.text.clock.pendingTimeouts.length);
};


/**
 * Test not showing the listening message when the ready message is not shown.
 */
test.text.testListeningMessageWhenNotReady = function() {
  text.interim_.textContent = 'some text';

  test.text.clock.setTime(1);
  text.startListeningMessageAnimation_();

  assertEquals(1, test.text.clock.pendingTimeouts.length);
  assertEquals(2001, test.text.clock.pendingTimeouts[0].activationTime);

  test.text.clock.advanceTime(2000);
  test.text.clock.pendingTimeouts.shift().callback();

  assertEquals('some text', text.interim_.textContent);
  assertEquals('', text.final_.textContent);
  assertEquals(0, test.text.clock.pendingTimeouts.length);
};

/**
 * Test not showing the listening message when the ready message is spoken.
 */
test.text.testListeningMessageWhenNotReady = function() {
  // Show the "Ready" message.
  text.interim_.textContent = 'Ready';
  assertEquals('', text.final_.textContent);

  // Set the "Listening..." timeout.
  test.text.clock.setTime(1);
  text.startListeningMessageAnimation_();
  assertEquals(1, test.text.clock.pendingTimeouts.length);
  assertEquals(2001, test.text.clock.pendingTimeouts[0].activationTime);

  // Simulate the user speaking the exact same "Ready" message.
  test.text.clock.advanceTime(1000);
  text.updateTextArea('Ready');
  assertEquals('Ready', text.interim_.textContent);
  assertEquals('', text.final_.textContent);

  // The "Listening..." timeout gets cleared and the message will not show up.
  assertEquals(0, test.text.clock.pendingTimeouts.length);
};
