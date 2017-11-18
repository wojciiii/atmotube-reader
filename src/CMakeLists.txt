include_directories(${GATTLIB_INCLUDE_DIRS})
include_directories(${LIBCONFUSE_INCLUDE_DIR})
include_directories(${atmotube-reader_SOURCE_DIR}/include)

link_directories(${GATTLIB_LIBDIR})

add_library (atmlib atmotube-private.h
  atmotube-handler.c atmotube-handler.h
  atmotube-output.c atmotube-output.h atmotube-config.h atmotube-config.c
  atmotube-plugin.c atmotube-plugin.h
  atmotube-interval.h atmotube-interval.c
  atmotube-search.c
  atmotube.c)
