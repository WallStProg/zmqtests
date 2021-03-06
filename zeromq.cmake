

# create compilation db for use w/various tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# common flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -fno-omit-frame-pointer -fPIC -D_REENTRANT -pthread")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb -fno-omit-frame-pointer -fPIC -D_REENTRANT -pthread")

# dont include any rpath at all
set(CMAKE_SKIP_RPATH TRUE)

message("CMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}")
message("CMAKE_CXX_COMPILER_ID=${CMAKE_CXX_COMPILER_ID}")

# need ZMQ_ROOT, either on cmd line or in environment
if (NOT DEFINED $ZMQ_ROOT)
   set(ZMQ_ROOT $ENV{ZMQ_ROOT})
   if ($ZMQ_ROOT STREQUAL "")
      message(FATAL_ERROR "Please set ZMQ_ROOT")
   endif()
endif()
message("ZMQ_ROOT set to ${ZMQ_ROOT}")

set(ZMQ_INCDIR "${ZMQ_ROOT}/include")
link_directories(BEFORE ${ZMQ_ROOT}/lib64 ${ZMQ_ROOT}/lib)
link_libraries(zmq)



#########################################################################

