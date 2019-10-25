#!/bin/bash

source ./setenv.sh

pkill -n $@ proxy
pkill $@ peer
