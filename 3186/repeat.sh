#!/bin/bash

set -o pipefail

ITER=0

rm -f tillcore.out
while [ $? == 0 ]; do
   let ITER=ITER+1
   echo '*** Iteration ' $ITER
   $@ 2>&1
   /bin/sleep 1
done
