set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_MULTITHREADED      ON)
FIND_PACKAGE( Boost COMPONENTS regex program_options thread unit_test_framework system REQUIRED )

if (NOT Boost_FOUND)
    MESSAGE(FATAL "Not found boost library!")
endif()

message(STATUS "boost include:" ${Boost_INCLUDE_DIRS})
message(STATUS "boost library:" ${Boost_LIBRARIES})
