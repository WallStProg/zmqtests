cmake_minimum_required (VERSION 2.8)

project (zmqtests)

if ("$ENV{ZMQ_ROOT}" STREQUAL "")
   message(FATAL_ERROR "Please set ZMQ_ROOT")
endif()

include(zeromq.cmake)

add_subdirectory(threads)
