execute_process(
  COMMAND "${PYTHON_EXECUTABLE}" "${WL_SOURCE_CHECKER}" -c "${SRC}"
  OUTPUT_VARIABLE checkresult
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

if (checkresult STREQUAL "")
  execute_process(
    COMMAND cmake -E touch ${OUTPUT_FILE}
  )
else (checkresult STREQUAL "")
  message ("${checkresult}")
endif (checkresult STREQUAL "")
