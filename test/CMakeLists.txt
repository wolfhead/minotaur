
include_directories(
  ../source
  ${Boost_INCLUDE_DIRS}
)

link_libraries(
  ${Boost_LIBRARIES}
  ${MINOTAUR_LIBRARIES}
  pthread
)

add_executable(t_local
  t_local.cpp)

add_executable(t_event_loop
  t_event_loop.cpp)

add_executable(t_lf_static_list_pool
  t_lf_static_list_pool.cpp)

add_executable(t_timer_slot
  t_timer_slot.cpp)

add_executable(t_timer_wheel
  t_timer_wheel.cpp)
