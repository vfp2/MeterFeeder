# MeterFeeder & Parking Warden

**MeterFeeder** is a C++ driver library that allows you to read in random bytes simultaneously from [Core Invention](https://coreinvention.com/)'s MED (Mind Enabled Drive) quantum random number generator USB devices. Read the comment in generator.h for more of a description or MED_DEVICES.md for a list of the different type of MED devices available.

**Parking Warden** is a work-in-progress (consider it a buggy early alpha) Python wrapper around MeterFeeder library that graphs the output of these devices.

## TL;DR; I just want to run it!

So far I've tried running it on Mac and Windows (Linux to come). Environment details:

```
OS X: 10.15.7
Apple clang version 12.0.0 (clang-1200.0.32.21)
Python: tested with 2.7.16 and 3.8.2
Visual Studio Code: 1.49.3 with the various C++ development plugins installed
```

```
Windows 10 64-bit
Python: 3.9.0
Visual Studio: Community 2019 (16.0.30804.86 D16.8)
Compiler: cl /version
    Microsoft (R) C/C++ Optimizing Compiler Version 19.28.29335 for x64 
    Microsoft (R) Incremental Linker Version 14.28.29335.0
```

### To build just MeterFeeder and a basic binary to read in random byte numers (unsigned char type):

#### On a Mac

Assuming you've got libusb 1.0.23 installed (with brew etc.) and have plugged in your USB MED devices:

```bash
$ ./build-mac-executable.sh 
$ ./meterfeeder
QWR4A003 (MED100K 100 kHz v1.0): 92
QWR4M004 (QNG Model PQ4000KU): 153
```

#### On a PC

```bash
C:\>build-win-executable.sh 
C:\>meterfeeder.exe
QWR4A003 (MED100K 100 kHz v1.0): 92
QWR4M004 (QNG Model PQ4000KU): 153
```

### To run Parking Warden

Currently the alpha doesn't properly implement MeterFeeder's ability to read in the serial numbers of all the USB devices connected and they're hardcoded so I suggest compiling and running meterfeeder above, get your serial numbers (QWR4XXXX) and update them with the device descriptions in lines 47,48,65,66 of parking_warden.py. The Python script is configured for 2 devices at the moment but you can change the yNdata/cyN/yN/yNb related code relative to how many (N) devices you have plugged in. Assuming you have 2 like myself;

```bash
$ pip install numpy matplotlib
$ ./build-mac-dylib.sh
$ python parking_warden.py
```

![ScreenShot](https://raw.github.com/vfp2/MeterFeeder/master/pw_screenshot.png)

###

## More info

The MED devices are based on the FTDI chipset and require the FTD2XX driver (only the Mac version is bundled at the moment in this repo), along with libusb. One day I'll get around to getting it tested and running on Linux and Windows. If you feel like trying it yourself then please do so and throw us a pull request.