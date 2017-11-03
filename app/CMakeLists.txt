set(atmotube_reader_SRCS atmotube-reader.c)

include_directories(${GATTLIB_INCLUDE_DIRS})
include_directories(${LIBCONFUSE_INCLUDE_DIR})

include_directories(${atmotube-reader_SOURCE_DIR}/include)

link_directories(${GATTLIB_LIBDIR})

add_executable(atmreader ${atmotube_reader_SRCS})
target_link_libraries(atmreader atmlib)
target_link_libraries(atmreader ${LIBCONFUSE_LIBRARY} ${GATTLIB_LIBRARIES} pthread dl)

set(atmotube_find_SRCS atmotube-find.c)

add_executable(atmfind ${atmotube_find_SRCS})
target_link_libraries(atmfind atmlib)
target_link_libraries(atmfind ${LIBCONFUSE_LIBRARY} ${GATTLIB_LIBRARIES} pthread dl)
