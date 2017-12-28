set(atmotube_test_common_SRCS atmotube-test-common.c atmotube-test-common.h)
set(atmotube_test_SRCS atmotube-test.c)
set(atmotube_db_test_SRCS atmotube-db-test.c)
#atmotube-test-common.c atmotube-test-common.h)

include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${GATTLIB_INCLUDE_DIRS})
include_directories(${CHECK_INCLUDE_DIRS})
include_directories(${atmotube-reader_SOURCE_DIR}/include)
include_directories(${atmotube-reader_SOURCE_DIR}/src/plugin)

link_directories(${GATTLIB_LIBDIR})

# Common:
add_library (atmotube_test_common ${atmotube_test_common_SRCS})
target_link_libraries(atmotube_test_common atmlib)

# Database test:
add_executable(atmreaderdbtest ${atmotube_db_test_SRCS})
target_link_libraries(atmreaderdbtest atmlib atmotube_test_common)
target_link_libraries(atmreaderdbtest db)
target_link_libraries(atmreaderdbtest ${LIBCONFUSE_LIBRARY} ${GATTLIB_LIBRARIES} ${CHECK_LIBRARIES} pthread dl)
add_test (NAME dbtest COMMAND atmreaderdbtest WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

add_custom_command(TARGET atmreaderdbtest POST_BUILD
  COMMAND rm -f test_db_plugin-*.db
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Removing db unittest output files"
  )

# Main test:
add_executable(atmreadertest ${atmotube_test_SRCS})
target_link_libraries(atmreadertest atmlib atmotube_test_common)
target_link_libraries(atmreadertest ${LIBCONFUSE_LIBRARY} ${GATTLIB_LIBRARIES} ${CHECK_LIBRARIES} pthread dl)

add_custom_command(TARGET atmreadertest POST_BUILD
  COMMAND rm -f *.db *.txt
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/test
  COMMENT "Removing unittest output files"
  )

add_test (NAME test COMMAND atmreadertest WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

