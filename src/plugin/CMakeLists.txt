set(atmotube_reader_SRCS atmotube-reader.c)

#include_directories(${GATTLIB_INCLUDE_DIRS})
#include_directories(${LIBCONFUSE_INCLUDE_DIR})

#link_directories(${GATTLIB_LIBDIR})

include_directories(${SQLITE3_INCLUDE_DIRS} ../)

add_library (file SHARED file.h file.c)
add_library (db SHARED db.h db.c)

target_link_libraries (db ${SQLITE3_LIBRARIES})

#add_executable(atmreader ${atmotube_reader_SRCS})
#target_link_libraries(atmreader atmlib)
#target_link_libraries(atmreader ${LIBCONFUSE_LIBRARY} ${GATTLIB_LIBRARIES} pthread)

#set(atmotube_find_SRCS atmotube-find.c)

#add_executable(atmfind ${atmotube_find_SRCS})
#target_link_libraries(atmfind atmlib)
#target_link_libraries(atmfind ${LIBCONFUSE_LIBRARY} ${GATTLIB_LIBRARIES} pthread)
