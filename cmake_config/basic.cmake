add_definitions(
  -g -O0 -Wall -std=c++0x 
  -Wno-deprecated-declarations
  -DMINOTAUR_MEM_CHECK=1
  -DMINOTAUR_CORO_THREAD_CHECK=1
  -DCORO_ASM=1
)

