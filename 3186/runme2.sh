#!/bin/bash -xv

# start the proxy
./proxy > proxy.out 2>&1 &

# start the "main" peer, using ${PREFIX} from environment (e.g., for valgrind)
${PREFIX} ./peer -send > peer0.out 2>&1 &

# start the peers that come and go
for i in `seq 1 5`; do
   rm -f peer${i}.out
   ./repeat.sh ./peer $@ > peer${i}.out 2>&1 &
done

# monitor the main peer
top -c -p $(pgrep -d',' -f -o "peer")
