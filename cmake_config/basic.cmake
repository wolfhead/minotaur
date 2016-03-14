add_definitions(
  -g -O2 -Wall -std=c++0x 
  -Wno-deprecated-declarations
  # dump memory footprint on exit for Freelist & IOBuffer
  #-DMINOTAUR_MEM_CHECK=1  
  
  # coro configuration
  -DCORO_ASM=1
)

