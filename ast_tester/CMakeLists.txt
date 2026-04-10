# Test programs from the ast_tester directory.
#
# These are the existing C test programs, plus Fortran tests that have
# been converted to C.  All tests use CTest and do not require any
# Starlink infrastructure.

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
        set(TARGET_LIBS ast $<LINK_LIBRARY:WHOLE_ARCHIVE,ast_err> m ${T_EXTRA_LIBS})
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

# --- Tests using internal headers (#define astCLASS) ---
ast_add_test(testconvert INTERNAL_HEADERS)
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
        EXTRA_LIBS $<LINK_LIBRARY:WHOLE_ARCHIVE,ast_grf_plplot> $<LINK_LIBRARY:WHOLE_ARCHIVE,ast_grf3d> ${PLPLOT_LIBRARIES}
        TEST_ARGS "lsst.fits-wcs" " " " " "testplotter.ps"
    )
    target_include_directories(testplotter PRIVATE ${PLPLOT_INCLUDE_DIRS})
    target_link_directories(testplotter PRIVATE ${PLPLOT_LIBRARY_DIRS})
endif()
