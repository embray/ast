# Driver script for ctest grid tests.  Invoked via `cmake -P`
# by add_grid_test().
#
# Runs testgrid to produce an SVG file.  If REF_FILE is provided,
# extracts <text> elements from both SVGs (sorted), and diffs them.
# This compares label content and positions while ignoring platform-
# dependent grid line tessellation.
#
# Required cache variables (supplied via -D):
#   TESTGRID  : absolute path to the testgrid executable
#   HEAD_FILE : input .head file
#   ATTR      : Plot attributes string (or " " for none)
#   FATTR     : FitsChan attributes string (or " " for none)
#   OUT_FILE  : output SVG file to produce
#
# Optional:
#   REF_FILE  : reference SVG file (omit for smoke-only)
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
    # Extract sorted <text> lines from both files.
    file(STRINGS "${REF_FILE}" _ref_lines REGEX "^<text ")
    file(STRINGS "${OUT_FILE}" _out_lines REGEX "^<text ")
    list(SORT _ref_lines)
    list(SORT _out_lines)

    # Normalize integer pixel coordinates by rounding each integer down to the
    # nearest even value (floor(N/2)*2).  This gives +/-1 pixel tolerance so
    # that builds using libmvec vector-math (which may differ from scalar libm
    # by 1 ULP) still match the reference after the integer pixel rounding that
    # occurs inside the SVG plotter.  Non-coordinate numbers (font-size, label
    # text like "-20") are also rounded but identically in both files, so they
    # continue to compare equal.
    function(_normalize_pixels lines_var out_var)
        set(_result)
        foreach(_line IN LISTS ${lines_var})
            set(_normalized "")
            set(_rest "${_line}")
            while(_rest MATCHES "^([^0-9]*)([0-9]+)(.*)")
                set(_prefix "${CMAKE_MATCH_1}")
                set(_num    "${CMAKE_MATCH_2}")
                set(_rest   "${CMAKE_MATCH_3}")
                math(EXPR _rounded "${_num} / 2 * 2")
                string(APPEND _normalized "${_prefix}${_rounded}")
            endwhile()
            string(APPEND _normalized "${_rest}")
            list(APPEND _result "${_normalized}")
        endforeach()
        set(${out_var} "${_result}" PARENT_SCOPE)
    endfunction()

    _normalize_pixels(_ref_lines _ref_norm)
    _normalize_pixels(_out_lines _out_norm)

    # Write to temp files for diffing.
    string(REPLACE ";" "\n" _ref_text "${_ref_norm}")
    string(REPLACE ";" "\n" _out_text "${_out_norm}")
    file(WRITE "${OUT_FILE}.ref.txt" "${_ref_text}\n")
    file(WRITE "${OUT_FILE}.out.txt" "${_out_text}\n")

    execute_process(
        COMMAND "${CMAKE_COMMAND}" -E compare_files
            "${OUT_FILE}.ref.txt" "${OUT_FILE}.out.txt"
        RESULT_VARIABLE _rv
    )
    if(NOT _rv EQUAL 0)
        message(STATUS "Text label mismatch; showing diff:")
        execute_process(
            COMMAND diff -u "${OUT_FILE}.ref.txt" "${OUT_FILE}.out.txt"
        )
        message(FATAL_ERROR
            "Grid text labels in ${OUT_FILE} differ from reference ${REF_FILE}")
    endif()
endif()
