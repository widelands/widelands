file (WRITE ${CMAKE_CURRENT_BINARY_DIR}/VERSION "${WL_VERSION}")

configure_file (${SOURCE_DIR}/src/build_info.cc.cmake ${BINARY_DIR}/src/build_info.cc)

message (STATUS "Version of Widelands Build is ${WL_VERSION} ${CMAKE_BUILD_TYPE}")
