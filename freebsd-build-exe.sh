#!/bin/sh
# TODO: will make a nice multi-platform friendly CMakeFile or something soon :-D

g++ -std=c++11 -g ./src/*.cpp -o ./builds/freebsd/meterfeeder -lpthread -lftdi1
