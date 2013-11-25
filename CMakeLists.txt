cmake_minimum_required(VERSION 2.6)

enable_testing()

project(minotaur)

set(CMAKE_CONFIG_DIR ${CMAKE_SOURCE_DIR}/cmake_config)

include(${CMAKE_CONFIG_DIR}/basic.cmake)
include(${CMAKE_CONFIG_DIR}/boost.cmake)
include(${CMAKE_CONFIG_DIR}/allspark.cmake)
include(${CMAKE_CONFIG_DIR}/minotaur.cmake)

add_subdirectory(source)
add_subdirectory(test)