#!/bin/bash -xv

# accept version on command line
[[ -n $1 ]] && ZMQ_VERSION=$1
# default to latest version
[[ -z ${ZMQ_VERSION} ]] && ZMQ_VERSION=4.2.5
export ZMQ_VERSION

if [[ ${OSTYPE} == *darwin* ]]; then
   # on Mac, assume zeromq was installed w/"brew install zeromq"
   # Note: also need "brew install ossp-uuid"
   export ZMQ_ROOT=/usr/local/Cellar/zeromq/${ZMQ_VERSION}
elif [[ ${OSTYPE} == *linux* ]]; then
   # change this to reflect location of ZeroMQ
   # NOTE: below assumes build from source -- have been unable to find a reliable install
   # for CentOS 6/7
   #export ZMQ_ROOT=/build/share/libzmq/${ZMQ_VERSION}/release
   export ZMQ_ROOT=$HOME/install/libzmq/${ZMQ_VERSION}/release
   #export ZMQ_ROOT=/usr
else
   echo "Unknown OS!"
   exit 1
fi

# TSAN
if [[ ${BUILD_TYPE} == "tsan" ]];then
   export ZMQ_ROOT=$HOME/install/libzmq/${ZMQ_VERSION}/tsan
   export TSAN_LOGDIR=$HOME/tsan
   export TSAN_OPTIONS="detect_deadlocks=1 report_destroy_locked=1 verbosity=9 exitcode=0 log_path=${TSAN_LOGDIR} log_exe_name=1 log_suffix=.txt history_size=7 second_deadlock_stack=1"
fi

# ASAN
if [[ ${BUILD_TYPE} == "asan" ]];then
   export ZMQ_ROOT=$HOME/install/libzmq/${ZMQ_VERSION}/asan
   export ASAN_LOGDIR=$HOME/asan
   export ASAN_OPTIONS="log_path=${ASAN_LOGDIR}/asan:abort_on_error=1:verbosity=0:debug=1:malloc_context_size=256:detect_stack_use_after_return=1:allow_user_segv_handler=1:handle_segv=0:fast_unwind_on_malloc=0:check_initialization_order=1:detect_odr_violation=1:use_odr_indicator=1:halt_on_error=1"
   export LSAN_OPTIONS=":detect_leaks=0"
fi

export LD_LIBRARY_PATH=${ZMQ_ROOT}/lib64:${LD_LIBRARY_PATH}

export PING_PUB=5755
export PONG_PUB=5855
