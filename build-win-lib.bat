::#!/bin/sh
::# quick hack for local windows library build

REM Check the Visual Studio PATH config in README, then run vcvars64.bat before running this script
cl.exe /O2 /D_USRDLL /D_WINDLL *.cpp /MT /link /DLL /OUT:meterfeeder.dll ftd2xx\amd64\ftd2xx.lib