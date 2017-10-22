# Inter-thread signaling with ZeroMQ

In multi-threaded programs that use ZeroMQ, there are common use-cases that require the different threads to communicate:

- Since subscriptions can only be created and deleted by the thread that owns the SUB socket, there needs to be a way to subscribe and unsubscribe to filters in a thread-safe manner.
- In any code that uses `zmq_poll` to read from multiple sockets, there needs to be a way to terminate the `zmq_poll` loop in a thread-safe manner.

The example code from [The ZeroMQ Guide](http://zguide.zeromq.org/page:all#Signaling-Between-Threads-PAIR-Sockets) is, ummm, misleading at best -- several of the examples given will fail under certain conditions, by either dropping messages or deadlocking.

The sample programs here demonstrate several different approaches to inter-thread signalling with ZeroMQ, and show which work and which don't in "real-world" scenarios.

> By "real-world" in this case I mean the following:
>
> - The code supports an arbitrary number of threads;
> - The code supports arbitrary numbers of messages;
> - Messages can be sent at any time;
> - The code is likely to be implemented as a library which provides an interface between ZeroMQ and the application.  In this scenario, the library code (the code calling ZeroMQ functions) cannot make any assumptions about its context.
>
> For instance, in my own case I need to deal with library code hosted by a Java program running under the JVM -- that library code has no control over when threads are started or stopped.

## TL;DR
- PAIR sockets don't work -- they will eventually deadlock
- PUB/SUB sockets don't work -- under certain conditions, the initial "n" messages sent will be lost.
- PUSH/PULL sockets *do* work:
 - senders block until a connection is established
 - they don't exhibit the same deadlocking behavior as pair sockets
- CLIENT/SERVER sockets also work, and have the added benefit that they are thread-safe by design.
 
## Program Logic
The different programs are essentially identical, except for the parts that need to be different depending on the socket type.

The program creates a main thread that reads from both a "data" socket and a "control" socket.  This is a common pattern, especially when it is necessary to read from multiple sockets, but even when reading from a single socket.  The main thread calls `zmq_poll` on both sockets in a loop, and processes any messages received.

The program also creates a number of "control" threads, which send a specified number of "ping" messages, with embedded sequence number, to the control socket.  Each control thread terminates when it has sent the specified number of messages.

When the main thread receives a ping message, it compares the incoming sequence number with the last sequence number received for that thread, and prints an error message if the incoming number is not one greater than the previous.

The program waits for the control threads to finish, then terminates the main thread, and prints the highest sequence number sent and received for each control thread.

### Socket-specific code

For the non-thread-safe sockets (e.g, PUB/SUB, PAIR), each control thread creates its sending socket in thread-local storage, along with a clean-up function that call `zmq_close` on the socket when the thread terminates.

> Note that this approach isn't practical in all cases -- e.g., in code hosted by a Java process it's not always possible to ensure that each sending thread will terminate when desired.  In that kind of situation, it's possible to call `zmq_ctx_shutdown` rather than `zmq_ctx_destroy` to terminate ZeroMQ, even though not all sockets have been closed.  This will presumably result in memory and/or resource "leaks" at shutdown, but it's the best that can be done.

When using ZMQ\_PAIR sockets, the control thread(s) will connect and disconnect from the specified endpoint for each send.  This is necessary because ZMQ\_PAIR sockets are *exclusive* - i.e., there can only be a single ZMQ_PAIR connection active at any given time.

When run with the `-poll` switch, the sending thread(s) call `zmq_poll` following `zmq_connect`, as hinted at [here](<https://github.com/zeromq/libzmq/issues/2267>).  

- With pub/sub sockets this improves things a bit, especially with a small number of threads (e.g., 10).  It is not reliable, however, as can be seen by running with a larger number of threads (e.g., 100).
- With pub/sub sockets, `zmq_poll` is also called following `zmq_send`, as per the [above](<https://github.com/zeromq/libzmq/issues/2267>).  
- With pair sockets, calling `zmq_poll` after `zmq_send` results in the `zmq_disconnect` returning errno 2.  

## Running the programs

The program names should be self-explanatory.  ALL programs take the same command-line parameters:

Param&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;   | Meaning
------------- | -------------
-threads n | Run "n" control threads.  Default is 10.
-msgs n | Each control thread sends n ping commands. Default is 100.
-poll | Call `zmq_poll` after `zmq_connect` (and `zmq_send`, for pub/sub).
-sleep n | Sleep for n seconds after control threads finish (to allow time for messages to be received and processed).  Default is 1.
-debug | Verbose debugging information.
-quiet | No debugging information.

## Results

### PUB/SUB with 10 threads
In this example, the initial ping messages are lost. In addition, the shutdown message is lost, so the program must be terminated with Ctrl-C.

```
$ ./pubsub -threads 10 -msgs 1000 -quiet
Using libzmq version 4.2.3
Using pub/sub sockets
Sleeping for 1 seconds at shutdown
Thread 2383150 received # 17, expected # 1
Thread 2383158 received # 19, expected # 1
Thread 2383160 received # 19, expected # 1
Thread 2383168 received # 19, expected # 1
Thread 2383180 received # 18, expected # 1
Thread 2383178 received # 19, expected # 1
Thread 2383188 received # 18, expected # 1
Thread 2383190 received # 18, expected # 1
Thread 2383198 received # 19, expected # 1
Thread 2383170 received # 19, expected # 1
Waiting...^C
Thread 2383150 sent # 1000, received # 1000
Thread 2383158 sent # 1000, received # 1000
Thread 2383160 sent # 1000, received # 1000
Thread 2383168 sent # 1000, received # 1000
Thread 2383170 sent # 1000, received # 1000
Thread 2383178 sent # 1000, received # 1000
Thread 2383180 sent # 1000, received # 1000
Thread 2383188 sent # 1000, received # 1000
Thread 2383190 sent # 1000, received # 1000
Thread 2383198 sent # 1000, received # 1000
$
```

### PUB/SUB with 10 threads and poll
Polling after the connect and send calls *seems* to work with a relatively small number of threads, but read on...

```
$ ./pubsub -threads 10 -msgs 1000 -quiet -poll
Using libzmq version 4.2.3
Using pub/sub sockets
Sleeping for 1 seconds at shutdown
Polling after connect
Waiting...
$
```


### PUB/SUB with 100 threads and poll

With more threads, several threads lose initial messages at connect, but shutdown does proceed normally.

```
$ ./pubsub -threads 100 -msgs 1000 -quiet -poll
Using libzmq version 4.2.3
Using pub/sub sockets
Sleeping for 1 seconds at shutdown
Polling after connect
Thread bc9810 received # 3, expected # 1
Thread bc9800 received # 3, expected # 1
Thread bc9808 received # 3, expected # 1
Thread bc9828 received # 3, expected # 1
Thread bc9830 received # 3, expected # 1
Thread bc9858 received # 2, expected # 1
Thread bc9860 received # 2, expected # 1
Thread bc97d0 received # 2, expected # 1
Thread bc9870 received # 2, expected # 1
Thread bc9878 received # 2, expected # 1
Thread bc9888 received # 2, expected # 1
Thread bc9850 received # 2, expected # 1
Thread bc9820 received # 3, expected # 1
Thread bc9818 received # 3, expected # 1
Thread bc97d8 received # 3, expected # 1
Thread bc9838 received # 3, expected # 1
Thread bc9880 received # 2, expected # 1
Thread bc9840 received # 3, expected # 1
Thread bc9868 received # 4, expected # 1
Thread bc97f0 received # 4, expected # 1
Thread bc9890 received # 2, expected # 1
Thread bc9898 received # 2, expected # 1
Thread bc97e8 received # 4, expected # 1
Thread bc98a8 received # 2, expected # 1
Thread bc97f8 received # 4, expected # 1
Thread bc97e0 received # 4, expected # 1
Thread bc9848 received # 3, expected # 1
Thread bc98a0 received # 2, expected # 1
Waiting...
$ 
```

The issue with dropping initial messages is a [well-known problem with ZeroMQ](https://github.com/zeromq/libzmq/issues/2759) when PUB sockets connect to SUB sockets.  

Unfortunately, the recommended work-around (inserting `zmq_poll` calls after connects and sends) only works up to a point.  From the various issues and email threads, it appears that the `zmq_poll` must be issued on the SUB side of the connection, but in "real-world" code that work-around is not practical:

- The SUB side has no way to know that it is supposed to issue the `zmq_poll` (unlike in the simple examples documented).
- Again, in "real-world" code the SUB side is most likely *already* in a `zmq_poll` call, but the subscription information is apparently exchanged only on entry to `zmq_poll`.

> A possible solution to this problem would be to have the SUB side return `EINTR` from the `zmq_poll` call when a connection attempt is made.   This would allow the code on the SUB side to re-enter the `zmq_poll` call, which could trigger the exchange of subscription information.  I have no idea whether this would be practical, or even possible.

### PAIRs with 1 thread and poll
When using ZMQ_PAIR sockets, the sending and receving threads deadlock, with or without `-poll`, and the program has to be terminated with Ctrl-C.

When run without `-poll`, the pairs example deadlocks immediately:

```
$ ./pairs -threads 1 -msgs 1000 -quiet
Using libzmq version 4.2.3
Using pair sockets
Sleeping for 1 seconds at shutdown
^C
Thread e71150 sent # 2, received # 1
$ 
```

When run with `-poll`, it takes a little longer:

```
$ ./pairs -poll -threads 1 -msgs 1000 -quiet
Using libzmq version 4.2.3
Using pair sockets
Sleeping for 1 seconds at shutdown
Polling after connect
^C
Thread 2510150 sent # 37, received # 36
$ 
```

Running the debugger on the hung program shows the relevant stack traces:

```
$ gdb -p $(pidof pairs)
...
(gdb) thread apply all bt
Thread 5 (Thread 0x7f029a6f1700 (LWP 15800)):
#0  0x00000033bfedf383 in poll () from /lib64/libc.so.6
#1  0x00007f029af5e83e in zmq::socket_poller_t::wait (this=0x7f029a6f0d30, events_=0x7f0294004a20, n_events_=2, timeout_=-1)
    at ~/work/libzmq/src/socket_poller.cpp:447
#2  0x00007f029af5c376 in zmq_poller_wait_all (poller_=0x7f029a6f0d30, events=0x7f0294004a20, n_events=2, timeout_=-1)
    at ~/work/libzmq/src/zmq.cpp:1371
#3  0x00007f029af5cbda in zmq_poller_poll (items_=0x7f029a6f0e00, nitems_=2, timeout_=-1)
    at ~/work/libzmq/src/zmq.cpp:813
#4  0x00007f029af5bd09 in zmq_poll (items_=0x7f029a6f0e00, nitems_=2, timeout_=-1)
    at ~/work/libzmq/src/zmq.cpp:861
#5  0x0000000000404992 in mainLoop () at ~/work/zmqtests/threads/pairs.cpp:88
#6  0x00000033c0607aa1 in start_thread () from /lib64/libpthread.so.0
#7  0x00000033bfee8bcd in clone () from /lib64/libc.so.6

Thread 4 (Thread 0x7f0299cf0700 (LWP 15801)):
#0  0x00000033bfedf383 in poll () from /lib64/libc.so.6
#1  0x00007f029af35127 in zmq::signaler_t::wait (this=0x7f028c000e08, timeout_=-1)
    at ~/work/libzmq/src/signaler.cpp:233
#2  0x00007f029af097bb in zmq::mailbox_t::recv (this=0x7f028c000da0, cmd_=0x7f0299cefc00, timeout_=-1)
    at ~/work/libzmq/src/mailbox.cpp:81
#3  0x00007f029af3aaee in zmq::socket_base_t::process_commands (this=0x7f028c0008c0, timeout_=-1, throttle_=false)
    at ~/work/libzmq/src/socket_base.cpp:1335
#4  0x00007f029af3a2e4 in zmq::socket_base_t::send (this=0x7f028c0008c0, msg_=0x7f0299cefd60, flags_=0)
    at ~/work/libzmq/src/socket_base.cpp:1148
#5  0x00007f029af5ab76 in s_sendmsg (s_=0x7f028c0008c0, msg_=0x7f0299cefd60, flags_=0)
    at ~/work/libzmq/src/zmq.cpp:375
#6  0x00007f029af5acbd in zmq_send (s_=0x7f028c0008c0, buf_=0x7f0299cefe40, len_=24, flags_=0)
    at ~/work/libzmq/src/zmq.cpp:409
#7  0x0000000000404836 in sendCommand (context=0x1a88ec0, msg=0x7f0299cefe40, msgSize=24)
    at ~/work/zmqtests/threads/pairs.cpp:55
#8  0x00000000004042e9 in sendControlMsg (context=0x1a88ec0, command=80 'P', voidArg=0x1a88150, longArg=121)
    at ~/work/zmqtests/threads/common.h:55
#9  0x0000000000404364 in commandLoop (threadAddr=0x1a88150) at ~/work/zmqtests/threads/common.h:65
#10 0x00000033c0607aa1 in start_thread () from /lib64/libpthread.so.0
#11 0x00000033bfee8bcd in clone () from /lib64/libc.so.6
...
```

### PUSH/PULL
Push/pull sockets appear to work, with or without `zmq_poll`.

```
$ ./pushpull -threads 100 -msgs 1000 -quiet -poll
Using libzmq version 4.2.3
Using pub/sub sockets
Sleeping for 1 seconds at shutdown
Polling after connect
Waiting...
$ 
```

```
$ ./pushpull -threads 100 -msgs 1000 -quiet
Using libzmq version 4.2.3
Using pub/sub sockets
Sleeping for 1 seconds at shutdown
Waiting...
$
```

Push/pull sockets have the desirable quality that they block on send if the message cannot be delivered:

> When a ZMQ\_PUSH socket enters the mute state due to having reached the high water mark for all downstream nodes, or if there are no downstream nodes at all, then any zmq_send(3) operations on the socket shall block until the mute state ends or at least one downstream node becomes available for sending; **messages are not discarded**. 
> 
> <http://api.zeromq.org/4-2:zmq-socket>

### CLIENT/SERVER
Client/Server sockets also work, both with and without `-poll`.

```
$ ./clientserver -poll -threads 100 -msgs 1000 -quiet
Using libzmq version 4.2.3
Using client/server sockets
Sleeping for 1 us between sends
Polling after connect
Waiting...
$
```

```
$ ./clientserver -threads 100 -msgs 1000 -quiet
Using libzmq version 4.2.3
Using client/server sockets
Sleeping for 1 us between sends
Waiting...
$ 
```

Client/server sockets have the added benefit that they are thread-safe by design, which avoids the complications of allocating the sending socket in thread-local storage, and closing it when done.

>Note that client/server sockets are currently marked "draft" -- you need to `#define ZMQ_BUILD_DRAFT_API` to make them available, and they may change slightly in subsequent releases.

# Conclusions

For inter-thread signalling, the examples given in [The ZeroMQ Guide](http://zguide.zeromq.org/page:all#Signaling-Between-Threads-PAIR-Sockets) are misleading -- while the samples will work in the simple use-case documented, for "real-world" programs where an abitrary number of threads need to be able to send an arbitrary number of messages at any time, they fail by either dropping initial messages or hanging.

For reliable inter-thread signalling, either PUSH/PULL or CLIENT/SERVER sockets should be used, primarily because both socket types have the desirable quality that sends block until the message can be delivered.

> I didn't bother testing ROUTER/DEALER sockets because they *should* operate similarly to PUSH/PULL, but have the added complication of requiring multi-part messages.  If anyone cares enough, I'd be happy to accept a pull request.

While other socket types (e.g., PUB/SUB, PAIR) may *appear* to work, they will fail under certain conditions, and thus cannot be relied on. 

Some additional notes: 

- If PUSH/PULL sockets are used, one must take care not to access them across threads.  
 - The sample programs illustrate one way to ensure thread-safety using thread-local strorage for the sending sockets.     
- CLIENT/SERVER sockets avoid the complications of allocating and freeing thread-local storage, since they are thread-safe by design.
- CLIENT/SERVER sockets are currently considered DRAFT, and as such may change somewhat in subsequent releases (although any changes should not affect their basic operation).

## References

Dropped messages at connect: <https://github.com/zeromq/libzmq/issues/2267>, <https://github.com/zeromq/libzmq/issues/1594>

Deadlock w/ZMQ_PAIR sockets: <https://github.com/zeromq/libzmq/issues/2759>









