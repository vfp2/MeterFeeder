g++ -c meterFeeder2.cpp driver.cpp generator.cpp
g++ -o meterFeeder2.exe generator.o driver.o meterFeeder2.o -L"." -lftd2xx
