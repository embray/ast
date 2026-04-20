# wcsconverter C Port Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Port `ast_tester/wcsconverter.f` to C and register each of its 14 WCS-conversion fixtures as an independently reported ctest that diffs output against a committed reference file.

**Architecture:** Standalone C executable `wcsconverter` (public AST API only) + a CMake script-mode driver (`run_wcsconverter_test.cmake`) that invokes the tool and compares its output to a reference file via `diff -u`. A small `add_wcsconv_test()` helper function in `ast_tester/CMakeLists.txt` makes registering a new fixture a one-line change. The tool and all its ctest invocations pick up coverage, warning, and sanitizer flags through the existing `ast_apply_dev_options()` helper.

**Tech Stack:** C99, AST public API (`ast.h`), CMake ≥ 3.24, POSIX `unlink`/`strcasecmp`.

**Design spec:** `docs/superpowers/specs/2026-04-16-wcsconverter-c-port-design.md`

---

## File Structure

- **Create `ast_tester/wcsconverter.c`** — CLI harness. One file, one `main()`, a couple of small helpers. ~180 lines. Owns: argv parsing, FitsChan/Channel input, encoding-dispatched output, exit code.
- **Create `cmake/run_wcsconverter_test.cmake`** — CMake script-mode runner. Single-purpose: invoke binary, diff output, surface a unified diff on mismatch.
- **Modify `ast_tester/CMakeLists.txt`** — add the `wcsconverter` build target, the `add_wcsconv_test()` helper function, and 14 fixture registrations.

---

## Task 1: Port wcsconverter.f to C (build only, no tests yet)

**Files:**
- Create: `ast_tester/wcsconverter.c`
- Modify: `ast_tester/CMakeLists.txt` (append build target only, no tests yet)

Goal of this task: get the binary to build and run a single end-to-end conversion manually. No ctest integration yet.

- [ ] **Step 1: Create `ast_tester/wcsconverter.c`**

```c
/*
 *  wcsconverter: read a WCS FrameSet from an input file and write it out
 *  in a specified encoding.  Ported from ast_tester/wcsconverter.f.
 *
 *  Usage:
 *     wcsconverter <in file> <encoding> <out file> [<attrs>]
 *
 *  Reads a FrameSet from "in file" (as FITS headers if possible,
 *  otherwise as an AST dump), and writes the FrameSet to "out file"
 *  using the specified encoding.
 *
 *  Parameters:
 *     in file   A text file containing FITS headers or an AST dump of a
 *               FrameSet.
 *     encoding  A FITS encoding name (e.g. "fits-wcs", "native", "dss",
 *               "fits-iraf", "fits-aips", "fits-pc"), or "ast".
 *     out file  The output file.  An AST dump if encoding is "ast",
 *               otherwise a sequence of 80-character FITS cards.
 *     attrs     Optional attribute settings applied to the input FitsChan
 *               before reading the headers (e.g. "cdmatrix=1,FitsDigits=8").
 *
 *  Differences from the Fortran original:
 *
 *  - err_mark / err_rlse / err_annul are omitted; errors are handled
 *    with astWatch(&status) + astOK + astClearStatus.
 *
 *  - The Fortran ObjectCaching diagnostic prints (which only fire when
 *    the default value is unexpected) are dropped.  astTune is still
 *    called silently so memory behaviour matches the Fortran original.
 *
 *  - The SOURCE and SINK Fortran subroutines that shuttled lines through
 *    UNIT=10 are replaced by astChannel's SourceFile / SinkFile
 *    attributes; no callback plumbing is needed.  FITS-card output still
 *    uses fopen/fprintf driven by an astFindFits("%f", ...) loop because
 *    astFindFits is the natural iterator for card-by-card output.
 *
 *  - Fortran DELETEFILE subroutine replaced by unlink(3).
 *
 *  - CHR_LEN is replaced by strlen.
 *
 *  - Exit code is non-zero on usage error (Fortran silently RETURNs) so
 *    ctest can distinguish success from "no args supplied".
 *
 *  - Both "AST" and "ast" encoding strings are accepted, matching the
 *    Fortran behaviour via strcasecmp.
 */

#include "ast.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

int main( int argc, char *argv[] ) {
   int status_value = 0;
   int *status = &status_value;

   AstFitsChan *fc = NULL;
   AstChannel *chan = NULL;
   AstObject *object = NULL;
   FILE *fp = NULL;
   char line[256];
   char *card = NULL;

   if( argc < 4 ) {
      fprintf( stderr,
               "Usage: wcsconverter <in file> <encoding> <out file> "
               "[<attrs>]\n" );
      return 1;
   }

   const char *in_file   = argv[1];
   const char *encoding  = argv[2];
   const char *out_file  = argv[3];
   const char *attrs     = ( argc >= 5 ) ? argv[4] : " ";

   astWatch( status );

   /* Use object caching to minimise allocation of new memory.  We do not
    * echo the previous value (the Fortran only printed it if it was
    * non-default; that would be stdout noise in ctest logs). */
   astTune( "ObjectCaching", 1 );

   /* Create a FitsChan and apply any attribute settings. */
   fc = astFitsChan( NULL, NULL, " " );
   if( argc >= 5 ) astSet( fc, "%s", attrs );

   /* Read each line out of the text file and store it in the FitsChan. */
   fp = fopen( in_file, "r" );
   if( !fp ) {
      fprintf( stderr, "wcsconverter: cannot open input file '%s'\n",
               in_file );
      return 1;
   }
   while( fgets( line, (int) sizeof( line ), fp ) ) {
      size_t n = strlen( line );
      while( n > 0 && ( line[n-1] == '\n' || line[n-1] == '\r' ) ) {
         line[--n] = '\0';
      }
      astPutFits( fc, line, 0 );
   }
   fclose( fp );

   /* Set the value of CDMatrix, unless it has already been set. */
   if( !astTest( fc, "CDMATRIX" ) ) {
      int cdm = astGetI( fc, "CDMATRIX" );
      astSetI( fc, "CDMATRIX", cdm );
   }

   /* Attempt to read an Object from the FitsChan. */
   astClear( fc, "CARD" );
   object = astRead( fc );

   if( !astOK ) astClearStatus;

   /* If no object was read, attempt to read an object from the text file
    * as an AST dump via an astChannel with SourceFile set. */
   if( !object ) {
      astAnnul( fc );
      fc = NULL;
      chan = astChannel( NULL, NULL, "SourceFile=%s", in_file );
      object = astRead( chan );
      astAnnul( chan );
   }

   /* Abort if no object was read. */
   if( !object ) {
      fprintf( stderr, "wcsconverter: no WCS could be read from %s\n",
               in_file );
      return 1;
   }

   /* Delete any pre-existing output file. */
   unlink( out_file );

   /* Otherwise write out the object using the specified encoding. */
   if( strcasecmp( encoding, "AST" ) == 0 ) {

      chan = astChannel( NULL, NULL, "SinkFile=%s", out_file );
      if( astWrite( chan, object ) != 1 ) {
         fprintf( stderr,
                  "wcsconverter: WCS read from %s could not be converted "
                  "to %s format.\n", in_file, encoding );
      }
      astAnnul( chan );

   } else {

      if( !fc ) fc = astFitsChan( NULL, NULL, "%s", attrs );
      astSetC( fc, "ENCODING", encoding );
      astClear( fc, "CARD" );

      if( astWrite( fc, object ) != 1 ) {
         fprintf( stderr,
                  "wcsconverter: WCS read from %s could not be converted "
                  "to %s format.\n", in_file, encoding );
      } else {
         fp = fopen( out_file, "w" );
         if( !fp ) {
            fprintf( stderr,
                     "wcsconverter: cannot open output file '%s'\n",
                     out_file );
            astAnnul( fc );
            astAnnul( object );
            return 1;
         }
         astClear( fc, "CARD" );
         while( astFindFits( fc, "%f", line, 1 ) ) {
            /* astFindFits returns a NUL-terminated 80-character card
             * (with any trailing blanks included).  Write exactly 80
             * chars per line to match the Fortran "A80" output. */
            fprintf( fp, "%-80.80s\n", line );
         }
         fclose( fp );
      }

      astAnnul( fc );
   }

   astAnnul( object );
   (void) card;

   return astOK ? 0 : 1;
}
```

- [ ] **Step 2: Add the build target to `ast_tester/CMakeLists.txt`**

Find the comment `# --- Fortran tests converted to C (Batch 2) ---` (currently around line 90). After the last entry in that batch (or at the very end of the file, just before the conditional PLplot block), insert:

```cmake
# --- wcsconverter: regression-diff harness ported from wcsconverter.f ---
add_executable(wcsconverter wcsconverter.c)
target_include_directories(wcsconverter PRIVATE ${TEST_INCLUDES})
target_link_libraries(wcsconverter PRIVATE ${TEST_LIBS})
target_compile_definitions(wcsconverter PRIVATE HAVE_CONFIG_H)
ast_apply_dev_options(wcsconverter)
```

The exact insertion point is: immediately before the `if(AST_ENABLE_HUGE_TEST)` block (currently line 125). This keeps the wcsconverter-related machinery contiguous and away from the conditional blocks.

- [ ] **Step 3: Configure the build**

```
cmake -B build -DCMAKE_BUILD_TYPE=Debug
```

Expected: configure succeeds; `wcsconverter` target is listed.

- [ ] **Step 4: Build the tool**

```
cmake --build build --target wcsconverter
```

Expected: `build/ast_tester/wcsconverter` produced with no warnings.

If the build fails, read the error, fix `wcsconverter.c`, rebuild. Do not proceed to the next step until the build is clean.

- [ ] **Step 5: Smoke-test by hand**

```
cp ast_tester/timj.ast /tmp/timj.ast
./build/ast_tester/wcsconverter /tmp/timj.ast fits-wcs /tmp/timj-new.fits-wcs "cdmatrix=1,FitsDigits=8"
diff -u ast_tester/timj.fits-wcs /tmp/timj-new.fits-wcs
```

Expected: `diff` produces no output (exit code 0). If there is a diff, something is wrong in the port — debug before continuing.

- [ ] **Step 6: Commit**

```
git add ast_tester/wcsconverter.c ast_tester/CMakeLists.txt
git commit -m "Add C port of wcsconverter.f regression harness"
```

---

## Task 2: CMake driver script

**Files:**
- Create: `cmake/run_wcsconverter_test.cmake`

- [ ] **Step 1: Create the driver**

```cmake
# Driver script for ctest cases that run wcsconverter and diff its output
# against a reference file.  Invoked via `cmake -P` by add_wcsconv_test().
#
# Required cache variables (supplied via -D):
#   WCSCONVERTER : absolute path to the wcsconverter executable
#   IN_FILE      : input file (resolved against CWD, i.e. the test working directory)
#   REF_FILE     : reference file to diff against
#   OUT_FILE     : output file to produce (deleted first by wcsconverter)
#   ENCODING     : encoding name, e.g. fits-wcs, native, ast, dss, ...
#   ATTRS        : attribute string, or "" for none

foreach(_req IN ITEMS WCSCONVERTER IN_FILE REF_FILE OUT_FILE ENCODING)
    if(NOT DEFINED ${_req})
        message(FATAL_ERROR "run_wcsconverter_test.cmake: ${_req} not set")
    endif()
endforeach()

if(NOT DEFINED ATTRS)
    set(ATTRS "")
endif()

set(_cmd "${WCSCONVERTER}" "${IN_FILE}" "${ENCODING}" "${OUT_FILE}")
if(NOT ATTRS STREQUAL "")
    list(APPEND _cmd "${ATTRS}")
endif()

execute_process(
    COMMAND ${_cmd}
    RESULT_VARIABLE _rv
)
if(NOT _rv EQUAL 0)
    message(FATAL_ERROR "wcsconverter exited with code ${_rv} "
                        "(cmd: ${_cmd})")
endif()

execute_process(
    COMMAND "${CMAKE_COMMAND}" -E compare_files "${REF_FILE}" "${OUT_FILE}"
    RESULT_VARIABLE _rv
)
if(NOT _rv EQUAL 0)
    message(STATUS "Output mismatch; showing unified diff:")
    execute_process(
        COMMAND diff -u "${REF_FILE}" "${OUT_FILE}"
    )
    message(FATAL_ERROR
        "wcsconverter output ${OUT_FILE} differs from reference ${REF_FILE}")
endif()
```

- [ ] **Step 2: Commit**

```
git add cmake/run_wcsconverter_test.cmake
git commit -m "Add CMake script-mode driver for wcsconverter diff tests"
```

---

## Task 3: Register the first fixture and verify it passes

**Files:**
- Modify: `ast_tester/CMakeLists.txt`

Goal: add the helper function and the first fixture (`timj_fits-wcs`). Once this passes under ctest, the remaining fixtures are trivial one-line additions in Task 4.

- [ ] **Step 1: Add the helper function and first fixture**

In `ast_tester/CMakeLists.txt`, directly below the `wcsconverter` build target added in Task 1, insert:

```cmake
# Helper to register one wcsconverter regression-diff test.
# Each call copies the input + reference file into the build tree and
# registers a single ctest whose command is a cmake -P invocation of
# run_wcsconverter_test.cmake.  Adding a new fixture is a one-line
# add_wcsconv_test(...) call below, plus dropping the input/reference
# files into ast_tester/.
function(add_wcsconv_test)
    cmake_parse_arguments(W "" "NAME;INPUT;REFERENCE;ENCODING;ATTRS" "" ${ARGN})
    if(NOT W_NAME OR NOT W_INPUT OR NOT W_REFERENCE OR NOT W_ENCODING)
        message(FATAL_ERROR
            "add_wcsconv_test: NAME, INPUT, REFERENCE, ENCODING all required")
    endif()
    if(NOT DEFINED W_ATTRS)
        set(W_ATTRS "")
    endif()
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/${W_INPUT}"
                   "${CMAKE_CURRENT_BINARY_DIR}/${W_INPUT}" COPYONLY)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/${W_REFERENCE}"
                   "${CMAKE_CURRENT_BINARY_DIR}/${W_REFERENCE}" COPYONLY)
    add_test(NAME wcsconv_${W_NAME}
        COMMAND ${CMAKE_COMMAND}
            -DWCSCONVERTER=$<TARGET_FILE:wcsconverter>
            -DIN_FILE=${W_INPUT}
            -DREF_FILE=${W_REFERENCE}
            -DOUT_FILE=${W_NAME}.out
            -DENCODING=${W_ENCODING}
            -DATTRS=${W_ATTRS}
            -P "${CMAKE_SOURCE_DIR}/cmake/run_wcsconverter_test.cmake"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
    if(AST_ENABLE_SANITIZERS)
        set_tests_properties(wcsconv_${W_NAME} PROPERTIES
            ENVIRONMENT "ASAN_OPTIONS=detect_leaks=0")
    endif()
endfunction()

add_wcsconv_test(NAME timj_fits-wcs
                 INPUT timj.ast
                 REFERENCE timj.fits-wcs
                 ENCODING fits-wcs
                 ATTRS "cdmatrix=1,FitsDigits=8")
```

- [ ] **Step 2: Reconfigure and build**

```
cmake -B build
cmake --build build --target wcsconverter
```

Expected: configuration succeeds, no warnings.

- [ ] **Step 3: Run the one test**

```
ctest --test-dir build -R wcsconv_timj_fits-wcs --output-on-failure
```

Expected: `1/1 Test #N: wcsconv_timj_fits-wcs ... Passed`.

If the test fails, inspect the diff in the ctest log, identify whether the diff is real (code bug) or spurious (e.g. trailing whitespace, line endings). Fix `wcsconverter.c` or the driver script, rebuild, re-run. Do not proceed until green.

- [ ] **Step 4: Commit**

```
git add ast_tester/CMakeLists.txt
git commit -m "Register first wcsconverter regression test (timj_fits-wcs)"
```

---

## Task 4: Register the remaining 13 fixtures

**Files:**
- Modify: `ast_tester/CMakeLists.txt`

- [ ] **Step 1: Add the remaining fixture registrations**

Immediately below the `add_wcsconv_test(NAME timj_fits-wcs ...)` line added in Task 3, append:

```cmake
add_wcsconv_test(NAME timj_fits-iraf
                 INPUT timj.ast
                 REFERENCE timj.fits-iraf
                 ENCODING fits-iraf
                 ATTRS "FitsDigits=8")

add_wcsconv_test(NAME timj_fits-aips
                 INPUT timj.ast
                 REFERENCE timj.fits-aips
                 ENCODING fits-aips
                 ATTRS "FitsDigits=8")

add_wcsconv_test(NAME timj_fits-pc
                 INPUT timj.ast
                 REFERENCE timj.fits-pc
                 ENCODING fits-pc
                 ATTRS "fitsrounding=0,FitsDigits=8")

add_wcsconv_test(NAME timj_native
                 INPUT timj.ast
                 REFERENCE timj.native
                 ENCODING native
                 ATTRS "FitsDigits=8")

add_wcsconv_test(NAME a20070718_fits-wcs
                 INPUT a20070718_00010_02_cube.ast
                 REFERENCE a20070718_00010_02_cube.fits-wcs
                 ENCODING fits-wcs
                 ATTRS "FitsDigits=8")

add_wcsconv_test(NAME dss_fits-dss_ast
                 INPUT dss.fits-dss
                 REFERENCE dss.ast
                 ENCODING ast
                 ATTRS "FitsDigits=8")

add_wcsconv_test(NAME dss_ast_dss
                 INPUT dss.ast
                 REFERENCE dss.dss
                 ENCODING dss
                 ATTRS "FitsDigits=8")

add_wcsconv_test(NAME dss_ast_fits-wcs
                 INPUT dss.ast
                 REFERENCE dss.fits-wcs
                 ENCODING fits-wcs
                 ATTRS "cdmatrix=1,FitsDigits=8")

add_wcsconv_test(NAME degen1_fits-wcs
                 INPUT degen1.ast
                 REFERENCE degen1.fits-wcs
                 ENCODING fits-wcs
                 ATTRS "cdmatrix=1,FitsDigits=8")

add_wcsconv_test(NAME sip_fits-wcs
                 INPUT sip.head
                 REFERENCE sip.fits-wcs
                 ENCODING fits-wcs
                 ATTRS "cdmatrix=1,sipreplace=0,FitsDigits=8")

add_wcsconv_test(NAME sip2_fits-wcs
                 INPUT sip2.head
                 REFERENCE sip2.fits-wcs
                 ENCODING fits-wcs
                 ATTRS "cdmatrix=1,sipreplace=0,FitsDigits=8")

add_wcsconv_test(NAME lsst_fits-wcs
                 INPUT lsst.ast
                 REFERENCE lsst.fits-wcs
                 ENCODING fits-wcs
                 ATTRS "cdmatrix=1,FitsDigits=8")

add_wcsconv_test(NAME longslit_fits-wcs
                 INPUT longslit.fits-pc
                 REFERENCE longslit.fits-wcs
                 ENCODING fits-wcs
                 ATTRS "cdmatrix=1,FitsDigits=8")
```

Note: the fixture `sip.head` is already `configure_file`-copied earlier in this `CMakeLists.txt` (for `testfitschan_c`). The duplicate copy performed by `add_wcsconv_test` is harmless — `configure_file` with `COPYONLY` is a no-op when the destination is up to date.

- [ ] **Step 2: Reconfigure**

```
cmake -B build
```

Expected: configure succeeds; 14 new `wcsconv_*` tests are registered.

- [ ] **Step 3: Run all wcsconv tests**

```
cmake --build build --target wcsconverter
ctest --test-dir build -R ^wcsconv_ --output-on-failure
```

Expected: `14/14 tests passed`.

If any fail, the unified diff from the driver script will be visible in the ctest log. Investigate each failure: is it a stale reference file (check `git log` on the reference), a platform difference (e.g. floating-point formatting), or a real regression? Fix before continuing.

- [ ] **Step 4: Commit**

```
git add ast_tester/CMakeLists.txt
git commit -m "Register remaining 13 wcsconverter regression fixtures"
```

---

## Task 5: Sanitizer build verification

**Files:** none (verification only)

- [ ] **Step 1: Configure and build with sanitizers**

```
cmake -B build-san -DCMAKE_BUILD_TYPE=Debug \
                   -DAST_ENABLE_WARNINGS=ON \
                   -DAST_ENABLE_SANITIZERS=ON
cmake --build build-san --target wcsconverter
```

Expected: build succeeds. The project notes in `CLAUDE.md` say AST isn't fully warnings-clean, but the new `wcsconverter.c` must not introduce new warnings.

If new warnings appear and they name `wcsconverter.c`, fix them. Warnings in other source files are pre-existing and out of scope.

- [ ] **Step 2: Run all wcsconv tests under sanitizers**

```
ctest --test-dir build-san -R ^wcsconv_ --output-on-failure
```

Expected: `14/14 tests passed`. `ASAN_OPTIONS=detect_leaks=0` is already exported for these tests by `add_wcsconv_test` (the pattern used throughout `ast_tester/CMakeLists.txt`).

If UBSan or ASan reports an issue, fix it. A real bug in `wcsconverter.c` (e.g. a buffer overrun in the `fgets` loop) is a blocker.

- [ ] **Step 3: No commit** — this is a verification step only, nothing to save.

---

## Task 6: Coverage verification

**Files:** none (verification only)

Goal: confirm the 14 new ctests increase coverage of `src/fitschan.c` writer paths.

- [ ] **Step 1: Configure coverage build**

```
cmake -B build-cov -DCMAKE_BUILD_TYPE=Debug -DAST_ENABLE_COVERAGE=ON
cmake --build build-cov
```

Expected: build succeeds.

- [ ] **Step 2: Generate the coverage report**

```
cmake --build build-cov --target coverage-report
```

(If `lcov` is not installed the target is absent; fall back to `ctest --test-dir build-cov --output-on-failure` and inspect `.gcda` files directly with `gcov`.)

Expected output: `build-cov/coverage/lcov.info` populated; the `coverage-report` target's console output reports non-zero line counts.

- [ ] **Step 3: Inspect fitschan.c coverage**

Either:
- Build the HTML report: `cmake --build build-cov --target coverage-html`, then open `build-cov/coverage/html/src/fitschan.c.gcov.html` in a browser.
- Or: `grep -E "^(DA|FN|SF):" build-cov/coverage/lcov.info | awk -F, '/SF:.*fitschan\.c$/,/end_of_record/'` and look for non-zero `DA:<line>,<count>` entries in the `WriteFits*`, `WriteDss*`, `WriteIraf*`, `WriteAIPS*`, `WritePC*`, `WriteWcs*`, `Write` (class-level) regions of the file.

Success bar: at least one non-zero count line each in the code paths for FITS-WCS, FITS-IRAF, FITS-AIPS, FITS-PC, NATIVE, and DSS output. If any of these paths shows zero counts, investigate whether the corresponding test actually exercises that encoder (the encoder may be invoked via a generic dispatch that covers a different file).

If the target encoder path is genuinely not reached, that is a test-gap finding to document in `PLAN.md` (or open a follow-up task) rather than a blocker — the present plan is to port the Fortran harness, not add new coverage beyond what the Fortran harness itself had.

- [ ] **Step 4: No commit** — verification only.

---

## Task 7: Update PLAN.md

**Files:**
- Modify: `PLAN.md`

The project-level `PLAN.md` tracks test-conversion status (per the `CLAUDE.md` conversion pattern section). Record the wcsconverter port.

- [ ] **Step 1: Open `PLAN.md`, find the test-conversion status section**

- [ ] **Step 2: Add an entry**

Add a bullet / row (match the existing style) noting that `wcsconverter.f` has been ported to `wcsconverter.c` and that the 14 fixture cases previously driven by the `ast_tester` tcsh script are now registered as individual `wcsconv_*` ctest entries.

- [ ] **Step 3: Commit**

```
git add PLAN.md
git commit -m "Record wcsconverter.f → wcsconverter.c conversion in PLAN.md"
```

---

## Plan self-review

**Spec coverage:**
- Spec "C port of wcsconverter.f using public API" → Task 1.
- Spec "Each fixture as its own ctest entry failing with unified diff on mismatch" → Tasks 2, 3, 4 (driver script + helper + 14 registrations).
- Spec "Coverage-instrumented execution populates counts for fitschan.c paths" → Task 6.
- Spec "Adding a new fixture is a one-line change" → Task 3's `add_wcsconv_test` helper; verified mechanically by Task 4 (which adds 13 fixtures with exactly one `add_wcsconv_test(...)` call each).
- Spec "Sanitizer run must not crash any test" → Task 5.
- Spec fixture table (14 rows) → matches Tasks 3 + 4 one-to-one.
- Spec prologue-comment content → present verbatim at the top of `wcsconverter.c` in Task 1.

No spec requirement lacks a task.

**Placeholder scan:** No "TBD", no "similar to earlier", no "handle errors appropriately". Every step shows code or an exact command.

**Type consistency:**
- `add_wcsconv_test(NAME ... INPUT ... REFERENCE ... ENCODING ... ATTRS ...)` keyword arguments match between definition (Task 3) and all 14 call sites (Tasks 3, 4).
- `run_wcsconverter_test.cmake` expects cache vars `WCSCONVERTER, IN_FILE, REF_FILE, OUT_FILE, ENCODING, ATTRS` (Task 2); they are set by the `add_test` command in Task 3's helper.
- `wcsconverter.c` argv contract (`<in> <encoding> <out> [<attrs>]`) matches the order the driver script passes arguments.
