cmake_minimum_required (VERSION 2.8)

project (3186)

include(./zeromq.cmake)

add_executable(pong pong.cpp common.c)
add_executable(ping ping.cpp common.c)

