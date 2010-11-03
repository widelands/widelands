execute_process (
  COMMAND ${PYTHON_EXECUTABLE} ${SOURCE_DIR}/utils/detect_revision.py
  OUTPUT_VARIABLE WL_VERSION
  WORKING_DIRECTORY ${SOURCE_DIR}
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

file (WRITE ${BINARY_DIR}/VERSION "${WL_VERSION}")

configure_file (${SOURCE_DIR}/src/build_info.cc.cmake ${BINARY_DIR}/src/build_info.cc)

message (STATUS "Version of Widelands Build is ${WL_VERSION}(${CMAKE_BUILD_TYPE})")