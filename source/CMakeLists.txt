
set(MINOTAUR_SOURCE
  event/event_loop.cpp
  event/event_loop_data.cpp
  event/impl/event_loop_epoll.cpp
)

add_library(minotaur
  ${MINOTAUR_SOURCE}
)