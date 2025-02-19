#!/bin/sh
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Script to build binutils for both i386 and AMD64 Linux architectures.
# Must be run on an AMD64 supporting machine which has debootstrap and sudo
# installed.
# Uses Ubuntu Lucid chroots as build environment.

set -e

if [ x"$(whoami)" = x"root" ]; then
  echo "Script must not be run as root."
  exit 1
fi
sudo -v

OUTPUTDIR="${1:-$PWD/output-$(date +%Y%m%d-%H%M%S)}"
if [ ! -d "$OUTPUTDIR" ]; then
  mkdir -p "$OUTPUTDIR"
fi

# Download the source
VERSION=2.26
wget -c http://ftp.gnu.org/gnu/binutils/binutils-$VERSION.tar.bz2

# Verify the signature
wget -c -q http://ftp.gnu.org/gnu/binutils/binutils-$VERSION.tar.bz2.sig
if ! gpg --verify binutils-$VERSION.tar.bz2.sig; then
  echo "GPG Signature failed to verify."
  echo ""
  echo "You may need to import the vendor GPG key with:"
  echo "# gpg --keyserver pgp.mit.edu --recv-key 4AE55E93"
  exit 1
fi

# Extract the source
rm -rf binutils-$VERSION
tar jxf binutils-$VERSION.tar.bz2

# Patch the source
(
  cd binutils-$VERSION
  echo "long-plt.patch"
  echo "=================================="
  patch -p1 < ../long-plt.patch
  echo "----------------------------------"
  echo
  echo "icf-rel.patch"
  echo "=================================="
  patch -p1 < ../icf-rel.patch
  echo "----------------------------------"
  echo
  echo "icf-align.patch"
  echo "=================================="
  patch -p1 < ../icf-align.patch
  echo "----------------------------------"
  echo
)

for ARCH in i386 amd64; do
  if [ ! -d precise-chroot-$ARCH ]; then
    # Refresh sudo credentials
    sudo -v

    # Create the chroot
    echo ""
    echo "Building chroot for $ARCH"
    echo "============================="
    sudo debootstrap \
        --arch=$ARCH \
        --include=build-essential,flex,bison \
        precise precise-chroot-$ARCH
    echo "============================="
  fi

  BUILDDIR=precise-chroot-$ARCH/build

  # Clean up any previous failed build attempts inside chroot
  if [ -d "$BUILDDIR" ]; then
    sudo rm -rf "$BUILDDIR"
  fi

  # Copy data into the chroot
  sudo mkdir -p "$BUILDDIR"
  sudo cp -a binutils-$VERSION "$BUILDDIR"
  sudo cp -a build-one.sh "$BUILDDIR"

  # Do the build
  PREFIX=
  case $ARCH in
   i386)
     PREFIX="setarch linux32"
     ARCHNAME=i686-pc-linux-gnu
   ;;
   amd64)
     PREFIX="setarch linux64"
     ARCHNAME=x86_64-pc-linux-gnu
   ;;
  esac
  echo ""
  echo "Building binutils for $ARCH"
  LOGFILE="$OUTPUTDIR/build-$ARCH.log"
  if ! sudo $PREFIX chroot precise-chroot-$ARCH /build/build-one.sh /build/binutils-$VERSION > $LOGFILE 2>&1; then
    echo "Build failed! See $LOGFILE for details."
    exit 1
  fi

  # Copy data out of the chroot
  sudo chown -R $(whoami) "$BUILDDIR/output/"

  # Strip the output binaries
  strip "$BUILDDIR/output/$ARCHNAME/bin/"*

  # Copy them out of the chroot
  cp -a "$BUILDDIR/output/$ARCHNAME" "$OUTPUTDIR"

  # Clean up chroot
  sudo rm -rf "$BUILDDIR"
done

echo "Check you are happy with the binaries in"
echo "  $OUTPUTDIR"
echo "Then"
echo " * upload to Google Storage using the upload.sh script"
echo " * roll dependencies"
