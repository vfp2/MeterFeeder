::#!/bin/sh
::# quick hack for local windows library build

vcvars64.bat
cl.exe /O2 /D_USRDLL /D_WINDLL *.cpp /MT /link /DLL /OUT:meterfeeder.dll ftd2xx\amd64\ftd2xx.lib