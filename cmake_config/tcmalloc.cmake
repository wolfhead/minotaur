find_library(TCMALLOC_LIBRARIES libtcmalloc.a /usr/local/lib NO_DEFAULT_PATH)
find_library(LIBUNWIND_LIBRARIES libunwind.a /usr/local/lib NO_DEFAULT_PATH)
message(STATUS "tcmalloc library:" ${TCMALLOC_LIBRARIES})
message(STATUS "libunwind library:" ${LIBUNWIND_LIBRARIES})
