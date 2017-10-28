set(atmotube_reader_SRCS atmotube-reader.c)

include_directories(${GATTLIB_INCLUDE_DIRS})
include_directories(${LIBCONFUSE_INCLUDE_DIR})

link_directories(${GATTLIB_LIBDIR})

add_library (atmlib atmotube-private.h atmotube-handler.c atmotube-output.c atmotube-output.h atmotube-config.h atmotube-config.c atmotube-plugin.c atmotube-plugin.h interval.h interval.c atmotube.c atmotube.h)

add_executable(atmreader ${atmotube_reader_SRCS})
target_link_libraries(atmreader atmlib)
target_link_libraries(atmreader ${LIBCONFUSE_LIBRARY} ${GATTLIB_LIBRARIES} pthread dl)

set(atmotube_find_SRCS atmotube-find.c)

add_executable(atmfind ${atmotube_find_SRCS})
target_link_libraries(atmfind atmlib)
target_link_libraries(atmfind ${LIBCONFUSE_LIBRARY} ${GATTLIB_LIBRARIES} pthread dl)
