::# quick hack for local windows exe build

::# gcc method
::# g++ -Wall -L./ftd2xx/amd64 -lftd2xx64 -g src\*.cpp -o builds\windows\meterfeeder.exe

::# Microsoft C++ compiler method
REM Check the Visual Studio PATH config in README, then run vcvars64.bat before running this script
cl.exe /O2 src\*.cpp /MT /link /OUT:builds\windows\meterfeeder.exe ftd2xx\amd64\ftd2xx.lib