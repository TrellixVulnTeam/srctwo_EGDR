#!/bin/bash
#
# Helper to do build so you don't have to remember all the steps/args.


set -eu

# Some base locations.
readonly ScriptDir=$(dirname "$(echo $0 | sed -e "s,^\([^/]\),$(pwd)/\1,")")
readonly ProtoRootDir="${ScriptDir}/../.."

printUsage() {
  NAME=$(basename "${0}")
  cat << EOF
usage: ${NAME} [OPTIONS]

This script does the common build steps needed.

OPTIONS:

 General:

   -h, --help
         Show this message
   -c, --clean
         Issue a clean before the normal build.
   -a, --autogen
         Start by rerunning autogen & configure.
   -r, --regenerate-descriptors
         Run generate_descriptor_proto.sh to regenerate all the checked in
         proto sources.
   -j #, --jobs #
         Force the number of parallel jobs (useful for debugging build issues).
   --core-only
         Skip some of the core protobuf build/checks to shorten the build time.
   --skip-xcode
         Skip the invoke of Xcode to test the runtime on both iOS and OS X.
   --skip-xcode-ios
         Skip the invoke of Xcode to test the runtime on iOS.
   --skip-xcode-debug
         Skip the Xcode Debug configuration.
   --skip-xcode-release
         Skip the Xcode Release configuration.
   --skip-xcode-osx
         Skip the invoke of Xcode to test the runtime on OS X.
   --skip-objc-conformance
         Skip the Objective C conformance tests (run on OS X).
   --xcode-quiet
         Pass -quiet to xcodebuild.

EOF
}

header() {
  echo ""
  echo "========================================================================"
  echo "    ${@}"
  echo "========================================================================"
}

# Thanks to libtool, builds can fail in odd ways and since it eats some output
# it can be hard to spot, so force error output if make exits with a non zero.
wrapped_make() {
  set +e  # Don't stop if the command fails.
  make $*
  MAKE_EXIT_STATUS=$?
  if [ ${MAKE_EXIT_STATUS} -ne 0 ]; then
    echo "Error: 'make $*' exited with status ${MAKE_EXIT_STATUS}"
    exit ${MAKE_EXIT_STATUS}
  fi
  set -e
}

NUM_MAKE_JOBS=$(/usr/sbin/sysctl -n hw.ncpu)
if [[ "${NUM_MAKE_JOBS}" -lt 2 ]] ; then
  NUM_MAKE_JOBS=2
fi

DO_AUTOGEN=no
DO_CLEAN=no
REGEN_DESCRIPTORS=no
CORE_ONLY=no
DO_XCODE_IOS_TESTS=yes
DO_XCODE_OSX_TESTS=yes
DO_XCODE_DEBUG=yes
DO_XCODE_RELEASE=yes
DO_OBJC_CONFORMANCE_TESTS=yes
XCODE_QUIET=no
while [[ $# != 0 ]]; do
  case "${1}" in
    -h | --help )
      printUsage
      exit 0
      ;;
    -c | --clean )
      DO_CLEAN=yes
      ;;
    -a | --autogen )
      DO_AUTOGEN=yes
      ;;
    -r | --regenerate-descriptors )
      REGEN_DESCRIPTORS=yes
      ;;
    -j | --jobs )
      shift
      NUM_MAKE_JOBS="${1}"
      ;;
    --core-only )
      CORE_ONLY=yes
      ;;
    --skip-xcode )
      DO_XCODE_IOS_TESTS=no
      DO_XCODE_OSX_TESTS=no
      ;;
    --skip-xcode-ios )
      DO_XCODE_IOS_TESTS=no
      ;;
    --skip-xcode-osx )
      DO_XCODE_OSX_TESTS=no
      ;;
    --skip-xcode-debug )
      DO_XCODE_DEBUG=no
      ;;
    --skip-xcode-release )
      DO_XCODE_RELEASE=no
      ;;
    --skip-objc-conformance )
      DO_OBJC_CONFORMANCE_TESTS=no
      ;;
    --xcode-quiet )
      XCODE_QUIET=yes
      ;;
    -*)
      echo "ERROR: Unknown option: ${1}" 1>&2
      printUsage
      exit 1
      ;;
    *)
      echo "ERROR: Unknown argument: ${1}" 1>&2
      printUsage
      exit 1
      ;;
  esac
  shift
done

# Into the proto dir.
cd "${ProtoRootDir}"

# if no Makefile, force the autogen.
if [[ ! -f Makefile ]] ; then
  DO_AUTOGEN=yes
fi

if [[ "${DO_AUTOGEN}" == "yes" ]] ; then
  header "Running autogen & configure"
  ./autogen.sh
  ./configure \
    CPPFLAGS="-mmacosx-version-min=10.9 -Wunused-const-variable -Wunused-function" \
    CXXFLAGS="-Wnon-virtual-dtor -Woverloaded-virtual"
fi

if [[ "${DO_CLEAN}" == "yes" ]] ; then
  header "Cleaning"
  wrapped_make clean
  if [[ "${DO_XCODE_IOS_TESTS}" == "yes" ]] ; then
    XCODEBUILD_CLEAN_BASE_IOS=(
      xcodebuild
        -project objectivec/ProtocolBuffers_iOS.xcodeproj
        -scheme ProtocolBuffers
    )
    if [[ "${DO_XCODE_DEBUG}" == "yes" ]] ; then
      "${XCODEBUILD_CLEAN_BASE_IOS[@]}" -configuration Debug clean
    fi
    if [[ "${DO_XCODE_RELEASE}" == "yes" ]] ; then
      "${XCODEBUILD_CLEAN_BASE_IOS[@]}" -configuration Release clean
    fi
  fi
  if [[ "${DO_XCODE_OSX_TESTS}" == "yes" ]] ; then
    XCODEBUILD_CLEAN_BASE_OSX=(
      xcodebuild
        -project objectivec/ProtocolBuffers_OSX.xcodeproj
        -scheme ProtocolBuffers
    )
    if [[ "${DO_XCODE_DEBUG}" == "yes" ]] ; then
      "${XCODEBUILD_CLEAN_BASE_OSX[@]}" -configuration Debug clean
    fi
    if [[ "${DO_XCODE_RELEASE}" == "yes" ]] ; then
      "${XCODEBUILD_CLEAN_BASE_OSX[@]}" -configuration Release clean
    fi
  fi
fi

if [[ "${REGEN_DESCRIPTORS}" == "yes" ]] ; then
  header "Regenerating the descriptor sources."
  ./generate_descriptor_proto.sh -j "${NUM_MAKE_JOBS}"
fi

if [[ "${CORE_ONLY}" == "yes" ]] ; then
  header "Building core Only"
  wrapped_make -j "${NUM_MAKE_JOBS}"
else
  header "Building"
  # Can't issue these together, when fully parallel, something sometimes chokes
  # at random.
  wrapped_make -j "${NUM_MAKE_JOBS}" all
  wrapped_make -j "${NUM_MAKE_JOBS}" check
  # Fire off the conformance tests also.
  cd conformance
  wrapped_make -j "${NUM_MAKE_JOBS}" test_cpp
  cd ..
fi

# Ensure the WKT sources checked in are current.
objectivec/generate_well_known_types.sh --check-only -j "${NUM_MAKE_JOBS}"

header "Checking on the ObjC Runtime Code"
objectivec/DevTools/pddm_tests.py
if ! objectivec/DevTools/pddm.py --dry-run objectivec/*.[hm] objectivec/Tests/*.[hm] ; then
  echo ""
  echo "Update by running:"
  echo "   objectivec/DevTools/pddm.py objectivec/*.[hm] objectivec/Tests/*.[hm]"
  exit 1
fi

readonly XCODE_VERSION_LINE="$(xcodebuild -version | grep Xcode\  )"
readonly XCODE_VERSION="${XCODE_VERSION_LINE/Xcode /}"  # drop the prefix.

if [[ "${DO_XCODE_IOS_TESTS}" == "yes" ]] ; then
  XCODEBUILD_TEST_BASE_IOS=(
    xcodebuild
      -project objectivec/ProtocolBuffers_iOS.xcodeproj
      -scheme ProtocolBuffers
  )
  if [[ "${XCODE_QUIET}" == "yes" ]] ; then
    XCODEBUILD_TEST_BASE_IOS+=( -quiet )
  fi
  # Don't need to worry about form factors or retina/non retina;
  # just pick a mix of OS Versions and 32/64 bit.
  # NOTE: Different Xcode have different simulated hardware/os support.
  case "${XCODE_VERSION}" in
    6.* )
      echo "ERROR: Xcode 6.3/6.4 no longer supported for building, please use 8.0 or higher." 1>&2
      exit 10
      ;;
    7.* )
      echo "ERROR: The unittests include Swift code that is now Swift 3.0." 1>&2
      echo "ERROR: Xcode 8.0 or higher is required to build the test suite, but the library works with Xcode 7.x." 1>&2
      exit 11
      ;;
    8.0* )
      # The 8.* device seem to hang and never start under Xcode 8.
      XCODEBUILD_TEST_BASE_IOS+=(
          -destination "platform=iOS Simulator,name=iPhone 4s,OS=9.0" # 32bit
          -destination "platform=iOS Simulator,name=iPhone 7,OS=10.0" # 64bit
          -destination "platform=iOS Simulator,name=iPad 2,OS=9.0" # 32bit
          -destination "platform=iOS Simulator,name=iPad Pro (9.7 inch),OS=10.0" # 64bit
      )
      ;;
    8.1* )
      XCODEBUILD_TEST_BASE_IOS+=(
          -destination "platform=iOS Simulator,name=iPhone 4s,OS=8.1" # 32bit
          -destination "platform=iOS Simulator,name=iPhone 7,OS=10.1" # 64bit
          -destination "platform=iOS Simulator,name=iPad 2,OS=8.1" # 32bit
          -destination "platform=iOS Simulator,name=iPad Pro (9.7 inch),OS=10.1" # 64bit
      )
      ;;
    8.2* )
      XCODEBUILD_TEST_BASE_IOS+=(
          -destination "platform=iOS Simulator,name=iPhone 4s,OS=8.1" # 32bit
          -destination "platform=iOS Simulator,name=iPhone 7,OS=10.2" # 64bit
          -destination "platform=iOS Simulator,name=iPad 2,OS=8.1" # 32bit
          -destination "platform=iOS Simulator,name=iPad Pro (9.7 inch),OS=10.2" # 64bit
      )
      ;;
    8.3* )
      XCODEBUILD_TEST_BASE_IOS+=(
          -destination "platform=iOS Simulator,name=iPhone 4s,OS=8.1" # 32bit
          -destination "platform=iOS Simulator,name=iPhone 7,OS=10.3" # 64bit
          -destination "platform=iOS Simulator,name=iPad 2,OS=8.1" # 32bit
          -destination "platform=iOS Simulator,name=iPad Pro (9.7 inch),OS=10.3" # 64bit
      )
      ;;
    * )
      echo "Time to update the simulator targets for Xcode ${XCODE_VERSION}"
      exit 2
      ;;
  esac
  if [[ "${DO_XCODE_DEBUG}" == "yes" ]] ; then
    header "Doing Xcode iOS build/tests - Debug"
    "${XCODEBUILD_TEST_BASE_IOS[@]}" -configuration Debug test
  fi
  if [[ "${DO_XCODE_RELEASE}" == "yes" ]] ; then
    header "Doing Xcode iOS build/tests - Release"
    "${XCODEBUILD_TEST_BASE_IOS[@]}" -configuration Release test
  fi
  # Don't leave the simulator in the developer's face.
  killall Simulator
fi
if [[ "${DO_XCODE_OSX_TESTS}" == "yes" ]] ; then
  XCODEBUILD_TEST_BASE_OSX=(
    xcodebuild
      -project objectivec/ProtocolBuffers_OSX.xcodeproj
      -scheme ProtocolBuffers
      # Since the ObjC 2.0 Runtime is required, 32bit OS X isn't supported.
      -destination "platform=OS X,arch=x86_64" # 64bit
  )
  if [[ "${XCODE_QUIET}" == "yes" ]] ; then
    XCODEBUILD_TEST_BASE_OSX+=( -quiet )
  fi
  case "${XCODE_VERSION}" in
    6.* )
      echo "ERROR: Xcode 6.3/6.4 no longer supported for building, please use 8.0 or higher." 1>&2
      exit 10
      ;;
    7.* )
      echo "ERROR: The unittests include Swift code that is now Swift 3.0." 1>&2
      echo "ERROR: Xcode 8.0 or higher is required to build the test suite, but the library works with Xcode 7.x." 1>&2
      exit 11
      ;;
  esac
  if [[ "${DO_XCODE_DEBUG}" == "yes" ]] ; then
    header "Doing Xcode OS X build/tests - Debug"
    "${XCODEBUILD_TEST_BASE_OSX[@]}" -configuration Debug test
  fi
  if [[ "${DO_XCODE_RELEASE}" == "yes" ]] ; then
    header "Doing Xcode OS X build/tests - Release"
    "${XCODEBUILD_TEST_BASE_OSX[@]}" -configuration Release test
  fi
fi

if [[ "${DO_OBJC_CONFORMANCE_TESTS}" == "yes" ]] ; then
  header "Running ObjC Conformance Tests"
  cd conformance
  wrapped_make -j "${NUM_MAKE_JOBS}" test_objc
  cd ..
fi
