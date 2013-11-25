find_path(MINOTAUR_INCLUDE_DIRS local.hpp ${CMAKE_SOURCE_DIR}/source NO_DEFAULT_PATH)
set(MINOTAUR_LIBRARIES minotaur)

message(STATUS "minotaur include: " ${MINOTAUR_INCLUDE_DIRS})
message(STATUS "minotaur libraries: " ${MINOTAUR_LIBRARIES})