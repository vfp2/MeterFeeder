##
 # Parking Warden - a graphical graph implementation of the MeterFeeder Library
 # 
 # by fp2.dev
 ##

import os
import platform
from ctypes import *
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

# Config the Python<->C++ interface with MeterFeeder
meter_feeder_lib = cdll.LoadLibrary(os.getcwd() + '/libmeterfeeder.dylib')
meter_feeder_lib.Initialize.argtypes = c_char_p,
meter_feeder_lib.Initialize.restype = c_int
meter_feeder_lib.GetNumberGenerators.restype = c_int
meter_feeder_lib.GetByte.argtypes = c_char_p, c_char_p,
meter_feeder_lib.GetByte.restype = c_ubyte

# Initialize the driver
errorReason = create_string_buffer(256)
result = meter_feeder_lib.Initialize(errorReason)
print("MeterFeeder::Initialize: result: " + str(result) + ", errorReason:", errorReason.value)
if (len(errorReason.value) > 0):
    exit(result)
result = meter_feeder_lib.GetNumberGenerators()
print("MeterFeeder::GetNumberGenerators: result: " + str(result))
# TODO: implement "MeterFeeder::GetListGenerators and dynamically get the serial numbers

#
# >>> Real-time graphing results
#

def bin_array(num): # source: https://stackoverflow.com/a/47521145/1103264
    """Convert a positive integer num into an 8-bit bit vector"""
    return np.array(list(np.binary_repr(num).zfill(8))).astype(np.int8)

fig, ax = plt.subplots()
y1data, y2data = [], []
cy1, cy2 = 0, 0

def update(frame):
    global cy1, cy2

    y1 = meter_feeder_lib.GetByte(b"QWR4M004", errorReason) # TODO: enter your own serial number
    y2 = meter_feeder_lib.GetByte(b"QWR4A003", errorReason) # TODO: enter your own serial number
    y1b = bin_array(y1)
    y2b = bin_array(y2)
    for i in range(8):
        plt.cla() # clear legend each draw
        if (y1b[i] == 1):
            cy1 += 1
            y1data.append(cy1)
        else:
            cy1 -= 1
            y1data.append(cy1)
        if (y2b[i] == 1):
            cy2 += 1
            y2data.append(cy2)
        else:
            cy2 -= 1
            y2data.append(cy2)
        plt.plot(y1data, 'b-', label="QWR4M004 (QNG Model PQ4000KU)")    # TODO: enter your own serial number/description
        plt.plot(y2data, 'r-', label="QWR4A003 (MED100K 100 kHz v1.0)")  # TODO: enter your own serial number/description

    plt.legend(loc=2)

ani = FuncAnimation(fig, update, interval=1)
plt.show()

#
# <<< EOF Real-time graphing results
#

# Shutdown the driver
meter_feeder_lib.Shutdown()
print("MeterFeeder::Shutdown")