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
    cmake_parse_arguments(T "INTERNAL_HEADERS" "" "SOURCES;EXTRA_LIBS" ${ARGN})
    if(NOT T_SOURCES)
        set(T_SOURCES ${name}.c)
    endif()
    add_executable(${name} ${T_SOURCES})
    target_include_directories(${name} PRIVATE ${TEST_INCLUDES})
    target_link_libraries(${name} PRIVATE ${TEST_LIBS} ${T_EXTRA_LIBS})
    target_compile_definitions(${name} PRIVATE HAVE_CONFIG_H)
    add_test(NAME ${name} COMMAND ${name}
             WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
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
# --- Fortran tests converted to C ---
ast_add_test(testzoommap)
ast_add_test(testnormmap)
ast_add_test(testmapping)
ast_add_test(testskyframe)
ast_add_test(testcmpframe)
ast_add_test(testlutmap)
ast_add_test(testratemap)
ast_add_test(testchannel)

# --- Conditional tests ---
if(HAVE_YAML)
    ast_add_test(testyamlchan)
endif()

if(HAVE_PTHREADS)
    ast_add_test(testthreads EXTRA_LIBS Threads::Threads)
endif()
