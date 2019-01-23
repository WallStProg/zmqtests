# create compilation db for use w/various tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# common flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -fno-omit-frame-pointer -fPIC -D_REENTRANT ")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb -fno-omit-frame-pointer -fPIC -D_REENTRANT ")

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
if (APPLE)
   set(ZMQ_LIBDIR "${ZMQ_ROOT}/lib")
else()
   set(ZMQ_LIBDIR "${ZMQ_ROOT}/lib64")
endif()
set(ZMQ_CFLAGS "-I ${ZMQ_INCDIR} -I ../")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ZMQ_CFLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ZMQ_CFLAGS}")
link_directories(${ZMQ_LIBDIR})
link_libraries(zmq)


if("$ENV{BUILD_TYPE}" STREQUAL "tsan")
  #
  # NOTE: TSAN generates many false(?) positives w/ZMQ, esp. if ZMQ is built w/o "-DZMQ_HAVE_ATOMIC_INTRINSICS=1"
  # These flag settings have been arrived at w/help of https://github.com/dvyukov along with trial and error to
  # enable ZMQ to "play nice" w/other code instrumented w/TSAN.
  #
  # Note also that a runtime suppression "called_from_lib:libzmq.so" is also required to prevent spurious reports when
  # ZMQ calls intercepted functions (e.g., memcpy).
  #
  # See https://github.com/google/sanitizers/issues/941 for more info.
  #
  # P.S.  These settings may cause warnings like following -- this is a known issue w/cmake passing compile-specific flags to linker
  # when building shared libs.
  # clang-7: warning: argument unused during compilation: '-mllvm -tsan-instrument-memory-accesses=false' [-Wunused-command-line-argument]
  #
  # standard flags
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -fPIE")
  # following settings disable TSAN instrumentation (*except* for func entry/exit)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mllvm -tsan-instrument-memory-accesses=false ")
  # not strictly needed if building w/"-DZMQ_HAVE_ATOMIC_INTRINSICS=1" (see above)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mllvm -tsan-instrument-atomics=false")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mllvm -tsan-handle-cxx-exceptions=false")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mllvm -tsan-instrument-memintrinsics=false")
  # this is needed for proper stack traces w/TSAN
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mllvm -tsan-instrument-func-entry-exit=true")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=thread  ")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=thread -pie ")
endif()

# use address sanitizer? (same params for gcc/clang)
if("$ENV{BUILD_TYPE}" STREQUAL "asan")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fsanitize-address-use-after-scope")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize-address-use-after-scope")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=address -fsanitize-address-use-after-scope")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address -fsanitize-address-use-after-scope")
endif()


#########################################################################
