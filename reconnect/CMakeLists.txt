cmake_minimum_required (VERSION 2.8)

project (3186)

include(../zeromq.cmake)

add_executable(peer peer.cpp monitor.cpp common.c)
target_link_libraries(peer uuid pthread)

add_executable(proxy proxy.cpp common.c)
target_link_libraries(proxy uuid pthread)

add_executable(connect connect.cpp monitor.cpp common.c)
target_link_libraries(connect uuid pthread)

