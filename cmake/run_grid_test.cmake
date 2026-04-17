# Driver script for ctest grid smoke tests.  Invoked via `cmake -P`
# by add_grid_test().
#
# Runs testgrid on a FITS header file and checks the exit code.
# No output comparison — this is a smoke test only (verifies astGrid
# does not crash on the given WCS).
#
# Required cache variables (supplied via -D):
#   TESTGRID  : absolute path to the testgrid executable
#   HEAD_FILE : input .head file
#   ATTR      : Plot attributes string (or " " for none)
#   FATTR     : FitsChan attributes string (or " " for none)
#
# Optional:
#   BOX       : graphics bounds "xlo ylo xhi yhi" (omit for auto)

foreach(_req IN ITEMS TESTGRID HEAD_FILE ATTR FATTR)
    if(NOT DEFINED ${_req})
        message(FATAL_ERROR "run_grid_test.cmake: ${_req} not set")
    endif()
endforeach()

set(_cmd "${TESTGRID}" "${HEAD_FILE}" "${ATTR}" "${FATTR}")
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
