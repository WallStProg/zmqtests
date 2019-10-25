#!/bin/bash -xv

source ./setenv.sh

# start the proxy
nohup ./proxy > proxy.out 2>&1 &

# start the "main" peer, using ${PREFIX} from environment (e.g., for valgrind)
nohup ${PREFIX} ./peer -long-lived $@ > peer0.out 2>&1 &

# start the "main" peer, using ${PREFIX} from environment (e.g., for valgrind)
nohup ${PREFIX} ./peer -long-lived $@ > peer1.out 2>&1 &
