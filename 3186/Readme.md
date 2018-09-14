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
- Similarly, when a process wants to exit it first sends a "disconnect" message on the proxy channel -- it does not actually exit until it receives its own disconnect message.

## Building
You will need to set `ZMQ_ROOT` before building.  The `setenv.sh` script can be modified to set `ZMQ_ROOT` for your environment.

```
$  source ./setenv.sh
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

## The Original Problem
When many peers connect and disconnect, the memory used by the process (as measured by `top`) grows continuously.

For example, running for approximately one hour with four processes continually connecting and disconnecting the VIRT/RES values for the long-lived peer process increase from 107m/2620 to 171m/73m.

However, this memory growth does not appear to be a leak per-se, since running under valgrind shows no leaks on normal shutdown.

### Reproducing the original problem
You can use the provided `runme.sh` script to start up a typical session.  This will run:

- a single proxy process (at port 5555);
- a single long-lived peer process;
- several short-lived peer processes that repeatedly connect, run for two seconds, then disconnect;
- finally, a top process that monitors the long-lived peer process.

To reproduce the problem:

```
./runme.sh -seconds 2
```

This will start a proxy, a long-lived peer, and a number of additional peer processes that will run for two seconds, disconnect, and restart.

You should be able to see the VIRT and RES columns for the long-lived peer process grow continually during the course of the test run.

```
top - 12:59:06 up  4:32,  4 users,  load average: 0.00, 0.02, 0.00
Tasks:   1 total,   0 running,   1 sleeping,   0 stopped,   0 zombie
Cpu(s):  0.8%us,  0.4%sy,  0.0%ni, 98.7%id,  0.0%wa,  0.0%hi,  0.2%si,  0.0%st
Mem:  16219820k total,  3067272k used, 13152548k free,   612024k buffers
Swap:  8388604k total,        0k used,  8388604k free,  1515004k cached

  PID USER      PR  NI  VIRT  RES  SHR S %CPU %MEM    TIME+  COMMAND                                                                
25788 btorpey   20   0  107m  24m 2172 S  0.3  0.2   0:00.69 ./peer                                                                 
```

### Stopping the test

To stop the test, enter Ctrl+C in the terminal session, then:

    killall -9 repeat.sh proxy peer

If running under valgrind, it can be pretty tricky finding valgrind in list of processes -- see this (<https://github.com/acg/psmisc/issues/1>) for more.  So, if running w/valgrind you may need to do something like this:

    killall peer proxy repeat.sh memcheck-amd64-linux
    
    
### Results
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

### Solution 
The root cause of the memory usage observed above is that the dataPub socket never gets a chance to run `process_commands`, which is the code that triggers clean-up of unused resources.

The solution is to call either `zmq_poll` on the dataPub socket after a disconnect -- this triggers `process_commands` on the dataPub socket, and allows it to clean up unused resources.

This Github issue (<https://github.com/zeromq/libzmq/issues/1256>) implies that calling `zmq_getsockopt(..., ZMQ_EVENTS...) also works, but in testing that does not appear to be the case.

## Additional Problems
However, there are two other problems that we ran into while investigating the memory issue, and we want to understand what causes them and how to either fix or avoid them:

1. One potential workaround that we tried in order to trigger process_commands was to send a dummy message on the dataPub socket after disconnecting the dataSub socket.  While this does trigger process_commands on the dataPub socket, there is apparently a race condition in libzmq where sending the dummy message causes the `zmq_ctx_term` call to intermittently hang forever. 
2. The hang only happens when we set linger to zero on the socket immediately before closing it -- if we set the linger when we create the socket then the `zmq_ctx_term` call never hangs

This behavior seems to indicate that:

1. There is a race between calling `zmq_send` and `zmq_close` on the same socket under certain conditions.  We need to understand the root cause of this race condition, and ideally fix it.  If we can't fix it, we need to understand precisely the conditions under which it occurs so we can reliably avoid it.
2. There is also a race between setting the linger on a socket, and that setting actually taking effect.  There is a Github issue (<https://github.com/zeromq/libzmq/pull/2910>) that describes a race caused by not using atomic operations to update the linger value, but that is NOT the problem we see.  As with the original problem, this race appears to be between process_commands on the application thread and the io thread.

To reproduce the hang problem:

```
./runme.sh -send -seconds 2
```

You can monitor the progress of the short-lived peer processes by tailing the log files:

```
$ tail -F peer*.out | grep peer
==> peer1.out <==
==> peer3.out <==
==> peer1.out <==
==> peer3.out <==
==> peer1.out <==
==> peer3.out <==
==> peer1.out <==
==> peer3.out <==
==> peer1.out <==
==> peer4.out <==
==> peer1.out <==
==> peer4.out <==
...
```

When you notice that a particular peer has stopped running, you can get a stack trace as follows:

```
$ grep Process peer2.out
...
11:35:48.416224	Process id=7793
$ pstack 7793
Thread 3 (Thread 0x7fec41560700 (LWP 8314)):
#0  0x00000033bfee91c3 in epoll_wait () from /lib64/libc.so.6
#1  0x00007fec4182c131 in zmq::epoll_t::loop() () from /home/btorpey/install/libzmq/4.2.3/dev/lib64/libzmq.so.5.1.3
#2  0x00007fec4182c404 in zmq::epoll_t::worker_routine(void*) () from /home/btorpey/install/libzmq/4.2.3/dev/lib64/libzmq.so.5.1.3
#3  0x00007fec418726ca in thread_routine () from /home/btorpey/install/libzmq/4.2.3/dev/lib64/libzmq.so.5.1.3
#4  0x00000033c0607aa1 in start_thread () from /lib64/libpthread.so.0
#5  0x00000033bfee8bcd in clone () from /lib64/libc.so.6
Thread 2 (Thread 0x7fec40b5f700 (LWP 8318)):
#0  0x00000033bfee91c3 in epoll_wait () from /lib64/libc.so.6
#1  0x00007fec4182c131 in zmq::epoll_t::loop() () from /home/btorpey/install/libzmq/4.2.3/dev/lib64/libzmq.so.5.1.3
#2  0x00007fec4182c404 in zmq::epoll_t::worker_routine(void*) () from /home/btorpey/install/libzmq/4.2.3/dev/lib64/libzmq.so.5.1.3
#3  0x00007fec418726ca in thread_routine () from /home/btorpey/install/libzmq/4.2.3/dev/lib64/libzmq.so.5.1.3
#4  0x00000033c0607aa1 in start_thread () from /lib64/libpthread.so.0
#5  0x00000033bfee8bcd in clone () from /lib64/libc.so.6
Thread 1 (Thread 0x7fec415627a0 (LWP 8310)):
#0  0x00000033bfedf383 in poll () from /lib64/libc.so.6
#1  0x00007fec4185926d in zmq::signaler_t::wait(int) () from /home/btorpey/install/libzmq/4.2.3/dev/lib64/libzmq.so.5.1.3
#2  0x00007fec41832e11 in zmq::mailbox_t::recv(zmq::command_t*, int) () from /home/btorpey/install/libzmq/4.2.3/dev/lib64/libzmq.so.5.1.3
#3  0x00007fec41819d7d in zmq::ctx_t::terminate() () from /home/btorpey/install/libzmq/4.2.3/dev/lib64/libzmq.so.5.1.3
#4  0x00007fec4187df4e in zmq_ctx_term () from /home/btorpey/install/libzmq/4.2.3/dev/lib64/libzmq.so.5.1.3
#5  0x00000000004057ad in main ()

```

This demonstrates that the process is stuck in `zmq_ctx_term`, and shows three threads: the application thread, the reaper thread and the IO thread.

To confirm that setting linger at socket creation time avoids the race condition, and thus avoids the hang in `zmq_ctx_term`:

```
./runme.sh -send -seconds 2 -early
```

So, to summarize, we have two problems:

- race between `zmq_send` and `zmq_close`
- race between setting linger and `zmq_close`

