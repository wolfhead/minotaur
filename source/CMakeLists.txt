INCLUDE (CheckSymbolExists)

include_directories(
  ${LOG4CPLUS_INCLUDE_DIRS}
)

CHECK_SYMBOL_EXISTS (epoll_wait "sys/epoll.h" HAS_EPOLL)
CHECK_SYMBOL_EXISTS (kqueue "sys/event.h" HAS_KQUEUE)

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
  net/io_descriptor_factory.cpp
  net/io_descriptor.cpp
  net/io_buffer.cpp
  net/io_handler.cpp
  net/socket.cpp
  net/acceptor.cpp
  net/channel.cpp
  net/service_channel.cpp
  net/client_channel.cpp
  net/async_sequence_keeper.cpp
  net/protocol/protocol_factory.cpp
  net/protocol/line/line_protocol.cpp
  net/protocol/rapid/rapid_protocol.cpp
  net/protocol/http/http_parser.c
  net/protocol/http/http_protocol.cpp
  coroutine/libcoro/coro.c
  coroutine/libcoro/coro.h
  coroutine/coroutine.cpp
  coroutine/coro_actor.cpp
  coroutine/coro_service.cpp
  coroutine/coro_factory.cpp
  service/service_handler.cpp
  service/coroutine_service_handler.cpp
  service/service.cpp

  #event/timer/timer_wheel.cpp
# matrix/matrix_item_map.cpp
)

add_library(minotaur
  ${MINOTAUR_SOURCE}
)
