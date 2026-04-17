# Test programs from the ast_tester directory.
#
# These are the existing C test programs, plus Fortran tests that have
# been converted to C.  All tests use CTest and do not require any
# Starlink infrastructure.

# Ensure test executables find the locally-built libast before any
# system-installed version (e.g. from a conda environment whose lib
# directory may be injected into RPATH by the compiler toolchain via
# $LDFLAGS, and the --disable-new-dtags flag forces RPATH which is
# searched before LD_LIBRARY_PATH).  Prepending -rpath for the build
# directory puts it first in the RPATH list.
set(CMAKE_EXE_LINKER_FLAGS
    "-Wl,-rpath,${CMAKE_BINARY_DIR} ${CMAKE_EXE_LINKER_FLAGS}")

# Common include directories for all tests
set(TEST_INCLUDES
    "${CMAKE_CURRENT_SOURCE_DIR}/.."          # top-level (pal2ast.h etc.)
    "${CMAKE_CURRENT_SOURCE_DIR}/../src"      # internal headers
    "${CMAKE_CURRENT_SOURCE_DIR}/../wcslib"   # wcslib headers
    "${CMAKE_BINARY_DIR}"                     # generated ast.h, ast_err.h, config.h
    "${CMAKE_BINARY_DIR}/src"                 # generated version.h, object.h
    "${CMAKE_SOURCE_DIR}/cmake"               # sae_par.h stub
    "${CMAKE_SOURCE_DIR}/cmake/star"          # mers.h stub
)

# Common link libraries for all tests.
# The satellite libraries (ast_err, ast_grf_*) are static archives whose
# symbols are referenced by libast at runtime (via -undefined dynamic_lookup
# on macOS).  We must force them into the executable with whole-archive
# linkage, otherwise the linker drops the unreferenced archive members and
# libast crashes on null function pointers at runtime.
set(TEST_LIBS
    ast
    $<LINK_LIBRARY:WHOLE_ARCHIVE,ast_err>
    $<LINK_LIBRARY:WHOLE_ARCHIVE,ast_grf_2.0>
    $<LINK_LIBRARY:WHOLE_ARCHIVE,ast_grf_3.2>
    $<LINK_LIBRARY:WHOLE_ARCHIVE,ast_grf_5.6>
    $<LINK_LIBRARY:WHOLE_ARCHIVE,ast_grf3d>
    m
)

# Helper function to add a test executable
function(ast_add_test name)
    cmake_parse_arguments(T "INTERNAL_HEADERS;NO_DEFAULT_GRF_LIBS" "" "SOURCES;EXTRA_LIBS;TEST_ARGS" ${ARGN})
    if(NOT T_SOURCES)
        set(T_SOURCES ${name}.c)
    endif()
    add_executable(${name} ${T_SOURCES})
    target_include_directories(${name} PRIVATE ${TEST_INCLUDES})
    
    if(T_NO_DEFAULT_GRF_LIBS)
        set(TARGET_LIBS ast $<LINK_LIBRARY:WHOLE_ARCHIVE,ast_err> ${T_EXTRA_LIBS} m)
    else()
        set(TARGET_LIBS ${TEST_LIBS} ${T_EXTRA_LIBS})
    endif()
    
    target_link_libraries(${name} PRIVATE ${TARGET_LIBS})
    target_compile_definitions(${name} PRIVATE HAVE_CONFIG_H)
    ast_apply_dev_options(${name})
    add_test(NAME ${name} COMMAND ${name} ${T_TEST_ARGS}
             WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
    # AST's memory system intentionally caches freed blocks for reuse,
    # so LeakSanitizer reports false positives. Disable leak detection.
    if(AST_ENABLE_SANITIZERS)
        set_tests_properties(${name} PROPERTIES
            ENVIRONMENT "ASAN_OPTIONS=detect_leaks=0")
    endif()
endfunction()

# --- Tests using public API only ---
ast_add_test(testerror)
ast_add_test(testobject)
ast_add_test(testconvert)

# --- Tests using internal headers (#define astCLASS) ---
ast_add_test(testresimp INTERNAL_HEADERS)

# --- Tests needing SAE_PAR stub ---
ast_add_test(testaxis)
ast_add_test(testframe)
ast_add_test(testunitnorm)

# --- Tests needing data files ---
configure_file(2dspline_c.dat "${CMAKE_CURRENT_BINARY_DIR}/2dspline_c.dat" COPYONLY)
ast_add_test(testsplinemap_c)

# --- Conditional tests ---
# --- Fortran tests converted to C (Batch 1) ---
ast_add_test(testzoommap)
ast_add_test(testnormmap)
ast_add_test(testmapping)
ast_add_test(testskyframe)
ast_add_test(testcmpframe)
ast_add_test(testlutmap)
ast_add_test(testratemap)
ast_add_test(testchannel)

# --- Fortran tests converted to C (Batch 2) ---
ast_add_test(testrate)
ast_add_test(testspecframe)
ast_add_test(testflux)
ast_add_test(testspecflux)
configure_file(splittest1.ast "${CMAKE_CURRENT_BINARY_DIR}/splittest1.ast" COPYONLY)
ast_add_test(testcmpmap)
ast_add_test(testpolymap)
ast_add_test(testchebymap)
ast_add_test(testunitnormmap_c SOURCES testunitnormmap.c)
ast_add_test(testtrangrid)
ast_add_test(testmoc)
ast_add_test(testfitstable)
ast_add_test(testframeset)
ast_add_test(testswitchmap)
ast_add_test(testtime)
ast_add_test(testkeymap)
ast_add_test(testmocchan)
ast_add_test(testxmlchan_c SOURCES testxmlchan.c)
ast_add_test(testtable_c SOURCES testtable.c)
configure_file(stcschan-test1-doc3.ast "${CMAKE_CURRENT_BINARY_DIR}/stcschan-test1-doc3.ast" COPYONLY)
configure_file(stcschan-test1-doc3-props.ast "${CMAKE_CURRENT_BINARY_DIR}/stcschan-test1-doc3-props.ast" COPYONLY)
ast_add_test(teststcschan_c SOURCES teststcschan.c)
ast_add_test(testregions_c SOURCES testregions.c)
ast_add_test(testrebinseq_c SOURCES testrebinseq.c)
ast_add_test(testrebin_c SOURCES testrebin.c)
foreach(_eg RANGE 1 10)
    configure_file(teststc_eg${_eg} "${CMAKE_CURRENT_BINARY_DIR}/teststc_eg${_eg}" COPYONLY)
endforeach()
ast_add_test(teststc_c SOURCES teststc.c)
configure_file(sip.head "${CMAKE_CURRENT_BINARY_DIR}/sip.head" COPYONLY)
configure_file(alt.header "${CMAKE_CURRENT_BINARY_DIR}/alt.header" COPYONLY)
configure_file(sparse.ast "${CMAKE_CURRENT_BINARY_DIR}/sparse.ast" COPYONLY)
ast_add_test(testfitschan_c SOURCES testfitschan.c)

# --- wcsconverter: regression-diff harness ported from wcsconverter.f ---
add_executable(wcsconverter wcsconverter.c)
target_include_directories(wcsconverter PRIVATE ${TEST_INCLUDES})
target_link_libraries(wcsconverter PRIVATE ${TEST_LIBS})
target_compile_definitions(wcsconverter PRIVATE HAVE_CONFIG_H)
ast_apply_dev_options(wcsconverter)

# ast_astequal: semantic equivalence check for AST object files.
# Used alongside string diffs for platform-robust verification.
add_executable(ast_astequal ast_astequal.c)
target_include_directories(ast_astequal PRIVATE ${TEST_INCLUDES})
target_link_libraries(ast_astequal PRIVATE ${TEST_LIBS})
target_compile_definitions(ast_astequal PRIVATE HAVE_CONFIG_H)
ast_apply_dev_options(ast_astequal)

# Helper to register wcsconverter regression tests.
# Each call copies the input + reference file into the build tree and
# registers two ctest entries:
#   wcsconv_<NAME>          -- byte-level diff of the produced output
#                              against the committed reference.  Catches
#                              unintentional changes to AST's output
#                              format.
#   wcsconv_<NAME>_astequal -- astEqual-based semantic check between the
#                              output and the reference.  Tolerates
#                              platform-specific serialisation and ulp
#                              drift.
# Adding a new fixture is a one-line add_wcsconv_test(...) call below,
# plus dropping the input/reference files into ast_tester/.
#
# Options:
#   SKIP_STRING_COMPARE -- register the astEqual test only; do not
#                          register the byte-level string-diff test.
#                          Use for fixtures whose serialised output is
#                          known to vary at the ulp level between
#                          platforms' math libraries, where a strict
#                          byte-diff is not a reliable regression gate
#                          on any single platform.
function(add_wcsconv_test)
    cmake_parse_arguments(W
        "SKIP_STRING_COMPARE"
        "NAME;INPUT;REFERENCE;ENCODING;ATTRS"
        ""
        ${ARGN})
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

    if(NOT W_SKIP_STRING_COMPARE)
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
    endif()

    add_test(NAME wcsconv_${W_NAME}_astequal
        COMMAND ${CMAKE_COMMAND}
            -DWCSCONVERTER=$<TARGET_FILE:wcsconverter>
            -DAST_ASTEQUAL=$<TARGET_FILE:ast_astequal>
            -DIN_FILE=${W_INPUT}
            -DREF_FILE=${W_REFERENCE}
            -DOUT_FILE=${W_NAME}.astequal.out
            -DENCODING=${W_ENCODING}
            -DATTRS=${W_ATTRS}
            -P "${CMAKE_SOURCE_DIR}/cmake/run_wcsconverter_astequal_test.cmake"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
    if(AST_ENABLE_SANITIZERS)
        set_tests_properties(wcsconv_${W_NAME}_astequal PROPERTIES
            ENVIRONMENT "ASAN_OPTIONS=detect_leaks=0")
    endif()
endfunction()

add_wcsconv_test(NAME timj_fits-wcs
                 INPUT timj.ast
                 REFERENCE timj.fits-wcs
                 ENCODING fits-wcs
                 ATTRS "cdmatrix=1,FitsDigits=8")

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
                 ATTRS "FitsDigits=8"
                 SKIP_STRING_COMPARE)

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

# --- .head file native-encoding round-trip tests ---
# These test FitsChan→FrameSet→Channel round-trips for .head files not
# previously covered by wcsconverter.  SKIP_STRING_COMPARE is used because
# native encoding output may vary at the ulp level between platforms.
add_wcsconv_test(NAME aitoff_native
                 INPUT aitoff.head
                 REFERENCE aitoff.native
                 ENCODING native
                 SKIP_STRING_COMPARE)

add_wcsconv_test(NAME car1_native
                 INPUT car1.head
                 REFERENCE car1.native
                 ENCODING native
                 ATTRS "carlin=1"
                 SKIP_STRING_COMPARE)

add_wcsconv_test(NAME car2_native
                 INPUT car2.head
                 REFERENCE car2.native
                 ENCODING native
                 ATTRS "carlin=1"
                 SKIP_STRING_COMPARE)

add_wcsconv_test(NAME car3_native
                 INPUT car3.head
                 REFERENCE car3.native
                 ENCODING native
                 SKIP_STRING_COMPARE)

add_wcsconv_test(NAME car4_native
                 INPUT car4.head
                 REFERENCE car4.native
                 ENCODING native
                 ATTRS "carlin=1"
                 SKIP_STRING_COMPARE)

add_wcsconv_test(NAME car5_native
                 INPUT car5.head
                 REFERENCE car5.native
                 ENCODING native
                 SKIP_STRING_COMPARE)

add_wcsconv_test(NAME car6_native
                 INPUT car6.head
                 REFERENCE car6.native
                 ENCODING native
                 SKIP_STRING_COMPARE)

add_wcsconv_test(NAME cobe_native
                 INPUT cobe.head
                 REFERENCE cobe.native
                 ENCODING native
                 SKIP_STRING_COMPARE)

add_wcsconv_test(NAME hpx_native
                 INPUT hpx.head
                 REFERENCE hpx.native
                 ENCODING native
                 SKIP_STRING_COMPARE)

add_wcsconv_test(NAME origin_native
                 INPUT origin.head
                 REFERENCE origin.native
                 ENCODING native
                 SKIP_STRING_COMPARE)

add_wcsconv_test(NAME polco_native
                 INPUT polco.head
                 REFERENCE polco.native
                 ENCODING native
                 SKIP_STRING_COMPARE)

add_wcsconv_test(NAME polco2_native
                 INPUT polco2.head
                 REFERENCE polco2.native
                 ENCODING native
                 SKIP_STRING_COMPARE)

add_wcsconv_test(NAME scp_native
                 INPUT scp.head
                 REFERENCE scp.native
                 ENCODING native
                 SKIP_STRING_COMPARE)

add_wcsconv_test(NAME serpens_native
                 INPUT serpens.head
                 REFERENCE serpens.native
                 ENCODING native
                 SKIP_STRING_COMPARE)

add_wcsconv_test(NAME specflux_native
                 INPUT specflux.head
                 REFERENCE specflux.native
                 ENCODING native
                 SKIP_STRING_COMPARE)

add_wcsconv_test(NAME timeplot_native
                 INPUT timeplot.head
                 REFERENCE timeplot.native
                 ENCODING native
                 SKIP_STRING_COMPARE)

add_wcsconv_test(NAME tnx_native
                 INPUT tnx.head
                 REFERENCE tnx.native
                 ENCODING native
                 SKIP_STRING_COMPARE)

add_wcsconv_test(NAME tsc_native
                 INPUT tsc.head
                 REFERENCE tsc.native
                 ENCODING native
                 SKIP_STRING_COMPARE)

add_wcsconv_test(NAME zpn_native
                 INPUT zpn.head
                 REFERENCE zpn.native
                 ENCODING native
                 SKIP_STRING_COMPARE)

add_wcsconv_test(NAME zpx_native
                 INPUT zpx.head
                 REFERENCE zpx.native
                 ENCODING native
                 SKIP_STRING_COMPARE)

# --- simplify: regression-diff harness ported from simplify.f ---
# Reads a Mapping from an AST dump, simplifies it, and diffs the result
# against a committed reference file.  Same pattern as wcsconverter above.
add_executable(simplify simplify.c)
target_include_directories(simplify PRIVATE ${TEST_INCLUDES})
target_link_libraries(simplify PRIVATE ${TEST_LIBS})
target_compile_definitions(simplify PRIVATE HAVE_CONFIG_H)
ast_apply_dev_options(simplify)

# Helper to register simplify regression tests.
# Each call copies the input .map and reference .simp files into the build
# tree and registers two ctest entries:
#   simplify_<NAME>          -- byte-level diff of the produced output
#                               against the committed reference.
#   simplify_<NAME>_astequal -- astEqual-based semantic check between the
#                               output and the reference (uses ast_astequal
#                               with encoding "ast").
#
# Options:
#   SKIP_STRING_COMPARE -- register the astEqual test only; do not
#                          register the byte-level string-diff test.
#                          Use for fixtures whose serialised output is
#                          known to vary at the ulp level between
#                          platforms' math libraries.
function(add_simplify_test)
    cmake_parse_arguments(S
        "SKIP_STRING_COMPARE"
        "NAME;INPUT;REFERENCE"
        ""
        ${ARGN})
    if(NOT S_NAME OR NOT S_INPUT OR NOT S_REFERENCE)
        message(FATAL_ERROR
            "add_simplify_test: NAME, INPUT, REFERENCE all required")
    endif()
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/${S_INPUT}"
                   "${CMAKE_CURRENT_BINARY_DIR}/${S_INPUT}" COPYONLY)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/${S_REFERENCE}"
                   "${CMAKE_CURRENT_BINARY_DIR}/${S_REFERENCE}" COPYONLY)

    if(NOT S_SKIP_STRING_COMPARE)
        add_test(NAME simplify_${S_NAME}
            COMMAND ${CMAKE_COMMAND}
                -DSIMPLIFY=$<TARGET_FILE:simplify>
                -DIN_FILE=${S_INPUT}
                -DREF_FILE=${S_REFERENCE}
                -DOUT_FILE=${S_NAME}.out
                -P "${CMAKE_SOURCE_DIR}/cmake/run_simplify_test.cmake"
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
        if(AST_ENABLE_SANITIZERS)
            set_tests_properties(simplify_${S_NAME} PROPERTIES
                ENVIRONMENT "ASAN_OPTIONS=detect_leaks=0")
        endif()
    endif()

    add_test(NAME simplify_${S_NAME}_astequal
        COMMAND ${CMAKE_COMMAND}
            -DSIMPLIFY=$<TARGET_FILE:simplify>
            -DAST_ASTEQUAL=$<TARGET_FILE:ast_astequal>
            -DIN_FILE=${S_INPUT}
            -DREF_FILE=${S_REFERENCE}
            -DOUT_FILE=${S_NAME}.astequal.out
            -P "${CMAKE_SOURCE_DIR}/cmake/run_simplify_astequal_test.cmake"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
    if(AST_ENABLE_SANITIZERS)
        set_tests_properties(simplify_${S_NAME}_astequal PROPERTIES
            ENVIRONMENT "ASAN_OPTIONS=detect_leaks=0")
    endif()
endfunction()

add_simplify_test(NAME brad
                  INPUT brad.map
                  REFERENCE brad.simp)

add_simplify_test(NAME lsst1
                  INPUT lsst1.map
                  REFERENCE lsst1.simp)

add_simplify_test(NAME rigby
                  INPUT rigby.map
                  REFERENCE rigby.simp
                  SKIP_STRING_COMPARE)

if(AST_ENABLE_HUGE_TEST)
    ast_add_test(testhuge_c SOURCES testhuge.c)
endif()

# --- Conditional tests ---
if(HAVE_YAML)
    ast_add_test(testyamlchan)
endif()

if(HAVE_PTHREADS)
    ast_add_test(testthreads EXTRA_LIBS Threads::Threads)
endif()

if(PLPLOT_FOUND)
    configure_file(lsst.fits-wcs "${CMAKE_CURRENT_BINARY_DIR}/lsst.fits-wcs" COPYONLY)
    ast_add_test(testplotter
        NO_DEFAULT_GRF_LIBS
        EXTRA_LIBS $<LINK_LIBRARY:WHOLE_ARCHIVE,ast_grf_plplot> $<LINK_LIBRARY:WHOLE_ARCHIVE,ast_grf3d_plplot> ${PLPLOT_LIBRARIES}
        TEST_ARGS "lsst.fits-wcs" " " " " "testplotter.ps"
    )
    target_include_directories(testplotter PRIVATE ${PLPLOT_INCLUDE_DIRS})
    target_link_directories(testplotter PRIVATE ${PLPLOT_LIBRARY_DIRS})

    configure_file(plot3d-test1.ast "${CMAKE_CURRENT_BINARY_DIR}/plot3d-test1.ast" COPYONLY)
    ast_add_test(testplot3d
        NO_DEFAULT_GRF_LIBS
        EXTRA_LIBS $<LINK_LIBRARY:WHOLE_ARCHIVE,ast_grf_plplot> $<LINK_LIBRARY:WHOLE_ARCHIVE,ast_grf3d_plplot> ${PLPLOT_LIBRARIES}
        TEST_ARGS "testplot3d.pdf"
    )
    target_include_directories(testplot3d PRIVATE ${PLPLOT_INCLUDE_DIRS})
    target_link_directories(testplot3d PRIVATE ${PLPLOT_LIBRARY_DIRS})

    # --- Plotter smoke tests ---
    # Run testplotter on each .head file with its .attr/.fattr/.box
    # companions.  Pure smoke test: verifies astGrid does not crash
    # on the given WCS.  No output comparison.
    function(add_plotter_test)
        cmake_parse_arguments(P ""
            "NAME;HEAD;ATTR;FATTR;BOX"
            ""
            ${ARGN})
        if(NOT P_NAME OR NOT P_HEAD)
            message(FATAL_ERROR
                "add_plotter_test: NAME, HEAD required")
        endif()
        if(NOT DEFINED P_ATTR)
            set(P_ATTR " ")
        endif()
        if(NOT DEFINED P_FATTR)
            set(P_FATTR " ")
        endif()
        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/${P_HEAD}"
                       "${CMAKE_CURRENT_BINARY_DIR}/${P_HEAD}" COPYONLY)

        set(_box_arg "")
        if(DEFINED P_BOX)
            set(_box_arg "${P_BOX}")
        endif()

        add_test(NAME plotter_${P_NAME}
            COMMAND ${CMAKE_COMMAND}
                -DTESTPLOTTER=$<TARGET_FILE:testplotter>
                -DHEAD_FILE=${P_HEAD}
                "-DATTR=${P_ATTR}"
                "-DFATTR=${P_FATTR}"
                -DOUT_FILE=plotter_${P_NAME}.pdf
                "-DBOX=${_box_arg}"
                -P "${CMAKE_SOURCE_DIR}/cmake/run_plotter_test.cmake"
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
        if(AST_ENABLE_SANITIZERS)
            set_tests_properties(plotter_${P_NAME} PROPERTIES
                ENVIRONMENT "ASAN_OPTIONS=detect_leaks=0")
        endif()
    endfunction()

    add_plotter_test(NAME aitoff
                     HEAD aitoff.head
                     ATTR "Grid=1,tickall=0,border=1,tol=0.001"
                     BOX "10.0 -10.0 300.0 280.0")

    add_plotter_test(NAME car1
                     HEAD car1.head
                     ATTR "numlabgap=0.05"
                     FATTR "carlin=1"
                     BOX "0 0 2962 562")

    add_plotter_test(NAME car2
                     HEAD car2.head
                     ATTR "numlabgap(2)=0.05,labelling=interior"
                     FATTR "carlin=1"
                     BOX "0 0 2962 562")

    add_plotter_test(NAME car3
                     HEAD car3.head
                     ATTR "grid=1"
                     BOX "0 0 400 400")

    add_plotter_test(NAME car4
                     HEAD car4.head
                     ATTR "format(1)=gd,format(2)=gd,Grid=1,tickall=0,width(axes)=3"
                     FATTR "carlin=1"
                     BOX "0 0 951 1851")

    add_plotter_test(NAME car5
                     HEAD car5.head
                     ATTR "border=1"
                     BOX "0 0 951 1851")

    add_plotter_test(NAME car6
                     HEAD car6.head
                     ATTR "format(1)=ghms,format(2)=gdm,edge(2)=r,tickall=0,grid=1"
                     BOX "0 0 5401 1741")

    add_plotter_test(NAME cobe
                     HEAD cobe.head
                     ATTR "Grid=0,labelling=exterior,gap(1)=50"
                     BOX "10.0 -10.0 300.0 280.0")

    add_plotter_test(NAME hpx
                     HEAD hpx.head
                     ATTR "Grid=1,labelling=interior,bottom(1)=0"
                     BOX "0.5 0.5 300.5 200.5")

    add_plotter_test(NAME origin
                     HEAD origin.head
                     ATTR "Grid=1,labelling=int"
                     BOX "0 0 500 500")

    add_plotter_test(NAME polco
                     HEAD polco.head
                     ATTR "border=1"
                     BOX "-300 -300 500 500")

    add_plotter_test(NAME polco2
                     HEAD polco2.head
                     BOX "-300 -300 500 500")

    add_plotter_test(NAME scp
                     HEAD scp.head
                     ATTR "border=1"
                     BOX "1.0 1.0 1500.0 1300.0")

    add_plotter_test(NAME serpens
                     HEAD serpens.head
                     BOX "0.5 0.5 150.5 120.5")

    add_plotter_test(NAME specflux
                     HEAD specflux.head
                     ATTR "system=freq,logplot=1,loggap(1)=100"
                     BOX "1 3 1.0E-8 9")

    add_plotter_test(NAME timeplot
                     HEAD timeplot.head
                     ATTR "minticklen=0.02,majticklen=0.05,gap(2)=0.1,NumLabGap=0.05,textlabgap=0.1,size=0.9"
                     BOX "0.046 3.0 1.008 3.3")

    add_plotter_test(NAME tnx
                     HEAD tnx.head
                     ATTR "Grid=1,textlabgap=0.1,border=0"
                     BOX "0.0 0.0 2048.0 4096.0")

    add_plotter_test(NAME tsc
                     HEAD tsc.head
                     ATTR "border=1"
                     BOX "1 1 1600 1600")

    add_plotter_test(NAME zpn
                     HEAD zpn.head
                     ATTR "tol=0.0001"
                     BOX "1 1 200.0 200.0")

    add_plotter_test(NAME zpx
                     HEAD zpx.head
                     ATTR "Grid=1,style(grid1)=2,size(textlab2)=2")

endif()
