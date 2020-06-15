#!/bin/bash

source ./setenv.sh ${ZMQ_VERSION}

rm -rf CMakeFiles CMakeCache*; cmake . && make clean && make
