#!/bin/bash

source ./setenv.sh

rm -rf CMakeFiles CMakeCache*;

cmake . && make clean && make
