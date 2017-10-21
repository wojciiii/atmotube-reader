set(atmotube_reader_SRCS atmotube-reader.c)

include_directories(${GATTLIB_INCLUDE_DIRS})
include_directories(${LIBCONFUSE_INCLUDE_DIR})

link_directories(${GATTLIB_LIBDIR})

add_library (atmlib atmotube.h atmotube-private.h atmotube.c atmotube-handler.c atmotube-config.h atmotube-config.c interval.h interval.c output.h output.c)

add_executable(atmreader ${atmotube_reader_SRCS})
target_link_libraries(atmreader atmlib)
target_link_libraries(atmreader ${LIBCONFUSE_LIBRARY} ${GATTLIB_LIBRARIES} pthread)

set(atmotube_find_SRCS atmotube-find.c)

add_executable(atmfind ${atmotube_find_SRCS})
target_link_libraries(atmfind atmlib)
target_link_libraries(atmfind ${LIBCONFUSE_LIBRARY} ${GATTLIB_LIBRARIES} pthread)
