set(ENV{WL_ROOT_DIR} ${WL_ROOT_DIR})
execute_process(
  COMMAND "${PYTHON_EXECUTABLE}" "${WL_SOURCE_CHECKER}" -c "${SRC}"
  OUTPUT_VARIABLE checkresult
  RESULT_VARIABLE exitcode
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

if (checkresult STREQUAL "" AND NOT exitcode)
  execute_process(
    COMMAND cmake -E touch ${OUTPUT_FILE}
  )
else (checkresult STREQUAL "" AND NOT exitcode)
  message ("${checkresult}")
  execute_process(
    COMMAND cmake -E remove ${OUTPUT_FILE}
  )
endif (checkresult STREQUAL "" AND NOT exitcode)
