#!/bin/bash

# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -e

for dir in */ ; do
  cd "$dir"

  if [ -f generate-chains.py ]; then
    python generate-chains.py

    # Cleanup temporary files.
    rm -rf */*.pyc
    rm -rf out/
  fi

  cd ..
done
