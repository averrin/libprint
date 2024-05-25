cmake_minimum_required(VERSION 3.29)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

project(libprint)
set( EXE_NAME ${PROJECT_NAME} )

include(cmake/CPM.cmake)
CPMAddPackage("gh:fmtlib/fmt#10.2.1")

include_directories("include")

add_executable(${EXE_NAME} libprint_test.cpp)
target_link_libraries(libprint PRIVATE fmt)
