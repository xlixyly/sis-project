cmake_minimum_required(VERSION 3.16)
project(sis_project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED IMPORTED_TARGET libpqxx)

add_library(sis_core src/sis.cpp)
target_include_directories(sis_core PUBLIC include)
target_link_libraries(sis_core PUBLIC PkgConfig::PQXX)
target_compile_options(sis_core PRIVATE -Wall -Wextra -Wpedantic)

add_executable(sis src/main.cpp)
target_link_libraries(sis PRIVATE sis_core)

enable_testing()
add_executable(sis_tests tests/test_sis.cpp)
target_link_libraries(sis_tests PRIVATE sis_core)
add_test(NAME sis_unit_tests COMMAND sis_tests)
