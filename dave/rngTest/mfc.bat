g++ -c meterFeeder2.cpp driver.cpp generator.cpp
g++ -o meterFeeder2 generator.o driver.o meterFeeder2.o -L"." -lftd2xx
