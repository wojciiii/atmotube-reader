set(atmotube_test_SRCS atmotube-test.c)

include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${GATTLIB_INCLUDE_DIRS})
include_directories(${CHECK_INCLUDE_DIRS})

link_directories(${GATTLIB_LIBDIR})

add_executable(atmreadertest ${atmotube_test_SRCS})
target_link_libraries(atmreadertest atmlib)
target_link_libraries(atmreadertest ${GATTLIB_LIBRARIES} ${CHECK_LIBRARIES} pthread)

add_test (NAME test COMMAND atmreadertest)
