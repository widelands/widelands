execute_process(
  COMMAND /usr/bin/cppcheck ${sourcefile} --suppressions ${WL_SOURCES_BINARY_ROOT}/CppCheckSuppressions --enable=exceptNew,exceptRealloc,possibleError,unusedFunctions --force --quiet -v --template gcc -I ${includedir}
  OUTPUT_VARIABLE checkresult
  ERROR_VARIABLE checkresult2
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_STRIP_TRAILING_WHITESPACE
)

if (checkresult STREQUAL "" AND checkresult2 STREQUAL "")
  execute_process(
    COMMAND cmake -E touch ${CMAKE_CURRENT_BINARY_DIR}/${stamp}
  )
else (checkresult STREQUAL "" AND checkresult2 STREQUAL "")
  if (NOT checkresult STREQUAL "")
    message ("${checkresult}")
  endif (NOT checkresult STREQUAL "")
  if (NOT checkresult2 STREQUAL "")
    message ("${checkresult2}")
  endif (NOT checkresult2 STREQUAL "")
endif (checkresult STREQUAL "" AND checkresult2 STREQUAL "")
