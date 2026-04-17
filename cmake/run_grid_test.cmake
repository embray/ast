# Driver script for ctest grid tests.  Invoked via `cmake -P`
# by add_grid_test().
#
# Runs testgrid on a FITS header file with GRF logging enabled.
# If REF_FILE is provided, diffs the produced log against it
# (regression mode).  Otherwise just checks the exit code (smoke mode).
#
# Required cache variables (supplied via -D):
#   TESTGRID  : absolute path to the testgrid executable
#   HEAD_FILE : input .head file
#   ATTR      : Plot attributes string (or " " for none)
#   FATTR     : FitsChan attributes string (or " " for none)
#   OUT_FILE  : output log file to produce
#
# Optional:
#   REF_FILE  : reference log file to diff against (omit for smoke-only)
#   BOX       : graphics bounds "xlo ylo xhi yhi" (omit for auto)

foreach(_req IN ITEMS TESTGRID HEAD_FILE ATTR FATTR OUT_FILE)
    if(NOT DEFINED ${_req})
        message(FATAL_ERROR "run_grid_test.cmake: ${_req} not set")
    endif()
endforeach()

set(_cmd "${TESTGRID}" "${HEAD_FILE}" "${ATTR}" "${FATTR}" "${OUT_FILE}")
if(DEFINED BOX AND NOT BOX STREQUAL "")
    separate_arguments(_box UNIX_COMMAND "${BOX}")
    list(APPEND _cmd ${_box})
endif()

execute_process(
    COMMAND ${_cmd}
    RESULT_VARIABLE _rv
)
if(NOT _rv EQUAL 0)
    message(FATAL_ERROR "testgrid exited with code ${_rv} "
                        "(cmd: ${_cmd})")
endif()

if(DEFINED REF_FILE AND NOT REF_FILE STREQUAL "")
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
            "testgrid output ${OUT_FILE} differs from reference ${REF_FILE}")
    endif()
endif()
