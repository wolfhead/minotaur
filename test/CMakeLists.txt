
include_directories(
  ../source/minotaur
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
  z
  pthread
  rt
)


add_executable(t_tagged_ptr  
  t_tagged_ptr.cpp)
add_test(t_tagged_ptr t_tagged_ptr)


add_executable(t_freelist
  t_freelist.cpp)
add_test(t_freelist
  t_freelist)

#add_executable(t_local
#  t_local.cpp)

#add_executable(t_lock_free_ring
#  t_lock_free_ring.cpp)

add_executable(t_sequencer
  t_sequencer.cpp)
add_test(t_sequencer
  t_sequencer)

add_executable(t_event_loop
  t_event_loop.cpp)

add_executable(t_io_buffer
  t_io_buffer.cpp)
add_test(t_io_buffer
  t_io_buffer)

add_executable(t_message_queue
  t_message_queue.cpp)
add_test(t_message_queue
  t_message_queue)

#add_executable(t_stage
#  t_stage.cpp)

add_executable(t_coroutine
  t_coroutine.cpp)
add_test(t_coroutine
  t_coroutine)

add_executable(t_timing_wheel
  t_timing_wheel.cpp)
add_test(t_timing_wheel
  t_timing_wheel)

add_executable(t_event_timer
  t_event_timer.cpp)
add_test(t_event_timer
  t_event_timer)

add_executable(t_timer
  t_timer.cpp)
add_test(t_timer
  t_timer)

add_executable(t_double_linked_list
  t_double_linked_list.cpp)
add_test(t_double_linked_list
  t_double_linked_list)

add_executable(t_linked_list
  t_linked_list.cpp)
add_test(t_linked_list
  t_linked_list)

add_executable(t_io_descriptor_factory
  t_io_descriptor_factory.cpp)
add_test(t_io_descriptor_factory
  t_io_descriptor_factory)

add_executable(t_http_protocol
  t_http_protocol.cpp)
add_test(t_http_protocol
  t_http_protocol)

add_executable(t_redis_parser
  t_redis_parser.cpp)
add_test(t_redis_parser
  t_redis_parser)

add_executable(t_matrix
  t_matrix.cpp)
add_test(t_matrix
  t_matrix)

add_executable(t_matrix_stat
  t_matrix_stat.cpp)
add_test(t_matrix_stat
  t_matrix_stat)

add_executable(t_string_util
  t_string_util.cpp)
add_test(t_string_util
  t_string_util)

#add_executable(t_lf_static_list_pool
#  t_lf_static_list_pool.cpp)

#add_executable(t_timer_slot
#  t_timer_slot.cpp)

#add_executable(t_timer_wheel
#  t_timer_wheel.cpp)
