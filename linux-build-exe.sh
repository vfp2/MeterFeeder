#!/bin/sh
# TODO: will make a nice multi-platform friendly CMakeFile or something soon :-D

g++ -std=c++17 -g ./src/*.cpp -o ./builds/linux/meterfeeder -lusb-1.0 -L./ftd2xx/linux -lftd2xx -lpthread
