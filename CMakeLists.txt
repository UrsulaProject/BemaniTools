cmake_minimum_required(VERSION 3.18)
project(BMT)

find_program(PKG_CONFIG_EXECUTABLE NAMES pkg-config HINTS /opt/homebrew/bin /usr/local/bin REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBZIP REQUIRED IMPORTED_TARGET libzip)
pkg_check_modules(LIBPLIST REQUIRED IMPORTED_TARGET libplist-2.0)
pkg_check_modules(LIBCRYPTO REQUIRED IMPORTED_TARGET libcrypto)
pkg_check_modules(JSONC REQUIRED IMPORTED_TARGET json-c)

aux_source_directory("${CMAKE_CURRENT_LIST_DIR}/lib" BMT_LIB_SRCS)
add_library(BMT STATIC "${BMT_LIB_SRCS}")
target_compile_features(BMT PUBLIC cxx_std_20)
target_include_directories(BMT PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")
target_link_libraries(BMT PUBLIC
    PkgConfig::LIBZIP
    PkgConfig::LIBPLIST
    PkgConfig::LIBCRYPTO
    PkgConfig::JSONC)

add_executable(BemaniTools tools/BemaniTools.cpp)
target_link_libraries(BemaniTools PRIVATE BMT)

include(CTest)
if(BUILD_TESTING)
    add_executable(BMTTests tests/BMTTests.cpp)
    target_link_libraries(BMTTests PRIVATE BMT)
    add_test(NAME BMTTests COMMAND BMTTests)
endif()
