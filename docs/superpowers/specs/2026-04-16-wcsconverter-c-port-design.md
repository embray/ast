# wcsconverter Fortran → C port

## Background

`ast_tester/wcsconverter.f` is a small CLI harness used as a regression
driver by the `ast_tester` tcsh script: for each of ~16 fixture headers,
it reads a FrameSet (from FITS cards or an AST dump), writes it out in
a requested encoding, and the script `diff`s the result against a
committed reference file. Any drift in `fitschan.c` (or related
encoding code paths) surfaces as a diff.

The CMake build (`ast_tester/CMakeLists.txt`) drives the active test
suite via `ctest` but does not currently include these conversion
regression tests. They are missed whenever a developer modifies
`fitschan.c` without manually running the old Fortran harness.

## Goals

1. A C port of `wcsconverter.f` that uses only the AST public C API,
   following the project's established Fortran→C conversion patterns.
2. Each fixture case from the Fortran `ast_tester` script registered
   as its own `ctest` entry, failing with a unified diff on output
   mismatch.
3. Coverage-instrumented execution: running `ctest` under
   `-DAST_ENABLE_COVERAGE=ON` must populate coverage counts for
   `fitschan.c` paths exercised by the conversions.
4. Adding a new fixture is a one-line change in `ast_tester/CMakeLists.txt`
   plus dropping the new input + reference file into the source tree.

## Non-goals

- Re-encoding or regenerating reference files. The existing committed
  `*.fits-wcs`, `*.fits-iraf`, `*.fits-aips`, `*.fits-pc`, `*.native`,
  `*.ast`, `*.dss` files are the source of truth.
- Supporting the Fortran-only `err_mark`/`err_rlse` semantics.
- Any runtime-plugin graphics integration — this is a text-only tool.

## Architecture

### Three pieces

1. `ast_tester/wcsconverter.c` — a standalone C executable, direct
   port of `wcsconverter.f`.
2. `cmake/run_wcsconverter_test.cmake` — a CMake script-mode driver
   (invoked via `cmake -P`) that runs the binary and diffs output.
3. `add_wcsconv_test()` function in `ast_tester/CMakeLists.txt` —
   registers each fixture as an independently reported ctest.

### wcsconverter.c

Public-API only (`ast.h`). No internal headers, no Starlink shims.

```
main(argc, argv):
    astWatch(&status)
    astTune("ObjectCaching", 1)                      // silent; no diagnostic prints
    fc = astFitsChan(NULL, NULL, " ")
    if argc >= 5: astSet(fc, argv[4])                // attrs

    fp_in = fopen(argv[1], "r")
    line loop: astPutFits(fc, line, 0)
    fclose(fp_in)

    if !astTest(fc, "CDMATRIX"):
        astSetI(fc, "CDMATRIX", astGetI(fc, "CDMATRIX"))  // self-pin default

    astClear(fc, "CARD")
    object = astRead(fc)
    if !astOK: astClearStatus

    if !object:                                      // fall back to AST dump
        astAnnul(fc); fc = NULL
        chan = astChannel(NULL, NULL, "SourceFile=%s", argv[1])
        object = astRead(chan)
        astAnnul(chan)

    if !object: print "wcsconverter: no WCS..."; exit 1

    unlink(argv[3])

    if strcasecmp(argv[2], "AST") == 0:
        chan = astChannel(NULL, NULL, "SinkFile=%s", argv[3])
        astWrite(chan, object)                       // result reported; non-fatal
        astAnnul(chan)
    else:
        if !fc: fc = astFitsChan(NULL, NULL, attrs)
        astSetC(fc, "ENCODING", argv[2])
        astClear(fc, "CARD")
        astWrite(fc, object)
        fp_out = fopen(argv[3], "w")
        astClear(fc, "CARD")
        while astFindFits(fc, "%f", line, 1): fprintf(fp_out, "%.80s\n", line)
        fclose(fp_out); astAnnul(fc)

    astAnnul(object)
    return astOK ? 0 : 1
```

The FITS-card output still uses `fopen`/`fprintf` directly because
`astFindFits` is the natural iterator for card-by-card output and the
FitsChan itself isn't a Channel (so `SinkFile` doesn't apply).

### run_wcsconverter_test.cmake

Driver script invoked via `cmake -P` with `-D`-passed arguments:
`WCSCONVERTER`, `IN_FILE`, `REF_FILE`, `OUT_FILE`, `ENCODING`, `ATTRS`.

```
execute_process(COMMAND ${WCSCONVERTER} ${IN_FILE} ${ENCODING} ${OUT_FILE} ${ATTRS}
                RESULT_VARIABLE rv)
if(rv): FATAL_ERROR "wcsconverter exited with ${rv}"

execute_process(COMMAND ${CMAKE_COMMAND} -E compare_files ${REF_FILE} ${OUT_FILE}
                RESULT_VARIABLE rv)
if(rv):
    execute_process(COMMAND diff -u ${REF_FILE} ${OUT_FILE})
    FATAL_ERROR "output mismatch for ${OUT_FILE}"
```

Using `cmake -P` (not a shell script) keeps the test cross-platform
and avoids adding a shell dependency. `diff -u` is POSIX and present
on every supported platform (macOS ships it; CMake on Windows
typically picks it up via git for Windows, but Windows isn't
currently a CI target so we don't stress this).

### CMakeLists.txt additions

```cmake
add_executable(wcsconverter wcsconverter.c)
target_include_directories(wcsconverter PRIVATE ${TEST_INCLUDES})
target_link_libraries(wcsconverter PRIVATE ${TEST_LIBS})
target_compile_definitions(wcsconverter PRIVATE HAVE_CONFIG_H)
ast_apply_dev_options(wcsconverter)    # picks up coverage/sanitizer/warning flags

function(add_wcsconv_test)
    cmake_parse_arguments(W "" "NAME;INPUT;REFERENCE;ENCODING;ATTRS" "" ${ARGN})
    configure_file(${W_INPUT}     "${CMAKE_CURRENT_BINARY_DIR}/${W_INPUT}"     COPYONLY)
    configure_file(${W_REFERENCE} "${CMAKE_CURRENT_BINARY_DIR}/${W_REFERENCE}" COPYONLY)
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
```

### Fixture list

Transcribed from `ast_tester/ast_tester` lines 165–180. The Fortran
script always appended `,FitsDigits=8` to whatever attributes were
supplied; the C registrations bake this into the `ATTRS` argument so
the list stays faithful to what the Fortran harness actually ran.

| NAME | INPUT | REFERENCE | ENCODING | ATTRS |
|---|---|---|---|---|
| timj_fits-wcs | timj.ast | timj.fits-wcs | fits-wcs | cdmatrix=1,FitsDigits=8 |
| timj_fits-iraf | timj.ast | timj.fits-iraf | fits-iraf | FitsDigits=8 |
| timj_fits-aips | timj.ast | timj.fits-aips | fits-aips | FitsDigits=8 |
| timj_fits-pc | timj.ast | timj.fits-pc | fits-pc | fitsrounding=0,FitsDigits=8 |
| timj_native | timj.ast | timj.native | native | FitsDigits=8 |
| a20070718_fits-wcs | a20070718_00010_02_cube.ast | a20070718_00010_02_cube.fits-wcs | fits-wcs | FitsDigits=8 |
| dss_fits-dss_ast | dss.fits-dss | dss.ast | ast | FitsDigits=8 |
| dss_ast_dss | dss.ast | dss.dss | dss | FitsDigits=8 |
| dss_ast_fits-wcs | dss.ast | dss.fits-wcs | fits-wcs | cdmatrix=1,FitsDigits=8 |
| degen1_fits-wcs | degen1.ast | degen1.fits-wcs | fits-wcs | cdmatrix=1,FitsDigits=8 |
| sip_fits-wcs | sip.head | sip.fits-wcs | fits-wcs | cdmatrix=1,sipreplace=0,FitsDigits=8 |
| sip2_fits-wcs | sip2.head | sip2.fits-wcs | fits-wcs | cdmatrix=1,sipreplace=0,FitsDigits=8 |
| lsst_fits-wcs | lsst.ast | lsst.fits-wcs | fits-wcs | cdmatrix=1,FitsDigits=8 |
| longslit_fits-wcs | longslit.fits-pc | longslit.fits-wcs | fits-wcs | cdmatrix=1,FitsDigits=8 |

Note: the Fortran list has `timj ast native` twice and
`"degen1 ast fits-wcs cdmatrix=1"` twice. Those are duplicate runs
that produce identical outputs and the same diff; they are redundant
and will not be duplicated in the C test list.

The Fortran also has `dss fits-dss ast` which reads DSS FITS headers
and writes an AST dump. Included above.

### Differences from the Fortran original (prologue comment content)

- `err_mark` / `err_rlse` / `err_annul` omitted; C uses
  `astWatch(&status)` + `astOK` + `astClearStatus`.
- The Fortran `ObjectCaching` diagnostic prints (which only fire when
  the default value differs) are dropped; `astTune("ObjectCaching", 1)`
  is called silently. Output file content is unaffected.
- Fortran `CHR_LEN(str)` replaced by `strlen(str)`.
- Fortran `DELETEFILE` subroutine replaced by `unlink(3)`.
- The Fortran `SOURCE`/`SINK` subroutines that shuttled lines through
  `UNIT=10` are replaced by `astChannel`'s `SourceFile` / `SinkFile`
  attributes; no callback plumbing is needed. FITS-card output still
  uses `fopen`/`fprintf` directly, driven by an `astFindFits("%f", ...)`
  loop.
- Fortran `AST__NULL` comparisons become `NULL` pointer tests.
- Uppercase `AST` or lowercase `ast` encoding string both accepted,
  matching the Fortran behaviour.

### Coverage verification

After implementation:

```
cmake -B build-cov -DAST_ENABLE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build-cov
cmake --build build-cov --target coverage-report   # (or: ctest then lcov)
```

Inspect `build-cov/coverage/lcov.info` (or the HTML report) and
confirm non-zero line counts in `src/fitschan.c` on code paths that
write out the various encodings. The bar for success: at least one
new covered line in each of the FITS-WCS, FITS-IRAF, FITS-AIPS,
FITS-PC, NATIVE, and DSS writer code paths.

## Error handling

- If `argc < 4`: print usage to stderr, exit 1 (Fortran `RETURN`s
  silently after a usage message; C should exit with non-zero so
  ctest reports the failure distinctly).
- If no Object could be read from either FitsChan or AST dump: print
  the same `"wcsconverter: no WCS could be read from ..."` message
  as the Fortran, exit 1.
- If `astWrite` returns 0 (write failed): print the same
  `"wcsconverter: WCS read from ... could not be converted to ..."`
  message, exit 1 so ctest catches it.
- Any unexpected `astOK == 0` at exit: exit 1.

## Testing

- All 14 ctest entries must pass on a standard `cmake --build build && ctest` run.
- A deliberate perturbation (e.g. editing `fitschan.c` to alter one
  output card) must cause exactly the expected subset of tests to
  fail with an informative unified diff. Revert the perturbation
  before merging.
- Coverage run must show non-zero counts in the encoder paths listed
  above.
- Sanitizer run (`-DAST_ENABLE_SANITIZERS=ON`) must not crash any of
  the 14 tests.
