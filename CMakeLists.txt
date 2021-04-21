cmake_minimum_required(VERSION 3.1)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

project(libprint)
set( EXE_NAME ${PROJECT_NAME} )

include(cmake/CPM.cmake)
CPMAddPackage("gh:fmtlib/fmt#7.1.3")

add_executable(${EXE_NAME} libprint_test.cpp)
target_link_libraries(libprint PRIVATE fmt)
# target_link_libraries(liblog PRIVATE fmt::fmt-header-only)
