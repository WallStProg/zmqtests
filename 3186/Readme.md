## Overview
This repo illustrates the issue from  <https://github.com/zeromq/libzmq/issues/3186>.

The `peer` program simulates the discovery protocol that I've implemented for my application.  This uses two different "channels":

- Each process connects to a `zmq_proxy` instance at a well-known address.  This connection is used to exchange messages among all the processes on the network.  These "discovery" messages communicate the endpoint address that other processes should connect to.
- Each process also creates a direct connection (i.e., no proxy) to every other process, using the endpoint addresses carried in the discovery messages.

When a process starts:

 - It binds a PUB socket and records the endpoint address; 
 - It connects a SUB socket to a zmq_proxy's PUB socket at a well-known address;
- When the process receives the "welcome" message from the proxy (see <https://somdoron.com/2015/09/reliable-pubsub/>), it connects a PUB socket to the zmq_proxy's SUB socket.
- The process then begins sending "connect" messages with its own PUB endpoint for other processes to connect to.
 - The discovery message is sent until it is received by the process itself, and the process connects its own SUB socket to its own PUB socket.
 - A discovery message is also sent any time a process sees a new peer on the network.

Similarly, when a process exits it first sends a "disconnect" message on the proxy channel -- it does not exit until it receives its own disconnect message.

## The Problem
When many peers connect and disconnect, the memory used by the process (as measured by `top`) grows continually.

For example, running for approximately one hour with four processes continually connecting and disconnecting the VIRT/RES values for the long-lived peer process increase from 107m/2620 to 171m/73m.

However, this memory growth does not appear to be a leak per-se, since running under valgrind shows no leaks on normal shutdown.

## Building
You will need to set `ZMQ_ROOT` before building.  The `setenv.sh` script can be modified to set `ZMQ_ROOT` for your environment.

```
$  source ../setenv.sh
$  cmake . && make clean && make
CMAKE_CXX_COMPILER=/usr/bin/c++
CMAKE_CXX_COMPILER_ID=GNU
-- Configuring done
-- Generating done
-- Build files have been written to: /home/btorpey/work/zmqtests/3186
[ 50%] Building CXX object CMakeFiles/peer.dir/peer.cpp.o
Linking CXX executable peer
[ 50%] Built target peer
[100%] Building C object CMakeFiles/proxy.dir/proxy.c.o
Linking C executable proxy
[100%] Built target proxy
$  
```

Or, you can supply `ZMQ_ROOT` on the cmake command line:

```
$  cmake -DZMQ_ROOT=$HOME/install/libzmq/4.2.3/dev . && make clean && make
CMAKE_CXX_COMPILER=/usr/bin/c++
CMAKE_CXX_COMPILER_ID=GNU
-- Configuring done
-- Generating done
-- Build files have been written to: /home/btorpey/work/zmqtests/3186
[ 50%] Building CXX object CMakeFiles/peer.dir/peer.cpp.o
Linking CXX executable peer
[ 50%] Built target peer
[100%] Building C object CMakeFiles/proxy.dir/proxy.c.o
Linking C executable proxy
[100%] Built target proxy
$  
```

## Running
You can use the provided `runme.sh` script to start up a typical session.  This will run:

- a single proxy process (at port 5555);
- a single long-lived peer process;
- several short-lived peer processes that repeatedly connect, run for two seconds, then disconnect;
- finally, a top process that monitors the long-lived peer process.

You should be able to see the VIRT and RES columns for the peer process grow continually during the course of the test run.

```
top - 12:59:06 up  4:32,  4 users,  load average: 0.00, 0.02, 0.00
Tasks:   1 total,   0 running,   1 sleeping,   0 stopped,   0 zombie
Cpu(s):  0.8%us,  0.4%sy,  0.0%ni, 98.7%id,  0.0%wa,  0.0%hi,  0.2%si,  0.0%st
Mem:  16219820k total,  3067272k used, 13152548k free,   612024k buffers
Swap:  8388604k total,        0k used,  8388604k free,  1515004k cached

  PID USER      PR  NI  VIRT  RES  SHR S %CPU %MEM    TIME+  COMMAND                                                                
25788 btorpey   20   0  107m  24m 2172 S  0.3  0.2   0:00.69 ./peer                                                                 
```

To stop the test, enter Ctrl+C in the terminal session, then:

    killall -9 repeat.sh proxy peer

## Results
When run with valgrind's massif tool, the program produces output similar to the following:

```
$ valgrind --tool=massif --xtree-memory=full --xtree-memory-file=callgrind.%p ./peer -s 300
$ callgrind_annotate --include=. callgrind.2951 | tee callgrind.2951.txt
--------------------------------------------------------------------------------
Profile data file 'callgrind.2951' (creator: xtree-1)
--------------------------------------------------------------------------------
Profiled target:  ./peer -s 300 (PID 2951)
Events recorded:  curB curBk totB totBk totFdB totFdBk
Events shown:     curB curBk totB totBk totFdB totFdBk
Event sort order: curB curBk totB totBk totFdB totFdBk
Thresholds:       99 0 0 0 0 0
Include dirs:     
User annotated:   
Auto-annotation:  off

--------------------------------------------------------------------------------
curB curBk       totB  totBk     totFdB totFdBk 
--------------------------------------------------------------------------------
   0     0 67,312,375 81,052 67,312,375  81,052  PROGRAM TOTALS

--------------------------------------------------------------------------------
curB curBk       totB  totBk     totFdB totFdBk  file:function
--------------------------------------------------------------------------------
   0     0 51,553,473 28,420 43,913,372  30,468  /home/btorpey/work/libzmq/4.2.3/src/thread.cpp:thread_routine
   0     0 51,553,473 28,420 43,913,372  30,468  /home/btorpey/work/libzmq/4.2.3/src/epoll.cpp:zmq::epoll_t::worker_routine(void*)
   0     0 51,553,473 28,420 43,913,372  30,468  /usr/src/debug/glibc-2.12-2-gc4ccff1/nptl/pthread_create.c:start_thread
   0     0 51,553,473 28,420 43,913,372  30,468  /usr/src/debug////////glibc-2.12-2-gc4ccff1/misc/../sysdeps/unix/sysv/linux/x86_64/clone.S:clone
   0     0 51,553,473 28,420 43,913,372  30,468  /home/btorpey/work/libzmq/4.2.3/src/epoll.cpp:zmq::epoll_t::loop()
   0     0 36,294,349 15,228 19,898,952  12,108  /home/btorpey/work/libzmq/4.2.3/src/stream_engine.cpp:zmq::stream_engine_t::in_event()
   0     0 27,306,000  1,665          0       0  /home/btorpey/work/libzmq/4.2.3/src/yqueue.hpp:zmq::yqueue_t<zmq::msg_t, 256>::allocate_chunk()
   0     0 27,273,200  1,663          0       0  /home/btorpey/work/libzmq/4.2.3/src/ypipe.hpp:zmq::ypipe_t<zmq::msg_t, 256>::ypipe_t()
   0     0 27,273,200  1,663          0       0  /home/btorpey/work/libzmq/4.2.3/src/yqueue.hpp:zmq::yqueue_t<zmq::msg_t, 256>::yqueue_t()
   0     0 25,309,464  1,394          0       0  /home/btorpey/work/libzmq/4.2.3/src/decoder_allocators.cpp:zmq::shared_message_memory_allocator::allocate()
   0     0 24,898,496  4,476          0       0  /home/btorpey/work/libzmq/4.2.3/src/pipe.cpp:zmq::pipepair(zmq::object_t**, zmq::pipe_t**, int*, bool*)
   0     0 20,532,904  3,730     41,776     746  /home/btorpey/work/libzmq/4.2.3/src/stream_engine.cpp:zmq::stream_engine_t::handshake()
   0     0 15,758,902 52,632 23,397,819  50,582  /home/btorpey/work/zmqtests/3186/peer.cpp:main
   0     0 13,544,376    746          0       0  /home/btorpey/work/libzmq/4.2.3/src/decoder.hpp:zmq::decoder_base_t<zmq::v2_decoder_t, zmq::shared_message_memory_allocator>::decoder_base_t(zmq::shared_message_memory_allocator*)
   0     0 13,544,376    746          0       0  /home/btorpey/work/libzmq/4.2.3/src/v2_decoder.cpp:zmq::v2_decoder_t::v2_decoder_t(unsigned long, long)
...
```

I've included the output files here -- qcachegrind/kcachegrind can open the callgrind.2951 file.