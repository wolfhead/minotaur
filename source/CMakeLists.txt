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
  event/event_loop.cpp
  event/event_loop_data.cpp
  event/event_loop_notifier.cpp
  event/event_loop_thread.cpp
  event/event_loop_stage.cpp
  net/io_descriptor.cpp
  net/socket.cpp
  net/acceptor.cpp
  net/channel.cpp
  net/io_buffer.cpp
  net/io_handler.cpp
  #event/timer/timer_wheel.cpp
  matrix/matrix_item_map.cpp
  io_service.cpp
)

add_library(minotaur
  ${MINOTAUR_SOURCE}
)
