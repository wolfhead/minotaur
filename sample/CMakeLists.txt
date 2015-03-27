
include_directories(
  ../source
  ${Boost_INCLUDE_DIRS})

link_libraries(
  ${MINOTAUR_LIBRARIES}
  ${Boost_LIBRARIES}
  ${LOG4CPLUS_LIBRARIES}
#${TCMALLOC_LIBRARIES}
#${LIBUNWIND_LIBRARIES}
  pthread
  rt
  -static-libstdc++ 
  -static-libgcc
)

add_executable(http_echo_server
  http_echo_server.cpp)
