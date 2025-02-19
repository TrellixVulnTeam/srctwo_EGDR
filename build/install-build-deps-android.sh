#!/bin/bash

# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Script to install everything needed to build chromium on android, including
# items requiring sudo privileges.
# See https://www.chromium.org/developers/how-tos/android-build-instructions

args="$@"

if ! uname -m | egrep -q "i686|x86_64"; then
  echo "Only x86 architectures are currently supported" >&2
  exit
fi

# Exit if any commands fail.
set -e

lsb_release=$(lsb_release --codename --short)

# Install first the default Linux build deps.
"$(dirname "${BASH_SOURCE[0]}")/install-build-deps.sh" \
  --no-syms --lib32 --no-arm --no-chromeos-fonts --no-nacl --no-prompt "${args}"

# Fix deps
sudo apt-get -f install

# common
sudo apt-get -y install lib32z1 lighttpd python-pexpect xvfb x11-utils

# Some binaries in the Android SDK require 32-bit libraries on the host.
# See https://developer.android.com/sdk/installing/index.html?pkg=tools
sudo apt-get -y install libncurses5:i386 libstdc++6:i386 zlib1g:i386

# Required by //components/cronet/tools/generate_javadoc.py
# TODO(375324): Stop requiring ANT.
sudo apt-get -y install ant

# Required for apk-patch-size-estimator
sudo apt-get -y install bsdiff

# Do our own error handling for java.
set +e

function IsJava8() {
  # Arg is either "java" or "javac"
  $1 -version 2>&1 | grep -q '1\.8'
}

if ! (IsJava8 java && IsJava8 javac); then
  sudo apt-get -y install openjdk-8-jre openjdk-8-jdk
fi

# There can be several reasons why java8 is not default despite being installed.
# Just show an error and exit.
if ! (IsJava8 java && IsJava8 javac); then
  echo
  echo "Automatic java installation failed."
  echo '`java -version` reports:'
  java -version
  echo
  echo '`javac -version` reports:'
  javac -version
  echo
  echo "Please ensure that JDK 8 is installed and resolves first in your PATH."
  echo -n '`which java` reports: '
  which java
  echo -n '`which javac` reports: '
  which javac
  echo
  echo "You might also try running:"
  echo "    sudo update-java-alternatives -s java-1.8.0-openjdk-amd64"
  exit 1
fi

echo "install-build-deps-android.sh complete."
