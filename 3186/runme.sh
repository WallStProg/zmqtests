#!/bin/bash -xv

# start the proxy
./proxy > proxy.out 2>&1 &

# start the "main" peer, using ${PREFIX} from environment (e.g., for valgrind)
${PREFIX} ./peer > peer.out 2>&1 &

# start the peers that come and go
for i in `seq 1 5`; do
   ./repeat.sh ./peer -s 2 > peer${i}.out 2>&1 &
done

# monitor the main peer
top -c -p $(pgrep -d',' -f -o "peer")
