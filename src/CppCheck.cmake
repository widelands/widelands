execute_process(
  COMMAND "${WL_SOURCE_CHECKER}" --enable=exceptNew,exceptRealloc,possibleError,unusedFunctions --force --quiet --template gcc "${sourcefile}"
  OUTPUT_VARIABLE checkresult
#  OUTPUT_FILE ${CMAKE_CURRENT_BINARY_DIR}/codecheck-stamps/${stamp}
  OUTPUT_STRIP_TRAILING_WHITESPACE
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..
)

if (checkresult STREQUAL "")
  execute_process(
    COMMAND cmake -E touch ${CMAKE_CURRENT_BINARY_DIR}/${stamp}
  )
else (checkresult STREQUAL "")
  message ("${checkresult}")
endif (checkresult STREQUAL "")
