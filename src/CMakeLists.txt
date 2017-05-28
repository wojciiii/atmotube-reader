set(atmotube_reader_SRCS atmotube-reader.c)

include_directories(${GATTLIB_INCLUDE_DIRS})
link_directories(${GATTLIB_LIBDIR})

add_library (atmlib atmotube.h atmotube.c)

add_executable(atmreader ${atmotube_reader_SRCS})
target_link_libraries(atmreader atmlib)
target_link_libraries(atmreader ${GATTLIB_LIBRARIES} pthread)
