cmake_minimum_required(VERSION 2.6)

enable_testing()

project(minotaur)

set(CMAKE_CONFIG_DIR ${CMAKE_SOURCE_DIR}/cmake_config)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

include(${CMAKE_CONFIG_DIR}/basic.cmake)
include(${CMAKE_CONFIG_DIR}/boost.cmake)
include(${CMAKE_CONFIG_DIR}/log4cplus.cmake)
include(${CMAKE_CONFIG_DIR}/minotaur.cmake)
include(${CMAKE_CONFIG_DIR}/tcmalloc.cmake)

add_subdirectory(source/minotaur)
add_subdirectory(test)
add_subdirectory(sample)

add_custom_target(install
    COMMAND echo ${CMAKE_CURRENT_SOURCE_DIR} > ~/.path_minotaur)

