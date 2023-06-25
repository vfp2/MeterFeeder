#!/bin/sh
#/usr/bin/clang++ -Wall -std=c++17 -stdlib=libc++ -L/usr/local/Cellar/libusb/1.0.26/lib/ -lusb-1.0 -L ./ftd2xx -lftd2xx -g ./src/dan.cpp ./src/driver.cpp ./src/generator.cpp -o ./dan
g++ -c meterFeeder2.cpp driver.cpp generator.cpp
g++ -o meterFeeder2 generator.o driver.o meterFeeder2.o -L/usr/local/Cellar/libusb/1.0.26/lib/ -lusb-1.0 -L ../../ftd2xx -lftd2xx