#!/bin/sh
# TODO: will make a nice multi-platform friendly CMakeFile or something soon :-D

g++ -std=c++11 -lusb-1.0 -L./ftd2xx/linux -lftd2xx -g *.cpp -o meterfeeder