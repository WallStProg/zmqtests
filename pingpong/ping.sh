#!/bin/bash

source ./setenv.sh

./ping -ping $PING_PUB -pong $PONG_PUB $@