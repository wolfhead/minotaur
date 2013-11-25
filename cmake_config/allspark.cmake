# Added by Rob, read and set ALLSPARK_INCLUDE_DIRS, ALLSPARK_LIBRARIES

if(WIN32)
  EXECUTE_PROCESS(COMMAND type $ENV{HOMEPATH}/.path_allspark OUTPUT_VARIABLE path_allspark)
  STRING(REPLACE "\r\n" "" path_allspark "${path_allspark}")
else(WIN32)
  EXECUTE_PROCESS(COMMAND cat $ENV{HOME}/.path_allspark OUTPUT_VARIABLE path_allspark)
  STRING(REPLACE "\n" "" path_allspark "${path_allspark}")
endif(WIN32)

set(ALLSPARK_CONF_INCLUDE_DIRS ${path_allspark}/include )
set(ALLSPARK_CONF_LIBRARY_DIRS ${path_allspark}/lib )

find_path(ALLSPARK_INCLUDE_DIRS
  NAMES allspark/allocator.h
  PATHS ${ALLSPARK_CONF_INCLUDE_DIRS})

find_library(ALLSPARK_LIBRARIES
  NAMES allspark
  PATHS ${ALLSPARK_CONF_LIBRARY_DIRS})

message(STATUS "Allspark include: " ${ALLSPARK_INCLUDE_DIRS})
message(STATUS "Allspark library: " ${ALLSPARK_LIBRARIES})

