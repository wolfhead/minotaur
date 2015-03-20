
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
#${TCMALLOC_LIBRARIES}
#${LIBUNWIND_LIBRARIES}
  pthread
  rt
  -static-libstdc++ 
  -static-libgcc
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

add_executable(t_coroutine
  t_coroutine.cpp)

add_executable(t_timing_wheel
  t_timing_wheel.cpp)

add_executable(t_event_timer
  t_event_timer.cpp)

add_executable(t_timer
  t_timer.cpp)

add_executable(t_double_linked_list
  t_double_linked_list.cpp)

add_executable(t_io_descriptor_factory
  t_io_descriptor_factory.cpp)

#add_executable(t_lf_static_list_pool
#  t_lf_static_list_pool.cpp)

#add_executable(t_timer_slot
#  t_timer_slot.cpp)

#add_executable(t_timer_wheel
#  t_timer_wheel.cpp)
