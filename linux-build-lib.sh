#!/bin/sh
# TODO: will make a nice multi-platform friendly CMakeFile or something soon :-D

g++ -std=c++11 -g ./src/*.cpp -o ./builds/linux/libmeterfeeder.so -lusb -L./ftd2xx/linux -lftd2xx -lpthread -shared -fPIC