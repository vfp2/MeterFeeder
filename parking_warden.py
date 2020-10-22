##
 # Parking Warden - a graphical graph implementation of the MeterFeeder Library
 # 
 # by fp2.dev
 ##

import platform
from ctypes import *
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

# Config the Python<->C++ interface with MeterFeeder
meter_feeder_lib = cdll.LoadLibrary('/Users/simon/MMI/code/MeterFeeder/libmeterfeeder.dylib')
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

fig, ax = plt.subplots()
y1data, y2data = [], []

def update(frame):
    plt.cla() # clear legend each frame

    y1 = meter_feeder_lib.GetByte(b"QWR4M004", errorReason)
    y1data.append(y1)
    plt.plot(y1data, 'b-', label="QWR4M004")

    y2 = meter_feeder_lib.GetByte(b"QWR4A003", errorReason)
    y2data.append(y2)
    plt.plot(y2data, 'r-', label="QWR4A003")

    plt.legend(loc=1)

ani = FuncAnimation(fig, update, interval=1)
plt.show()

#
# <<< EOF Real-time graphing results
#

# Shutdown the driver
meter_feeder_lib.Shutdown()
print("MeterFeeder::Shutdown")