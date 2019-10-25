## Overview
These programs help in testing the socket option to stop reconnecting under certain conditions. 

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
$  ./build.sh
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

This code is intended to be used with libzmq from <https://github.com/nyfix/libzmq/tree/reconnect-stop>.

## The Original Problem
By default, 0mq sockets will attempt to re-connect if they are disconnected for any reason.

This works well for sockets connecting to endpoints at a well-known address (i.e., a constant port).  This does not work well for sockets that connect to ephemeral ports that are assigned by the OS (i.e., when the target socket is bound to a wildcard port).  

### Reproducing the original problem
It's helpful to have two sessions open to demonstrate the problem.

In session #1:

```
tail -F peer0.out
```

In a second session:

```
./run.sh
```

You should then see several "HANDSHAKE_SUCCEEDED" messages at the end the log in the first session:

```
...
14:04:54.570196	socket:0x7f8c000008c0 name:dataPub value:0 event:4096 desc:HANDSHAKE_SUCCEEDED endpoint:tcp://127.0.0.1:34084
14:04:54.570209	socket:0x7f8c00009710 name:dataSub value:0 event:4096 desc:HANDSHAKE_SUCCEEDED endpoint:tcp://127.0.0.1:34084
14:04:54.570283	socket:0x7f8c00009710 name:dataSub value:0 event:4096 desc:HANDSHAKE_SUCCEEDED endpoint:tcp://127.0.0.1:26045
14:04:54.570322	socket:0x7f8c000008c0 name:dataPub value:0 event:4096 desc:HANDSHAKE_SUCCEEDED endpoint:tcp://127.0.0.1:34084
```
 
Now, from the second session kill one of the peers:

```
pkill -n peer
```

You should then see the remaining peer continually attempt to reconnect to the stopped process:

```
...
14:12:24.830349	socket:0x7f8c00009710 name:dataSub value:158 event:4 desc:CONNECT_RETRIED endpoint:tcp://127.0.0.1:26045
14:12:24.988605	socket:0x7f8c00009710 name:dataSub value:115 event:2 desc:CONNECT_DELAYED endpoint:tcp://127.0.0.1:26045
14:12:24.988650	socket:0x7f8c00009710 name:dataSub value:27 event:128 desc:CLOSED endpoint:tcp://127.0.0.1:26045
...
```

To stop the test:

    ./stop.sh

### Testing the change

Now run the programs with the new option enabled:

    ./run.sh -stop-reconnect-on 1
    
First, we'll kill the proxy and confirm that the peer attempts to re-connect:

    pkill -n proxy    

The log will show the reconnection attempts:

```
...
14:19:45.797417	socket:0x7f4bb801ba70 name:proxySub value:175 event:4 desc:CONNECT_RETRIED endpoint:tcp://127.0.0.1:5555
14:19:45.972685	socket:0x7f4bb801ba70 name:proxySub value:115 event:2 desc:CONNECT_DELAYED endpoint:tcp://127.0.0.1:5555
14:19:45.972725	socket:0x7f4bb801ba70 name:proxySub value:24 event:128 desc:CLOSED endpoint:tcp://127.0.0.1:5555
...
```

Now, restart the proxy process:

    ./proxy
    
The log should show something like the following:

```
...
14:20:26.831725	socket:0x7f4bb801ba70 name:proxySub value:0 event:4096 desc:HANDSHAKE_SUCCEEDED endpoint:tcp://127.0.0.1:5555
14:20:26.831874	Got W from 95b97457-3f8d-4a7d-8352-e803c2c17117 with endpoint tcp://127.0.0.1:27369

14:20:26.831887	Connecting proxy pub to: tcp://127.0.0.1:27369

14:20:26.832071	socket:0x7f4bb80128c0 name:proxyPub value:115 event:2 desc:CONNECT_DELAYED endpoint:tcp://127.0.0.1:27369
14:20:26.832108	socket:0x7f4bb80128c0 name:proxyPub value:28 event:1 desc:CONNECTED endpoint:tcp://127.0.0.1:27369
14:20:26.832286	socket:0x7f4bb80128c0 name:proxyPub value:0 event:4096 desc:HANDSHAKE_SUCCEEDED endpoint:tcp://127.0.0.1:27369
```

- Note that we disable reconnects on the `proxyPub` socket -- it will reconnect in response to the "welcome" message that we receive once we connect to the new instance of the proxy process.

The next step is to kill one of the two `peer` processes, and confirm that we stop trying to reconnect after the initial failure (i.e., ECONNREFUSED error):

    pkill -n peer
    
The log should look something like the following:

```
...
14:26:13.008183	socket:0x7f4bb8009710 name:dataSub value:25 event:512 desc:DISCONNECTED endpoint:tcp://127.0.0.1:21887
14:26:13.008271	socket:0x7f4bb80008c0 name:dataPub value:20 event:512 desc:DISCONNECTED endpoint:tcp://127.0.0.1:31628
14:26:13.008305	socket:0x7f4bb8009710 name:dataSub value:158 event:4 desc:CONNECT_RETRIED endpoint:tcp://127.0.0.1:21887
14:26:13.166616	socket:0x7f4bb8009710 name:dataSub value:115 event:2 desc:CONNECT_DELAYED endpoint:tcp://127.0.0.1:21887
14:26:13.166665	socket:0x7f4bb8009710 name:dataSub value:23 event:128 desc:CLOSED endpoint:tcp://127.0.0.1:21887
```
 
