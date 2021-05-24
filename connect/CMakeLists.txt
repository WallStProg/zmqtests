cmake_minimum_required (VERSION 2.8)

project (connect)

include(../zeromq.cmake)

include_directories(${ZMQ_INCDIR})
add_executable(connect connect.cpp monitor.cpp common.c)
target_link_libraries(connect pthread)

add_executable(dummy dummy.cpp monitor.cpp common.c)
target_link_libraries(dummy pthread)
