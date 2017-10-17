cmake_minimum_required(VERSION 2.6)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GATTLIB REQUIRED gattlib)

include(FindCheck.cmake)
include(FindLibConfuse.cmake)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wformat -Wunused -Wunused-function -Wunused-label -Wunused-parameter -Wunused-value -Wunused-variable -Wunused-but-set-parameter -Wunused-but-set-variable")

if (CHECK_FOUND)
  message(STATUS "Found check")
else (CHECK_FOUND)
  message(STATUS "Check not found")
endif(CHECK_FOUND)

add_subdirectory (src)

enable_testing()
add_subdirectory (test)
