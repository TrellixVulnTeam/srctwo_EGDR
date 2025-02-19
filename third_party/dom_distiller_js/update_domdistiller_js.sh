#!/bin/bash
#
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#

# Clones the dom-distiller repo, compiles and extracts its javascript Then
# copies that js into the Chromium tree.
# This script requires that ant is installed. It takes an optional parameter
# for which SHA1 to roll to. If left unspecified the script rolls to HEAD.

(
  set -e

  dom_distiller_js_path=$(dirname "${BASH_SOURCE[0]}")
  src_path=$dom_distiller_js_path/../..
  readme_chromium=$dom_distiller_js_path/README.chromium
  [ ! -f $readme_chromium ] && echo "$readme_chromium is not found" && exit 1
  tmpdir=/tmp/domdistiller-$$
  changes=$tmpdir/domdistiller.changes
  bugs=$tmpdir/domdistiller.bugs
  curr_gitsha=$(grep 'Version:' $readme_chromium | awk '{print $2}')

  rm -rf $tmpdir
  mkdir $tmpdir
  pushd $tmpdir

  git clone https://github.com/chromium/dom-distiller.git
  pushd dom-distiller

  # The new git SHA1 is HEAD or the first command line parameter.
  [[ -z "$1" ]] && gitsha_target="HEAD" || gitsha_target="$1"
  new_gitsha=$(git rev-parse --short=10 ${gitsha_target})
  git reset --hard ${new_gitsha}
  git log --oneline ${curr_gitsha}..${new_gitsha} > $changes

  echo -n BUG= > $bugs

  # This extracts BUG= lines from the log, extracts the numbers part, removes
  # whitespace and deletes empty lines. Then, split on ',', sort, uniquify and
  # rejoin. Finally, remove the trailing ',' and concat to $bugs.
  git log ${curr_gitsha}..${new_gitsha} \
    | grep BUG= \
    | sed -e 's/.*BUG=\(.*\)/\1/' -e 's/\s*//g' -e '/^$/d' \
    | tr ',' '\n' \
    | sort \
    | uniq \
    | tr '\n' ',' \
    | head --bytes=-1 \
    >> $bugs

  echo >> $bugs  # add a newline

  ant package
  popd # dom-distiller

  git clone https://github.com/chromium/dom-distiller-dist.git $tmpdir/dom-distiller-dist
  rm -rf $tmpdir/dom-distiller-dist/*
  pushd dom-distiller-dist
  cp -r $tmpdir/dom-distiller/out/package/* .
  git add .
  if [[ $(git status --short | wc -l) -ne 0 ]]; then
    git commit -a -m "Package for ${new_gitsha}"
    git push origin master
  else
    # No changes to external repo, but need to check if DEPS refers to same SHA1.
    echo "WARNING: There were no changes to the distribution package."
  fi
  new_dist_gitsha=$(git rev-parse HEAD)
  popd # dom-distiller-dist

  popd # tmpdir
  curr_dist_gitsha=$(grep -e "/external\/github.com\/chromium\/dom-distiller-dist.git" $src_path/DEPS | sed -e "s/.*'\([A-Za-z0-9]\{40\}\)'.*/\1/g")
  if [[ "${new_dist_gitsha}" == "${curr_dist_gitsha}" ]]; then
    echo "The roll does not include any changes to the dist package. Exiting."
    rm -rf $tmpdir
    exit 1
  fi

  cp $tmpdir/dom-distiller/LICENSE $dom_distiller_js_path/
  sed -i "s/Version: [0-9a-f]*/Version: ${new_gitsha}/" $readme_chromium
  sed -i -e "s/\('\/external\/github.com\/chromium\/dom-distiller-dist.git' + '@' + '\)\([0-9a-f]\+\)'/\1${new_dist_gitsha}'/" $src_path/DEPS

  gen_message () {
    echo "Roll DOM Distiller JavaScript distribution package"
    echo
    echo "Diff since last roll:"
    echo "https://github.com/chromium/dom-distiller/compare/${curr_gitsha}...${new_gitsha}"
    echo
    echo "Picked up changes:"
    cat $changes
    echo
    cat $bugs
  }

  message=$tmpdir/message
  gen_message > $message

  # Run checklicenses.py on the pulled files, but only print the output on
  # failures.
  $src_path/tools/checklicenses/checklicenses.py third_party/dom_distiller_js > $tmpdir/checklicenses.out || cat $tmpdir/checklicenses.out

  git commit -a -F $message

  rm -rf $tmpdir
)
