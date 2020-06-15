# Connect tests
The `connect.cpp` program can be used to test under what conditions connections are accepted or rejected by 0mq.

The `build.sh` script can be used to build the executables.  

Usage (values shown are defaults):

```
./connect -port 0 -stop-on-reconnect 0 -addr 127.0.0.1 -interval 1000
``` 

The `dummy.c` program can be used to test with a server that does nothing but `accept`.






