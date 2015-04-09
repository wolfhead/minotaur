
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

add_executable(rapid_echo_server
  rapid_echo_server.cpp)

add_custom_target(sample_copy_config
  COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/*.properties ${CMAKE_CURRENT_BINARY_DIR}/
  COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/*.xml ${CMAKE_CURRENT_BINARY_DIR}/
  )

add_dependencies(http_echo_server sample_copy_config)
