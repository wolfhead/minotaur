INCLUDE (CheckSymbolExists)

CHECK_SYMBOL_EXISTS (epoll_wait "sys/epoll.h" HAS_EPOLL)
CHECK_SYMBOL_EXISTS (kqueue "sys/event.h" HAS_kQUEUE)

# generate config.h
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/config.h)

IF(${HAS_EPOLL})
  SET(MINOTAUR_EVENT_SOURCE event/impl/event_loop_epoll.cpp)
ELSE()
  IF(${HAS_kQUEUE})
    SET(MINOTAUR_EVENT_SOURCE event/impl/event_loop_kqueue.cpp)
  ENDIF()
ENDIF()


set(MINOTAUR_SOURCE
  ${CMAKE_CURRENT_SOURCE_DIR}/config.h
  event/event_loop.cpp
  event/event_loop_data.cpp
  ${MINOTAUR_EVENT_SOURCE}
)

add_library(minotaur
  ${MINOTAUR_SOURCE}
)