This repo demonstrates a performance regression in zmq_poll when used with thread-safe sockets.  The regression first appeared in version 4.2.4 of libzmq.

See https://github.com/zeromq/libzmq/issues/3373 for more information.

## What does the test do?
The test simulates a microbench which we use to regression test performance, which in turn simulates a typical messaging pattern in our application.

- pong is a "server" -- it listens for requests and sends two replies: an "ack" and a "response".  (Never mind why for now -- it makes sense once you understand the application).
- ping is a "client" -- it sends a request, and then listens for the two replies.

Both ping and pong take several parameters:

- When run with no parameters, messages are received directly with zmq_recv.  
- When run with the `-poll` parameter, a zmq_poll is issued prior to the zmq_recv.  This is more like our typical use case, since in the real application we are reading from a number of sockets, not just one, so we need to poll.
- When run with the `-control` parameter, the zmq_poll also includes a "control" socket, which in the real application is used to send commands to the main dispatch thread.  This is the SERVER end of a CLIENT/SERVER socket pair.  We ended up with CLIENT/SERVER because other socket types exhibit various problems (see <https://github.com/zeromq/libzmq/issues/2759>). 

In our tests, the average round-trip latency (in microseconds) was:

ping params | 4.2.3| 4.2.4
------------- | ------------- | --------------------
(none)  | 54 | 55
-poll | 58 | 60
-poll -control  | 83 | 186

So, zmq_poll is a little slower than just zmq_recv, and introducing the control socket makes it even slower, but with version 4.2.4 the difference is huge (approximately double vs. 4.2.3, or an additional 100us).

## Reproducing the issue

The following is how I've reproduced the issue.  See the setenv.sh and build.sh files to customize to your environment, or just use as-is.

### Build libzmq (optional)

```
$ git clone https://github.com/WallStProg/zmqtests.git
$ cd zmqtests
$ cd libzmq/4.2.3
$ ./build.sh -b release 2>&1 | tee build.out
$ cd -
$ cd libzmq/4.2.4
$ ./build.sh -b release 2>&1 | tee build.out
$ cd -
```

### Testing w/4.2.3
If you built libzmq as above, the existing setenv.sh should "just work" -- otherwise modify it to reflect where the relevant builds are located.

```
$ cd pingpong
$ source setenv.sh 4.2.3
$ ./build.sh
$ ./ping.sh
15:05:35.838347	Param:ping=5755
15:05:35.838433	Param:pong=5855
Avg. latency=55 micros
$ ./ping.sh -poll
15:05:46.358380	Param:ping=5755
15:05:46.358468	Param:pong=5855
15:05:46.358472	Param:poll=1
Avg. latency=57 micros
$ ./ping.sh -poll -control
15:05:57.911399	Param:ping=5755
15:05:57.911486	Param:pong=5855
15:05:57.911491	Param:poll=1
15:05:57.911495	Param:control=1
Avg. latency=85 micros
$ killall -9 pong
$ cd -
```

### Testing w/4.2.4
```
$ cd pingpong
$ source setenv.sh 4.2.4
$ ./build.sh
$ ./pong.sh &
$ ./ping.sh 
14:58:41.986366	Param:ping=5755
14:58:41.986452	Param:pong=5855
Avg. latency=56 micros
$ ./ping.sh -poll
14:58:53.170306	Param:ping=5755
14:58:53.170394	Param:pong=5855
14:58:53.170398	Param:poll=1
Avg. latency=61 micros
$ ./ping.sh -poll -control
14:59:04.578411	Param:ping=5755
14:59:04.578498	Param:pong=5855
14:59:04.578502	Param:poll=1
14:59:04.578506	Param:control=1
Avg. latency=189 micros
$ killall -9 pong 
```

## Test Environment
The tests were run on CentOS 6.9, using gcc 4.8.2 (built from source and installed in a non-standard location).  The system has 4 x i5-2400@3.10GHz CPUs, with 16GB RAM.








