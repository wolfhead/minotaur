INCLUDE (CheckSymbolExists)

include_directories(
  ${Boost_INCLUDE_DIRS}
  ${LOG4CPLUS_INCLUDE_DIRS}
)

CHECK_SYMBOL_EXISTS (epoll_wait "sys/epoll.h" HAS_EPOLL)
CHECK_SYMBOL_EXISTS (kqueue "sys/event.h" HAS_KQUEUE)

add_custom_target(version_builder
  COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/version_builder.sh 
    ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/git_version.h
)

# generate config.h
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/config.h)

IF(${HAS_EPOLL})
  SET(MINOTAUR_EVENT_SOURCE event/impl/event_loop_epoll.cpp)
ELSE()
  IF(${HAS_KQUEUE})
    SET(MINOTAUR_EVENT_SOURCE event/impl/event_loop_kqueue.cpp)
  ENDIF()
ENDIF()

set(MINOTAUR_SOURCE
  ${CMAKE_CURRENT_SOURCE_DIR}/config.h
  ${MINOTAUR_EVENT_SOURCE}
  message.cpp
  handler.cpp
  io_service.cpp
  event/event_loop.cpp
  event/event_loop_data.cpp
  event/event_loop_notifier.cpp
  event/event_loop_thread.cpp
  event/event_loop_stage.cpp
  event/timer_thread.cpp
  net/io_message.cpp
  net/io_redis_message.cpp
  net/io_descriptor_factory.cpp
  net/io_descriptor.cpp
  net/io_buffer.cpp
  net/io_handler.cpp
  net/socket.cpp
  net/acceptor.cpp
  net/channel.cpp
  net/service_channel.cpp
  net/client_channel.cpp
  net/sync_client_channel.cpp
  net/async_client_channel.cpp
  net/stdio_channel.cpp
  net/async_sequence_keeper.cpp
  net/sync_sequence_keeper.cpp
  net/protocol/protocol_factory.cpp
  net/protocol/line/line_protocol.cpp
  net/protocol/rapid/rapid_protocol.cpp
  net/protocol/http/http_protocol.cpp
  net/protocol/redis/redis_parser.cpp
  net/protocol/redis/redis_protocol.cpp
  net/protocol/memcached/memcached_protocol.cpp

  coroutine/coroutine.cpp
  coroutine/coro_actor.cpp
  coroutine/coro_service.cpp
  coroutine/coro_factory.cpp

  service/service_handler.cpp
  service/coroutine_service_handler.cpp
  service/service_timer_thread.cpp
  service/service.cpp

  client/client.cpp
  client/client_router.cpp
  client/client_manager.cpp
  client/redis_client.cpp

  application/application.cpp
  application/config_manager.cpp

  common/slice.cpp
  common/string_util.cpp

  online_debug/online_debug.cpp
  online_debug/online_debug_logger_appender.cpp

  3rd-party/libcoro/coro.c
  3rd-party/tinyxml2/tinyxml2.cpp
  3rd-party/http_parser/http_parser.c
  3rd-party/md5/md5.c
  3rd-party/urlsafe64/urlsafe64.c
  3rd-party/crc/crc.cpp

  matrix/matrix.cpp
  matrix/matrix_item.cpp
  matrix/matrix_item_map.cpp
  matrix/matrix_collector.cpp
  matrix/matrix_stat.cpp
  matrix/matrix_stat_map.cpp
)

add_library(minotaur
  ${MINOTAUR_SOURCE}
)

add_dependencies(minotaur version_builder)
