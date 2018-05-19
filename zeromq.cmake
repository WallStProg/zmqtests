# create compilation db for use w/various tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# common flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -fno-omit-frame-pointer -fPIC -D_REENTRANT ")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb -fno-omit-frame-pointer -fPIC -D_REENTRANT ")

# dont include any rpath at all
set(CMAKE_SKIP_RPATH TRUE)

message("CMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}")
message("CMAKE_CXX_COMPILER_ID=${CMAKE_CXX_COMPILER_ID}")

# enable warnings
set(WARNFLAGS "${WARNFLAGS} -Wall")
set(WARNFLAGS "${WARNFLAGS} -Wextra")
set(WARNFLAGS "${WARNFLAGS} -Wformat")
set(WARNFLAGS "${WARNFLAGS} -Wformat-nonliteral")                # warn about non-literal format strings in printf etc.
#set(WARNFLAGS "${WARNFLAGS} -Wexit-time-destructors")
# disable warnings
set(WARNFLAGS "${WARNFLAGS} -Wno-reorder")                       # order of initialization in ctor
set(WARNFLAGS "${WARNFLAGS} -Wno-unused-parameter")              # given that API is defined in interface, this is kind of hard to enforce
set(WARNFLAGS "${WARNFLAGS} -Wno-ignored-qualifiers")            # e.g., const on value return types

# set ZMQ_ROOT in environment
set(ZMQ_ROOT $ENV{ZMQ_ROOT})
set(ZMQ_INCDIR "${ZMQ_ROOT}/include")
set(ZMQ_LIBDIR "${ZMQ_ROOT}/lib64")
set(ZMQ_CFLAGS "-I ${ZMQ_INCDIR} -I ../")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ZMQ_CFLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ZMQ_CFLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${ZMQ_LDFLAGS} ${ZMQ_LDLIBS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}  -Wl,-rpath,${ZMQ_LIBDIR}")
link_directories(${ZMQ_LIBDIR})
link_libraries(zmq)



#########################################################################

