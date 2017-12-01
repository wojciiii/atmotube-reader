cmake_minimum_required(VERSION 2.6)
project(atmotube-reader)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GATTLIB REQUIRED gattlib)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
#include(cmake/FindCheck.cmake)
#include(cmake/FindLibConfuse.cmake)
#include(cmake/FindSQLite3.cmake)
find_package(LibConfuse)
find_package(Check)
find_package(SQLite3)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -Wall -Wformat -Wunused -Wunused-function -Wunused-label -Wunused-parameter -Wunused-value -Wunused-variable -Wunused-but-set-parameter -Wunused-but-set-variable -Wmissing-braces -Wextra -pedantic -g -fdiagnostics-color=auto")

if (CHECK_FOUND)
  message(STATUS "Found check")
else (CHECK_FOUND)
  message(STATUS "Check not found")
endif(CHECK_FOUND)

add_subdirectory (src)
add_subdirectory (src/plugin)
add_subdirectory (app)

enable_testing()
add_subdirectory (test)
