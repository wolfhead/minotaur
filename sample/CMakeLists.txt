
include_directories(
  ../source/minotaur
  ${Boost_INCLUDE_DIRS})

link_libraries(
  ${MINOTAUR_LIBRARIES}
  ${Boost_LIBRARIES}
  ${LOG4CPLUS_LIBRARIES}
#  ${TCMALLOC_LIBRARIES}
#  ${LIBUNWIND_LIBRARIES}
  z
  pthread
  rt
)

add_executable(http_echo_server
  http_echo_server.cpp)

add_executable(echo_server
  echo_server.cpp)

add_executable(http_rapid_server
  http_rapid_server.cpp)

add_executable(http_redis_server
  http_redis_server.cpp)

add_executable(http_memcache_server
  http_memcache_server.cpp)

add_custom_target(sample_copy_config
  COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/script
  COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/*.properties ${CMAKE_CURRENT_BINARY_DIR}/
  COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/*.xml ${CMAKE_CURRENT_BINARY_DIR}/
  COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/script/*.py ${CMAKE_CURRENT_BINARY_DIR}/script/
  )

add_dependencies(http_echo_server sample_copy_config)
