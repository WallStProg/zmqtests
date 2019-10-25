#!/bin/bash

source ./setenv.sh

pkill $@ proxy
pkill $@ peer
