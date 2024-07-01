cmake_minimum_required(VERSION 3.29)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

project(libprint)
set( EXE_NAME ${PROJECT_NAME} )

set(FMT_VERSION 7.0.0)
include(cmake/CPM.cmake)
CPMAddPackage("gh:fmtlib/fmt#${FMT_VERSION}")

include_directories("include")

add_executable(${EXE_NAME} libprint_test.cpp)
target_link_libraries(libprint PRIVATE fmt)

target_compile_features(libprint PUBLIC cxx_std_20)
set_target_properties(libprint PROPERTIES CXX_EXTENSIONS OFF)
