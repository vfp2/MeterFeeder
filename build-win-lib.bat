::#!/bin/sh
::# quick hack for local windows library build

g++ -Wall -L./ftd2xx/amd64 -lftd2xx64 -g *.cpp -o meterfeeder.dll -shared -fPIC