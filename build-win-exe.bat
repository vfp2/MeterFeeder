::#!/bin/sh
::# quick hack for local windows exe build

g++ -Wall -L./ftd2xx/amd64 -lftd2xx64 -g *.cpp -o meterfeeder.exe