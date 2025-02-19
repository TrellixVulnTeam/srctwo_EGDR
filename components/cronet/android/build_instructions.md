# Checking out and building Cronet for Android

[TOC]

## Checking out the code

Follow all the
[Checking out and building Chromium for Android
](../../../docs/android_build_instructions.md) instructions up to and including
the
[Run the hooks](../../../docs/android_build_instructions.md#Run-the-hooks) step.

## Building Cronet

### Building Cronet for development and debugging

To build Cronet for development and debugging purposes:

```shell
$ ./components/cronet/tools/cr_cronet.py gn
$ ninja -C out/Debug cronet_package
```

Note: these commands clobber output of previously executed gn commands in
out/Debug.

### Building Cronet for releases

To build Cronet with optimizations and with debug information stripped out:

```shell
$ ./components/cronet/tools/cr_cronet.py -r gn 
$ ninja -C out/Release cronet_package
```

Note: these commands clobber output of previously executed gn commands in
out/Release.

### Building for other architectures

By default ARMv7 32-bit executables are generated. To generate executables
targeting other architectures modify [cr_cronet.py](../tools/cr_cronet.py)'s
`gn_args` variable to include:

*   For ARMv6 32-bit: `arm_version=6`
*   For ARMv8 64-bit: `target_cpu="arm64"`
*   For x86 32-bit: `target_cpu="x86"`
*   For x86 64-bit: `target_cpu="x64"`
*   For MIPS: `target_cpu="mipsel"`

## Running tests

### Running tests locally

First, connect an Android device by following the [Plug in your Android
device](../../../docs/android_build_instructions.md#Plug-in-your-Android-device)
steps. Prefer using a device running a userdebug build.

#### Running Cronet Java unit tests

To run Java unit tests that actuate the Cronet API:

```shell
$ ./components/cronet/tools/cr_cronet.py gn
$ ./components/cronet/tools/cr_cronet.py build-test
```

To run particular tests specify the test class and method name to the build-test
command. For example:

```shell
$ ./components/cronet/tools/cr_cronet.py build-test -f QuicTest#testQuicLoadUrl
```

#### Running net_unittests and cronet_unittests

To run C++ and Java unit tests of net/ functionality:

```shell
$ ./components/cronet/tools/cr_cronet.py gn
$ ninja -C out/Debug net_unittests
$ ./out/Debug/bin/run_net_unittests --fast-local-dev
```

For more information about running net_unittests, read
[Android Test Instructions](../../../docs/android_test_instructions.md).

There are a small number of C++ Cronet unit tests, called cronet_unittests, that
can be run by following the above instructions and substituting cronet_unittests
for net_unittests.

#### Running Cronet performance tests

To run Cronet's perf tests, follow the instructions in
[components/cronet/android/test/javaperftests/run.py](test/javaperftests/run.py)

### Running tests remotely

Once you've uploaded a Chromium change list using `git cl upload`, you can
launch a bot to build and test your change list:

```shell
$ git cl try -b android_cronet_tester
```

This will run both the Cronet Java unit tests and net_unittests.

## Debugging

### Debug Log

Messages from native (C++) code appear in the Android system log accessible with
`adb logcat`. By default you will see only messages designated as FATAL. To
enable more verbosity:

#### See VLOG(1) and VLOG(2) logging:

```shell
$ adb shell setprop log.tag.CronetUrlRequestContext VERBOSE
```

#### See VLOG(1) logging: 

```shell
$ adb shell setprop log.tag.CronetUrlRequestContext DEBUG 
```

#### See NO (only FATAL) logging: 

```shell
$ adb shell setprop log.tag.CronetUrlRequestContext NONE
```

### Network Log

NetLog is Chromium's network logging system. To create a NetLog dump, you can
use the following pair of methods:

```
CronetEngine.startNetLogToFile()
CronetEngine.stopNetLog()
```

Unlike the Android system log which is line-based, the Chromium log is formatted
in JSON.  As such, it will probably not be well-formed until you have called the
`stopNetLog()` method, as filesystem buffers will not have been flushed. 

Retrieve the file from your device's file system, and import it to chrome
browser at chrome://net-internals/#import, or
http://catapult-project.github.io/catapult/netlog_viewer which helps to
visualize the data.

### Symbolicating crash stacks

If an app or test using Cronet crashes it can be useful to know the functions
and line numbers involved in the stack trace. This can be done using the
Android system log:

```shell
$ ./components/cronet/tools/cr_cronet.py stack
```

Or using tombstones left behind after crashes:

```shell
$ CHROMIUM_OUTPUT_DIR=out/Debug ./build/android/tombstones.py
```
