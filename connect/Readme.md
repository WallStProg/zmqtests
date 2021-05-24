# Connect tests
The `connect.cpp` program can be used to test under what conditions connections are accepted or rejected by 0mq.

The `build.sh` script can be used to build the executables.  

Usage (values shown are defaults):


The `dummy.cpp` program can be used to test with a server that does nothing but `accept`:

```
$ ./dummy -port 9998
11:57:38.977865	Param:port=9998
11:57:38.978844	name:dataSub event:LISTENING value:11 local:tcp://127.0.0.1:9998 remote:
11:57:43.299963	name:dataSub event:ACCEPTED value:12 local:tcp://127.0.0.1:9998 remote:tcp://127.0.0.1:59398
11:57:43.300358	name:dataSub event:HANDSHAKE_SUCCEEDED value:0 local:tcp://127.0.0.1:9998 remote:tcp://127.0.0.1:59398
```

Connecting to the dummy server:

```
$ ./connect -port 9998
11:58:38.188335	Param:port=9998
11:58:38.189368	name:dataSub event:CONNECT_DELAYED value:115 local: remote:tcp://127.0.0.1:9998
11:58:38.189427	name:dataSub event:CONNECTED value:11 local:tcp://127.0.0.1:59404 remote:tcp://127.0.0.1:9998
11:58:38.189862	name:dataSub event:HANDSHAKE_SUCCEEDED value:0 local:tcp://127.0.0.1:59404 remote:tcp://127.0.0.1:9998
``` 







