#!/bin/sh
# TODO: will make a nice multi-platform friendly CMakeFile or something soon :-D

g++ -std=c++11 -g driver.cpp generator.cpp -o libmeterfeeder.so -lusb -L./ftd2xx/linux -lftd2xx -lpthread -shared -fPIC