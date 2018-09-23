#!/bin/bash

source ./setenv.sh

pkill -9 proxy
pkill -9 peer
pkill -9 -f repeat