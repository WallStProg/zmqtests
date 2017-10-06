Sample that reproduces problems referenced in https://github.com/zeromq/libzmq/issues/2759.

When run with no parameters, the program will eventually hang in what appears to be a deadlock between zmq_poll and zmq_send.  Use Ctrl-C to terminate the program.

```
$  ./threads
Using libzmq version 4.2.3
Using pair sockets
Sleeping for 0 us between sends
...
Got control msg from thread 602878 with seqNum 3, previous seqNum was 1
.....................................................................................................^C
Max control msg sent 104

Max control msg received 96
threads: /home/btorpey/work/zmqtests/threads/threads.cpp:37: void onSignal(int): Assertion `false' failed.
Aborted (core dumped)
```

The full stack trace is reproduced following.

The program can also be run with pub/sub sockets instead of pair sockets.  When run with pub/sub sockets, the program will not hang.
In either case, messages are (randomly?) dropped.

All (relevant) zmq calls are checked for errors, and the program will immediately terminate if any are found.  

This program has been tested on CentOS 6, using gcc 4.8.2, and libzmq 4.2.3 (d17581929cceceda02b4eb8abb054f996865c7a6).

The program takes following command-line params:

Param  | Meaning
------------- | -------------
-sleep n|Sleep for n microseconds between sends.
-pubsub|Use ZMQ_PUB/ZMQ_SUB sockets.  Default is to use ZMQ_PAIR sockets.
-debug|Output verbose debugging information.

### Stack Trace
Following is a sample stack trace from deadlock with ZMQ_PAIR sockets.

```

Thread 5 (Thread 0x7f9e96db0700 (LWP 25649)):
#0  0x00000033bfee91c3 in epoll_wait () from /lib64/libc.so.6
No symbol table info available.
#1  0x00007f9e975aa3e1 in zmq::epoll_t::loop (this=0x17729f0) at /home/btorpey/work/libzmq/master/src/epoll.cpp:168
        timeout = 0
        n = 1
        ev_buf = {{events = 1, data = {ptr = 0x1772ac0, fd = 24586944, u32 = 24586944, u64 = 24586944}}, {events = 1, data = {ptr = 0x7f9e840008c0, fd = -2080372544, u32 = 2214594752, u64 = 140318796155072}}, {events = 1, data = {ptr = 0x7f9e84000db0, 
              fd = -2080371280, u32 = 2214596016, u64 = 140318796156336}}, {events = 1, data = {ptr = 0x7f9e84000dd0, fd = -2080371248, u32 = 2214596048, u64 = 140318796156368}}, {events = 1, data = {ptr = 0x1772ac0, fd = 24586944, u32 = 24586944, u64 = 24586944}}, {
            events = 1, data = {ptr = 0x7f9e84000df0, fd = -2080371216, u32 = 2214596080, u64 = 140318796156400}}, {events = 1, data = {ptr = 0x7f9e84000e10, fd = -2080371184, u32 = 2214596112, u64 = 140318796156432}}, {events = 0, data = {ptr = 0x0, fd = 0, u32 = 0, 
              u64 = 0}} <repeats 188 times>, {events = 0, data = {ptr = 0x33bfa096d4 <check_match+228>, fd = -1079994668, u32 = 3214972628, u64 = 222258304724}}, {events = 0, data = {ptr = 0x96dafb4000000000, fd = 0, u32 = 0, u64 = 10870276902908592128}}, {
            events = 32670, data = {ptr = 0x0, fd = 0, u32 = 0, u64 = 0}}, {events = 2530933568, data = {ptr = 0xa00007f9e, fd = 32670, u32 = 32670, u64 = 42949705630}}, {events = 0, data = {ptr = 0x7f9e96fc3738, fd = -1761855688, u32 = 2533111608, 
              u64 = 140319114671928}}, {events = 2046907622, data = {ptr = 0xbfa09fe200000000, fd = 0, u32 = 0, u64 = 13808212250529366016}}, {events = 51, data = {ptr = 0xa, fd = 10, u32 = 10, u64 = 10}}, {events = 31982931, data = {ptr = 0x2600000000, fd = 0, 
              u32 = 0, u64 = 163208757248}}, {events = 0, data = {ptr = 0x7f9e97509998, fd = -1756325480, u32 = 2538641816, u64 = 140319120202136}}, {events = 0, data = {ptr = 0x96dafcc000000000, fd = 0, u32 = 0, u64 = 10870278552176033792}}, {events = 32670, data = {
              ptr = 0x7f9e9750b7c0, fd = -1756317760, u32 = 2538649536, u64 = 140319120209856}}, {events = 2538715128, data = {ptr = 0x7f9e, fd = 32670, u32 = 32670, u64 = 32670}}, {events = 0, data = {ptr = 0x7f9e96dafce0, fd = -1764033312, u32 = 2530933984, 
              u64 = 140319112494304}}, {events = 3227520768, data = {ptr = 0xc0601b0000000033, fd = 51, u32 = 51, u64 = 13862109339860336691}}, {events = 51, data = {ptr = 0x0, fd = 0, u32 = 0, u64 = 0}}, {events = 0, data = {ptr = 0x9784a83800000000, fd = 0, u32 = 0, 
              u64 = 10918036355124559872}}, {events = 32670, data = {ptr = 0x0, fd = 0, u32 = 0, u64 = 0}}, {events = 0, data = {ptr = 0x9784a83800000000, fd = 0, u32 = 0, u64 = 10918036355124559872}}, {events = 32670, data = {ptr = 0x7f9e97535e2a, fd = -1756144086, 
              u32 = 2538823210, u64 = 140319120383530}}, {events = 2538761448, data = {ptr = 0x9751b7f800007f9e, fd = 32670, u32 = 32670, u64 = 10903698448620486558}}, {events = 32670, data = {ptr = 0x500000000, fd = 0, u32 = 0, u64 = 21474836480}}, {events = 2733, 
            data = {ptr = 0xc0602df000000001, fd = 1, u32 = 1, u64 = 13862130161861787649}}, {events = 51, data = {ptr = 0x7f9e9750c600, fd = -1756314112, u32 = 2538653184, u64 = 140319120213504}}, {events = 2542054288, data = {ptr = 0x96dafd1000007f9e, fd = 32670, 
              u32 = 32670, u64 = 10870278895773450142}}, {events = 32670, data = {ptr = 0x7f9e96dafd38, fd = -1764033224, u32 = 2530934072, u64 = 140319112494392}}, {events = 2542053432, data = {ptr = 0x7f9e, fd = 32670, u32 = 32670, u64 = 32670}}, {events = 0, 
            data = {ptr = 0x7a0154e6, fd = 2046907622, u32 = 2046907622, u64 = 2046907622}}, {events = 3214975594, data = {ptr = 0x33, fd = 51, u32 = 51, u64 = 51}}, {events = 0, data = {ptr = 0x0, fd = 0, u32 = 0, u64 = 0}}, {events = 5, data = {ptr = 0x0, fd = 0, 
              u32 = 0, u64 = 0}}, {events = 0, data = {ptr = 0x1, fd = 1, u32 = 1, u64 = 1}}, {events = 2542053432, data = {ptr = 0x500007f9e, fd = 32670, u32 = 32670, u64 = 21474869150}}, {events = 0, data = {ptr = 0x0, fd = 0, u32 = 0, u64 = 0}}, {events = 1, 
            data = {ptr = 0x9784a83800000000, fd = 0, u32 = 0, u64 = 10918036355124559872}}, {events = 32670, data = {ptr = 0x0, fd = 0, u32 = 0, u64 = 0}}, {events = 2542054288, data = {ptr = 0x96dafcc000007f9e, fd = 32670, u32 = 32670, u64 = 10870278552176066462}}, {
            events = 32670, data = {ptr = 0x7f9e96fc4000, fd = -1761853440, u32 = 2533113856, u64 = 140319114674176}}, {events = 2530933976, data = {ptr = 0x9784ab9000007f9e, fd = 32670, u32 = 32670, u64 = 10918040031616597918}}, {events = 1, data = {
              ptr = 0x7f9e96dafce0, fd = -1764033312, u32 = 2530933984, u64 = 140319112494304}}, {events = 2538823210, data = {ptr = 0x96dafcf800007f9e, fd = 32670, u32 = 32670, u64 = 10870278792694235038}}, {events = 32670, data = {ptr = 0x100000000, fd = 0, u32 = 0, 
              u64 = 4294967296}}, {events = 0, data = {ptr = 0x97559d5700000000, fd = 0, u32 = 0, u64 = 10904795069735239680}}, {events = 32670, data = {ptr = 0x7f9e96dafdf0, fd = -1764033040, u32 = 2530934256, u64 = 140319112494576}}, {events = 2530934296, data = {
              ptr = 0x96fc44c800007f9e, fd = 32670, u32 = 32670, u64 = 10879646425604456350}}, {events = 32670, data = {ptr = 0x7f9e96fc22e8, fd = -1761860888, u32 = 2533106408, u64 = 140319114666728}}, {events = 2822930839, data = {ptr = 0xbfa0a26a00000000, fd = 0, 
              u32 = 0, u64 = 13808215033668173824}}, {events = 51, data = {ptr = 0x0, fd = 0, u32 = 0, u64 = 0}}, {events = 2533106408, data = {ptr = 0x9751b7f800007f9e, fd = 32670, u32 = 32670, u64 = 10903698448620486558}}, {events = 32670, data = {
              ptr = 0x7f9e9784a838, fd = -1752913864, u32 = 2542053432, u64 = 140319123613752}}, {events = 1, data = {ptr = 0x585406600000000, fd = 0, u32 = 0, u64 = 397794948922998784}}, {events = 0, data = {ptr = 0x33c0601b00, fd = -1067443456, u32 = 3227523840, 
              u64 = 222270855936}}, {events = 5, data = {ptr = 0x345da800000000, fd = 0, u32 = 0, u64 = 14739674924843008}}, {events = 0, data = {ptr = 0x0, fd = 0, u32 = 0, u64 = 0}}, {events = 0, data = {ptr = 0x300000000, fd = 0, u32 = 0, u64 = 12884901888}}, {
            events = 0, data = {ptr = 0x7f9e97502000, fd = -1756356608, u32 = 2538610688, u64 = 140319120171008}}, {events = 3214992416, data = {ptr = 0x500000033, fd = 51, u32 = 51, u64 = 21474836531}}, {events = 0, data = {ptr = 0x0, fd = 0, u32 = 0, u64 = 0}}, {
            events = 2538610688, data = {ptr = 0x9751b7f800007f9e, fd = 32670, u32 = 32670, u64 = 10903698448620486558}}, {events = 32670, data = {ptr = 0x5, fd = 5, u32 = 5, u64 = 5}}, {events = 0, data = {ptr = 0x96dafdd000000000, fd = 0, u32 = 0, 
              u64 = 10870279720407138304}}, {events = 32670, data = {ptr = 0x33c081c360 <__default_pthread_attr>, fd = -1065237664, u32 = 3229729632, u64 = 222273061728}}}
#2  0x00007f9e975aa6b4 in zmq::epoll_t::worker_routine (arg_=0x17729f0) at /home/btorpey/work/libzmq/master/src/epoll.cpp:203
No locals.
#3  0x00007f9e975f6c8e in thread_routine (arg_=0x1772a70) at /home/btorpey/work/libzmq/master/src/thread.cpp:106
        signal_set = {__val = {18446744067267100671, 18446744073709551615 <repeats 15 times>}}
        rc = 0
        self = 0x1772a70
#4  0x00000033c0607aa1 in start_thread () from /lib64/libpthread.so.0
No symbol table info available.
#5  0x00000033bfee8bcd in clone () from /lib64/libc.so.6
No symbol table info available.

Thread 4 (Thread 0x7f9e959ae700 (LWP 25651)):
#0  0x00000033bfedf383 in poll () from /lib64/libc.so.6
No symbol table info available.
#1  0x00007f9e97606615 in zmq::socket_poller_t::wait (this=0x7f9e959add50, events_=0x7f9e88000940, n_events_=2, timeout_=-1) at /home/btorpey/work/libzmq/master/src/socket_poller.cpp:447
        rc = 1
        timeout = -1
        found = 0
        clock = {last_tsc = 3183390477903906, last_time = 1029228886}
        now = 0
        end = 0
        first_pass = false
#2  0x00007f9e97604189 in zmq_poller_wait_all (poller_=0x7f9e959add50, events=0x7f9e88000940, n_events=2, timeout_=-1) at /home/btorpey/work/libzmq/master/src/zmq.cpp:1371
        rc = 0
#3  0x00007f9e976049d1 in zmq_poller_poll (items_=0x7f9e959ade20, nitems_=2, timeout_=-1) at /home/btorpey/work/libzmq/master/src/zmq.cpp:813
        poller = {tag = 3405691582, signaler = 0x0, items = std::vector of length 2, capacity 2 = {{socket = 0x17735d0, fd = 0, user_data = 0x0, events = 1, pollfd_index = -1}, {socket = 0x1775670, fd = 0, user_data = 0x0, events = 1, pollfd_index = -1}}, 
          need_rebuild = false, use_signaler = false, poll_size = 2, pollfds = 0x7f9e88000990}
        repeat_items = false
        j_start = -1785012672
        found_events = 32670
        rc = 0
        events = 0x7f9e88000940
#4  0x00007f9e97603aeb in zmq_poll (items_=0x7f9e959ade20, nitems_=2, timeout_=-1) at /home/btorpey/work/libzmq/master/src/zmq.cpp:861
No locals.
#5  0x0000000000401ad6 in mainLoop () at /home/btorpey/work/zmqtests/threads/threads.cpp:129
        items = {{socket = 0x17735d0, fd = 0, events = 1, revents = 0}, {socket = 0x1775670, fd = 0, events = 1, revents = 0}}
        rc = 1
        zmsg = {_ = "\000\000\000\000\000\000\000\000P\000\000\000\000\000\000\000x(`\000\000\000\000\000`\000\000\000\000\000\000\000`Á\300\063\000\000\000\300\030e\000\000\177\000\000\020\316\372\224\236\177\000\000M(`\227\000\000\000"}
#6  0x00000033c0607aa1 in start_thread () from /lib64/libpthread.so.0
No symbol table info available.
#7  0x00000033bfee8bcd in clone () from /lib64/libc.so.6
No symbol table info available.

Thread 3 (Thread 0x7f9e94fad700 (LWP 25652)):
#0  0x00000033bfedf383 in poll () from /lib64/libc.so.6
No symbol table info available.
#1  0x00007f9e975dd77d in zmq::signaler_t::wait (this=0x7f9e800095d8, timeout_=-1) at /home/btorpey/work/libzmq/master/src/signaler.cpp:233
        pfd = {fd = 12, events = 1, revents = 0}
        rc = 0
#2  0x00007f9e975b10c1 in zmq::mailbox_t::recv (this=0x7f9e80009570, cmd_=0x7f9e94facc00, timeout_=-1) at /home/btorpey/work/libzmq/master/src/mailbox.cpp:81
        rc = -1
        ok = false
#3  0x00007f9e975e3733 in zmq::socket_base_t::process_commands (this=0x7f9e800008c0, timeout_=-1, throttle_=false) at /home/btorpey/work/libzmq/master/src/socket_base.cpp:1335
        rc = 32670
        cmd = {destination = 0x7f9e80009630, type = zmq::command_t::pipe_term, args = {stop = {<No data fields>}, plug = {<No data fields>}, own = {object = 0x7f9e959ad980}, attach = {engine = 0x7f9e959ad980}, bind = {pipe = 0x7f9e959ad980}, 
            activate_read = {<No data fields>}, activate_write = {msgs_read = 140319091513728}, hiccup = {pipe = 0x7f9e959ad980}, pipe_term = {<No data fields>}, pipe_term_ack = {<No data fields>}, pipe_hwm = {inhwm = -1785013888, outhwm = 32670}, term_req = {
              object = 0x7f9e959ad980}, term = {linger = -1785013888}, term_ack = {<No data fields>}, reap = {socket = 0x7f9e959ad980}, reaped = {<No data fields>}, done = {<No data fields>}}}
#4  0x00007f9e975e2ef2 in zmq::socket_base_t::send (this=0x7f9e800008c0, msg_=0x7f9e94facdb0, flags_=0) at /home/btorpey/work/libzmq/master/src/socket_base.cpp:1148
        sync_lock = {mutex = 0x0}
        rc = -1
        timeout = -1
        end = 0
#5  0x00007f9e97602960 in s_sendmsg (s_=0x7f9e800008c0, msg_=0x7f9e94facdb0, flags_=0) at /home/btorpey/work/libzmq/master/src/zmq.cpp:375
        sz = 24
        rc = 0
        max_msgsz = 140319081024944
#6  0x00007f9e97602aa7 in zmq_send (s_=0x7f9e800008c0, buf_=0x7f9e94face70, len_=24, flags_=0) at /home/btorpey/work/libzmq/master/src/zmq.cpp:409
        msg = {_ = "\000\000\000\000\000\000\000\000P\000\000\000\000\000\000\000x(`\000\000\000\000\000h\000\000\000\000\000\000\000`Á\300\063\000\000\000\300\030e\000\000\177\000\000\020\316\372\224\236\177\000\000M(`\227\000\000\000"}
        __PRETTY_FUNCTION__ = "int zmq_send(void*, const void*, size_t, int)"
        s = 0x7f9e800008c0
        rc = 0
#7  0x0000000000401849 in sendCommand (context=0x176fec0, msg=0x7f9e94face70, msgSize=24) at /home/btorpey/work/zmqtests/threads/threads.cpp:68
        temp = 0x7f9e800008c0
        rc = 0
        i = -1065237664
        __PRETTY_FUNCTION__ = "void sendCommand(void*, zmqControlMsg*, int)"
#8  0x0000000000401942 in commandLoop (threadAddr=0x602878 <thread2>) at /home/btorpey/work/zmqtests/threads/threads.cpp:91
        msg = {command = 80 'P', threadAddr = 0x602878 <thread2>, seqNum = 104}
#9  0x00000033c0607aa1 in start_thread () from /lib64/libpthread.so.0
No symbol table info available.
#10 0x00000033bfee8bcd in clone () from /lib64/libc.so.6
No symbol table info available.

Thread 2 (Thread 0x7f9e963af700 (LWP 25650)):
#0  0x00000033bfee91c3 in epoll_wait () from /lib64/libc.so.6
No symbol table info available.
#1  0x00007f9e975aa3e1 in zmq::epoll_t::loop (this=0x1773270) at /home/btorpey/work/libzmq/master/src/epoll.cpp:168
        timeout = 0
        n = 1
        ev_buf = {{events = 1, data = {ptr = 0x1773340, fd = 24589120, u32 = 24589120, u64 = 24589120}}, {events = 0, data = {ptr = 0x0, fd = 0, u32 = 0, u64 = 0}} <repeats 255 times>}
#2  0x00007f9e975aa6b4 in zmq::epoll_t::worker_routine (arg_=0x1773270) at /home/btorpey/work/libzmq/master/src/epoll.cpp:203
No locals.
#3  0x00007f9e975f6c8e in thread_routine (arg_=0x17732f0) at /home/btorpey/work/libzmq/master/src/thread.cpp:106
        signal_set = {__val = {18446744067267100671, 18446744073709551615 <repeats 15 times>}}
        rc = 0
        self = 0x17732f0
#4  0x00000033c0607aa1 in start_thread () from /lib64/libpthread.so.0
No symbol table info available.
#5  0x00000033bfee8bcd in clone () from /lib64/libc.so.6
No symbol table info available.

Thread 1 (Thread 0x7f9e96fc0720 (LWP 25648)):
#0  0x00000033bfe32495 in raise () from /lib64/libc.so.6
No symbol table info available.
#1  0x00000033bfe33c75 in abort () from /lib64/libc.so.6
No symbol table info available.
#2  0x00000033bfe2b60e in __assert_fail_base () from /lib64/libc.so.6
No symbol table info available.
#3  0x00000033bfe2b6d0 in __assert_fail () from /lib64/libc.so.6
No symbol table info available.
#4  0x00000000004016ea in onSignal (sig=2) at /home/btorpey/work/zmqtests/threads/threads.cpp:37
        __PRETTY_FUNCTION__ = "void onSignal(int)"
#5  <signal handler called>
No symbol table info available.
#6  0x00000033bfeacbed in pause () from /lib64/libc.so.6
No symbol table info available.
#7  0x0000000000401e71 in main (argc=1, argv=0x7ffe5e215c98) at /home/btorpey/work/zmqtests/threads/threads.cpp:210
        major = 4
        minor = 2
        patch = 3

```

