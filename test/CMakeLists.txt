set(atmotube_test_SRCS atmotube-test.c)

include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${GATTLIB_INCLUDE_DIRS})
include_directories(${CHECK_INCLUDE_DIRS})
include_directories(${atmotube-reader_SOURCE_DIR}/include)

link_directories(${GATTLIB_LIBDIR})

add_executable(atmreadertest ${atmotube_test_SRCS})
target_link_libraries(atmreadertest atmlib)
target_link_libraries(atmreadertest ${LIBCONFUSE_LIBRARY} ${GATTLIB_LIBRARIES} ${CHECK_LIBRARIES} pthread dl)

#add_custom_command(
#OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/config.txt
#COMMAND ${CMAKE_COMMAND}
#-E copy ${CMAKE_CURRENT_SOURCE_DIR}/config.txt ${CMAKE_CURRENT_BINARY_DIR}/config.txt
#DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/config.txt)

add_test (NAME test COMMAND atmreadertest WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
