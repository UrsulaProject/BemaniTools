cmake_minimum_required(VERSION 3.18)
project(BMT)
aux_source_directory("${CMAKE_CURRENT_LIST_DIR}/lib" BMT_LIB_SRCS)
add_library(BMT STATIC "${BMT_LIB_SRCS}")
target_compile_features(BMT PUBLIC cxx_std_20)
target_include_directories(BMT PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")

add_executable(BFCLI tools/BFCLI.cpp)
target_link_libraries(BFCLI BMT)
