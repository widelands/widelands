execute_process(
  COMMAND "${WL_SOURCE_CHECKER}" -c "${sourcefile}"
  OUTPUT_VARIABLE checkresult
#  OUTPUT_FILE ${CMAKE_CURRENT_BINARY_DIR}/codecheck-stamps/${stamp}
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

if (checkresult STREQUAL "")
  execute_process(
    COMMAND cmake -E touch ${CMAKE_CURRENT_BINARY_DIR}/codecheck-stamps/${stamp}
  )
else (checkresult STREQUAL "")
  message ("${checkresult}")
endif (checkresult STREQUAL "")
