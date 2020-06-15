cmake_minimum_required (VERSION 2.8)

project (connect)

include(../zeromq.cmake)

add_executable(connect connect.cpp monitor.cpp common.c)
target_link_libraries(connect uuid pthread)

add_executable(dummy dummy.c)
