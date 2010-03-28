execute_process(
  COMMAND cmake -E touch ${CMAKE_CURRENT_BINARY_DIR}/codecheck-stamps/${stamp}
)

execute_process(
  COMMAND "${WL_SOURCE_CHECKER}" -c "${sourcefile}"
)