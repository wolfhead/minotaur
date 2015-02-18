
include_directories(
  ../source
  ${Boost_INCLUDE_DIRS}
)

add_definitions(
    -Wno-deprecated-declarations
    )

link_libraries(
  ${MINOTAUR_LIBRARIES}
  ${Boost_LIBRARIES}
  ${LOG4CPLUS_LIBRARIES}
  pthread
  rt
)


add_executable(t_tagged_ptr  t_tagged_ptr.cpp)

add_executable(t_freelist
  t_freelist.cpp)

#add_executable(t_local
#  t_local.cpp)

#add_executable(t_lock_free_ring
#  t_lock_free_ring.cpp)

add_executable(t_sequencer
  t_sequencer.cpp)

add_executable(t_event_loop
  t_event_loop.cpp)

add_executable(t_io_buffer
  t_io_buffer.cpp)

add_executable(t_message_queue
  t_message_queue.cpp)

add_executable(t_stage
  t_stage.cpp)

#add_executable(t_lf_static_list_pool
#  t_lf_static_list_pool.cpp)

#add_executable(t_timer_slot
#  t_timer_slot.cpp)

#add_executable(t_timer_wheel
#  t_timer_wheel.cpp)
