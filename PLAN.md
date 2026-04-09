# Plan: AST Test Coverage Enhancements

## Context

The CMake build originally had a single minimal installation test (`ast_test.c`).
The `ast_tester/` directory contains 10 C test programs and 32 Fortran test
programs that were only runnable via a Starlink-specific tcsh script. Many AST
classes had no C test coverage — their only tests were Fortran programs
depending on Starlink libraries (EMS, CHR, PSX). The goal is to:

1. Add the existing C tests to the CMake build
2. Convert Fortran tests to C to eliminate the Fortran/Starlink dependency

## Current status: 43 tests passing by default, plus 1 optional huge stress test

| Phase | Status |
|-------|--------|
| Phase 1: Add existing C tests to CMake | **Complete** |
| Phase 2 Batch 1: Simple Fortran conversions | **Complete** (8 tests) |
| Phase 2 Batch 2: Medium Fortran conversions | **Complete** (6 tests) |
| Phase 2 Batch 3: Larger Fortran conversions | **Complete** (5 tests) |
| Phase 2 Batch 4: Checkdump-pattern tests | **Complete** (4 tests) |
| Phase 2 Batch 5: Channel-callback tests | **Complete** |
| Phase 2 Batch 6: Huge/manual stress tests | **Complete** (1 test) |
| Phase 2 Batch 7: Final large tests | **Complete** (2 tests) |
| Phase 3: CI integration | **Complete** (tests run via ctest) |

### Test inventory (43 default + 1 optional)

**Original test (1):**
- ast_test — minimal installation check

**Existing C tests added to CMake (10):**
- testerror, testobject, testconvert, testresimp, testaxis, testframe,
  testunitnorm, testsplinemap_c, testyamlchan (conditional), testthreads (conditional)

**Fortran tests converted to C (31 default + 1 optional):**
- Batch 1: testzoommap, testnormmap, testmapping, testskyframe, testcmpframe,
  testlutmap, testratemap, testchannel
- Batch 2: testrate, testspecframe, testflux, testspecflux, testcmpmap, testpolymap
- Batch 3: testchebymap, testunitnormmap, testtrangrid, testmoc, testfitstable
- Batch 4: testframeset, testswitchmap, testtime, testkeymap
- Batch 5: testmocchan, testxmlchan, testtable, teststcschan, testfitschan
- Batch 6: testregions, testplotter (conditional on PLplot)
- Batch 7: testrebin, teststc
- Optional manual stress test: testhuge_c

## Phase 1 details (complete)

Created stub headers for Starlink dependencies:
- `cmake/sae_par.h` — defines `SAI__OK` and `SAI__ERROR`
- `cmake/star/mers.h` — inline no-op stubs for `errMark`, `errRlse`, `errStat`, `errAnnul`, `errRep`

Created `ast_tester/CMakeLists.txt` with:
- `WHOLE_ARCHIVE` linking for satellite libraries (ast_err, ast_grf_*)
  to resolve symbols referenced by libast at runtime
- Conditional tests for YAML and pthreads support
- Data file copying for testsplinemap_c and testcmpmap

Fixed `testobject.c` to use `baseName()` for portable `__FILE__` comparison
and `__LINE__` captures instead of hardcoded line numbers.

Fixed `testsplinemap_c.c` to check `fscanf` return values.

Fixed `testthreads.c` to return NULL from `worker()` thread function.

## Phase 2 conversion approach

Each converted test:
- Uses the **public C API** (`ast.h`) only
- Uses standard C strings and `astMalloc`/`astFree` (no CHR/PSX)
- Uses `astWatch(&status)` + `astOK` for error checking (no EMS)
- Fortran `err_mark`/`err_rlse` simply omitted
- Fortran channel source/sink callbacks replaced by `astToString`/`astFromString`
  or Channel `SinkFile`/`SourceFile` attributes
- Fortran `psx_calloc`/`psx_free` replaced by `astMalloc`/`astFree`
- Fortran 1-based indices adjusted to C 0-based where needed (e.g. `astGetCell`)

### Known differences from Fortran originals

Each converted test file has a header comment documenting any differences.
Key issues:

- **testfitstable.c**: String column sizes differ because C `astMapPut1C`
  stores actual string content without padding, while Fortran CHARACTER
  variables are stored at their declared length. This affects `ColumnSize`,
  `ColumnLenC`, `NAXIS1`, `TFORM`, and `TDIM` header values. The test skips
  hardcoded size checks and verifies round-trips instead.
  **TODO**: Decide whether the C API should pad strings for FITS BINTABLE
  conventions.

- **testunitnormmap.c**: The `differ()` comparison function has an absolute
  tolerance floor of `1e-14` not present in the Fortran original, needed
  because near-zero values can differ between simplified and unsimplified
  Mappings.

- **testhuge.c**: This is a faithful C port of the original huge 64-bit
  stress test. It is built only when `AST_ENABLE_HUGE_TEST=ON` and should
  be treated as a manual test rather than a routine CI test. It allocates
  two `60001 x 60001` float arrays and can take a very long time to run,
  especially under sanitizers.

- **testzoommap.c**: Simplified immutability error recovery (checks `!astOK`
  rather than specific `AST__IMMUT` code).

- **testfitschan.c**: FITS card padding ignored during assertions to match Fortran string comparison rules. Fixed a `heap-use-after-free` bug in `astStore_` (memory.c) exposed by AddressSanitizer during `astConvert`.

- **testregions.c**: Translated automatically and then fixed up to cast `astTranN` input arrays to `(const double *)`.
- **testrebinseq.c**: Translated to use `astRebinSeq[I|F|D]` depending on the types of the in/out pointers.

- **testrebin.c**: Collapses 27 Fortran subroutines (9 test groups × 3 type
  variants each) into 9 parameterized C functions using a DataType enum.
  Tests astRebinD/F/I across 6 spread functions.

- **teststc.c**: Tests STC classes using 10 external XML data files. Uses
  safeGetC() wrapper and strcmpTrim() for Fortran trailing-space semantics.
  astTranN arrays use in[ncoord][npoint] layout (Fortran column-major order).

## Phase 2 remaining work

### Unconverted Fortran tests
- testplot3d.f (1357 lines) — Plot3D (requires PGPLOT; interactive/graphical
  test that cannot be fully converted without a graphics backend)

## Phase 3 details (complete)

The GitHub Actions workflow runs `ctest` which automatically picks up all
test targets. No workflow changes were needed. Tests conditional on YAML
and pthreads are handled by CMake options.

## Huge stress test

The original `testhuge.f` now has a C port in `ast_tester/testhuge.c`.
Because it is intentionally enormous, it is not built by default.

Enable it with:

```
cmake -B build-huge -DCMAKE_BUILD_TYPE=Debug -DAST_ENABLE_HUGE_TEST=ON
cmake --build build-huge --target testhuge_c
```

Run it manually:

```
./build-huge/ast_tester/testhuge_c
```

This test is a manual stress test, not a routine regression test. It can
consume tens of gigabytes of RAM and may take a very long time to complete.
It should not normally be combined with sanitizers.

## Verification

```
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```
Shows 43 default tests, all passing.
