// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var should_be_injected_once = document.createElement('div');
should_be_injected_once.className = 'injected-once';
document.body.appendChild(should_be_injected_once);
