#!/bin/sh
# TODO: will make a nice multi-platform friendly CMakeFile or something soon :-D

/usr/bin/clang++ -Wall -std=c++17 -stdlib=libc++ -L/usr/local/Cellar/libusb/1.0.23/lib/ -lusb-1.0 -L ./ftd2xx -lftd2xx -g *.cpp -o meterfeeder