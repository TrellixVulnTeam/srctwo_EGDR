# Clang Tool Refactoring

[TOC]

## Introduction

Clang tools can help with global refactorings of Chromium code. Clang tools can
take advantage of clang's AST to perform refactorings that would be impossible
with a traditional find-and-replace regexp:

*   Constructing `scoped_ptr<T>` from `NULL`: <https://crbug.com/173286>
*   Implicit conversions of `scoped_refptr<T>` to `T*`: <https://crbug.com/110610>
*   Rename everything in Blink to follow Chromium style: <https://crbug.com/563793>

## Caveats

An invocation of the clang tool runs on one build config. Code that only
compiles on one platform or code that is guarded by a set of compile-time flags
can be problematic. Performing a global refactoring typically requires running
the tool once in each build config with code that needs to be updated.

Other minor issues:

*   Requires a git checkout.

## Prerequisites

A Chromium checkout created with `fetch` should have everything needed.

For convenience, add `third_party/llvm-build/Release+Asserts/bin` to `$PATH`.

## Writing the tool

LLVM uses C++11 and CMake. Source code for Chromium clang tools lives in
[//tools/clang](https://chromium.googlesource.com/chromium/src/tools/clang/+/master).
It is generally easiest to use one of the already-written tools as the base for
writing a new tool.

Chromium clang tools generally follow this pattern:

1.  Instantiate a [`clang::ast_matchers::MatchFinder`](http://clang.llvm.org/doxygen/classclang_1_1ast__matchers_1_1MatchFinder.html).
2.  Call `addMatcher()` to register [`clang::ast_matchers::MatchFinder::MatchCallback`](http://clang.llvm.org/doxygen/classclang_1_1ast__matchers_1_1MatchFinder_1_1MatchCallback.html)
    actions to execute when [matching](http://clang.llvm.org/docs/LibASTMatchersReference.html)
    the AST.
3.  Create a new `clang::tooling::FrontendActionFactory` from the `MatchFinder`.
4.  Run the action across the specified files with
    [`clang::tooling::ClangTool::run`](http://clang.llvm.org/doxygen/classclang_1_1tooling_1_1ClangTool.html#acec91f63b45ac7ee2d6c94cb9c10dab3).
5.  Serialize generated [`clang::tooling::Replacement`](http://clang.llvm.org/doxygen/classclang_1_1tooling_1_1Replacement.html)s
    to `stdout`.

Other useful references when writing the tool:

*   [Clang doxygen reference](http://clang.llvm.org/doxygen/index.html)
*   [Tutorial for building tools using LibTooling and LibASTMatchers](http://clang.llvm.org/docs/LibASTMatchersTutorial.html)

### Edit serialization format
```
==== BEGIN EDITS ====
r:::path/to/file1:::offset1:::length1:::replacement text
r:::path/to/file2:::offset2:::length2:::replacement text

       ...

==== END EDITS ====
```

The header and footer are required. Each line between the header and footer
represents one edit. Fields are separated by `:::`, and the first field must
be `r` (for replacement). In the future, this may be extended to handle header
insertion/removal. A deletion is an edit with no replacement text.

The edits are applied by [`apply_edits.py`](#Running), which understands certain
conventions:

*   The clang tool should munge newlines in replacement text to `\0`. The script
    knows to translate `\0` back to newlines when applying edits.
*   When removing an element from a 'list' (e.g. function parameters,
    initializers), the clang tool should emit a deletion for just the element.
    The script understands how to extend the deletion to remove commas, etc. as
    needed.

TODO: Document more about `SourceLocation` and how spelling loc differs from
expansion loc, etc.

### Why not RefactoringTool?
While clang has a [`clang::tooling::RefactoringTool`](http://clang.llvm.org/doxygen/classclang_1_1tooling_1_1RefactoringTool.html)
to automatically apply the generated replacements and save the results, it
doesn't work well for Chromium:

*   Clang tools run actions serially, so run time scales poorly to tens of
    thousands of files.
*   A parsing error in any file (quite common in NaCl source) prevents any of
    the generated replacements from being applied.

## Building
Synopsis:

```shell
tools/clang/scripts/update.py --bootstrap --force-local-build --without-android \
  --extra-tools rewrite_to_chrome_style
```

Running this command builds the [Oilpan plugin](https://chromium.googlesource.com/chromium/src/+/master/tools/clang/blink_gc_plugin/),
the [Chrome style
plugin](https://chromium.googlesource.com/chromium/src/+/master/tools/clang/plugins/),
and the [Blink to Chrome style rewriter](https://chromium.googlesource.com/chromium/src/+/master/tools/clang/rewrite_to_chrome_style/). Additional arguments to `--extra-tools` should be the name of
subdirectories in
[//tools/clang](https://chromium.googlesource.com/chromium/src/+/master/tools/clang).

It is important to use --bootstrap as there appear to be [bugs](https://crbug.com/580745)
in the clang library this script produces if you build it with gcc, which is the default.

Once clang is bootsrapped, incremental builds can be done by invoking `ninja` in
the `third_party/llvm-build/Release+Asserts` directory. In particular,
recompiling solely the tool you are writing can be accomplished by executing
`ninja rewrite_to_chrome_style` (replace `rewrite_to_chrome_style` with your
tool's name).

## Running
First, build all Chromium targets to avoid failures due to missing dependencies
that are generated as part of the build:

```shell
ninja -C out/Debug  # For non-Windows
ninja -d keeprsp -C out/Debug  # For Windows

# experimental alternative:
$gen_targets = $(ninja -C out/gn -t targets all \
                 | grep '^gen/[^: ]*\.[ch][pc]*:' \
                 | cut -f 1 -d :`)
ninja -C out/Debug $gen_targets
```

Then run the actual clang tool to generate a list of edits:

```shell
tools/clang/scripts/run_tool.py --tool <path to tool> \
  --generate-compdb
  -p out/Debug <path 1> <path 2> ... >/tmp/list-of-edits.debug
```

`--generate-compdb` can be omitted if the compile DB was already generated and
the list of build flags and source files has not changed since generation.

`<path 1>`, `<path 2>`, etc are optional arguments to filter the files to run
the tool against. This is helpful when sharding global refactorings into smaller
chunks. For example, the following command will run the `empty_string` tool
against just the `.c`, `.cc`, `.cpp`, `.m`, `.mm` files in `//net`.  Note that
the filtering is not applied to the *output* of the tool - the tool can emit
edits that apply to files outside of `//cc` (i.e. edits that apply to headers
from `//base` that got included by source files in `//cc`).

```shell
tools/clang/scripts/run_tool.py --tool empty_string  \
  --generated-compdb \
  -p out/Debug net >/tmp/list-of-edits.debug
```

Note that some header files might only be included from generated files (e.g.
from only from some `.cpp` files under out/Debug/gen).  To make sure that
contents of such header files are processed by the clang tool, the clang tool
needs to be run against the generated files.  The only way to accomplish this
today is to pass `--all` switch to `run_tool.py` - this will run the clang tool
against all the sources from the compilation database.

Finally, apply the edits as follows:

```shell
cat /tmp/list-of-edits.debug \
  | tools/clang/scripts/extract_edits.py \
  | tools/clang/scripts/apply_edits.py -p out/Debug <path 1> <path 2> ...
```

The apply_edits.py tool will only apply edits to files actually under control of
`git`.  `<path 1>`, `<path 2>`, etc are optional arguments to further filter the
files that the edits are applied to.  Note that semantics of these filters is
distinctly different from the arguments of `run_tool.py` filters - one set of
filters controls which files are edited, the other set of filters controls which
files the clang tool is run against.

## Debugging
Dumping the AST for a file:

```shell
clang++ -cc1 -ast-dump foo.cc
```

Using `clang-query` to dynamically test matchers (requires checking out
and building [clang-tools-extras](https://github.com/llvm-mirror/clang-tools-extra)):

```shell
clang-query -p path/to/compdb base/memory/ref_counted.cc
```

`printf` debugging:

```c++
  clang::Decl* decl = result.Nodes.getNodeAs<clang::Decl>("decl");
  decl->dumpColor();
  clang::Stmt* stmt = result.Nodes.getNodeAs<clang::Stmt>("stmt");
  stmt->dumpColor();
```

By default, the script hides the output of the tool. The easiest way to change
that is to `return 1` from the `main()` function of the clang tool.

## Testing
Synposis:

```shell
tools/clang/scripts/test_tool.py <tool name> [--apply-edits]
```

The name of the tool binary and the subdirectory for the tool in
`//tools/clang` must match. The test runner finds all files that match the
pattern `//tools/clang/<tool name>/tests/*-original.cc`, and runs the tool
across those files.
If `--apply-edits` switch is presented, tool outputs are applied to respective
files and compared to the `*-expected.cc` version. If there is a mismatch, the
result is saved in `*-actual.cc`.
When `--apply-edits` switch is not presented, tool outputs are compared to
`*-expected.txt` and if different, the result is saved in `*-actual.txt`. Note
that in this case, only one test file is expected.
