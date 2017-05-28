cmake_minimum_required(VERSION 2.6)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GATTLIB REQUIRED gattlib)

include(FindCheck.cmake)

if (CHECK_FOUND)
  message(STATUS "Found check")
else (CHECK_FOUND)
  message(STATUS "Check not found")
endif(CHECK_FOUND)

add_subdirectory (src)

enable_testing()
add_subdirectory (test)

#set(ble_scan_SRCS ble_scan.c)
#set(atmotube_reader_SRCS atmotube-reader.c)

#include_directories(${GATTLIB_INCLUDE_DIRS})
#link_directories(${GATTLIB_LIBDIR})

#add_executable(ble_scan ${ble_scan_SRCS})
#target_link_libraries(ble_scan ${GATTLIB_LIBRARIES} pthread)

#add_executable(atmreader ${atmotube_reader_SRCS})
#target_link_libraries(atmreader ${GATTLIB_LIBRARIES} pthread)
