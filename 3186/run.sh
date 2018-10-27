#!/bin/bash -xv

source ./setenv.sh

# start the proxy
nohup ./proxy > proxy.out 2>&1 &

# start the "main" peer, using ${PREFIX} from environment (e.g., for valgrind)
nohup ${PREFIX} ./peer -long-lived $@ > peer0.out 2>&1 &

# start the peers that come and go
for i in `seq 1 50`; do
   rm -f peer${i}.out
   nohup ./repeat ./peer $@ > peer${i}.out 2>&1 &
done
