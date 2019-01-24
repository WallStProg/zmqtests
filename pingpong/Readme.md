This repo demonstrates a performance regression in zmq_poll when used with thread-safe sockets.  The regression first appeared in version 4.2.4 of libzmq.

See https://github.com/zeromq/libzmq/issues/3373 for more information.

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








