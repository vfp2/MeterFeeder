#!/bin/sh
g++ -std=c++11 meterfeeder-libusb-test.cpp -lpthread -lftdi1 -lusb-1.0 -o meterfeeder-libusb-test && sudo ./meterfeeder-libusb-test
