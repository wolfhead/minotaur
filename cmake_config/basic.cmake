add_definitions(
  -g -O3 -Wall -std=c++0x 
  -Wno-deprecated-declarations
  # dump memory footprint on exit for Freelist & IOBuffer
  -DMINOTAUR_MEM_CHECK=1  
  # check if the coro runs in the created thread
  -DMINOTAUR_CORO_THREAD_CHECK=1
  
  # coro configuration
  -DCORO_ASM=1
  -DCORO_USE_VALGRIND=1
)

