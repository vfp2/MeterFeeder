##
 # Parking Warden - a graphical graph implementation of the MeterFeeder Library
 # 
 # by fp2.dev
 ##

# import numpy as np
# import matplotlib.pyplot as plt

# y = np.linspace(-10, 10, 1000)

# # calculate the x value for each element of the y vector
# x = y**2 + 2*y + 2  

# fig, ax = plt.subplots()
# ax.plot(x, y)

# plt.show()

import platform
from ctypes import *

# Config the Python<->C++ interface with MeterFeeder
meter_feeder_lib = cdll.LoadLibrary('/Users/simon/DropboxPersonal/MMI/codeiMac/MeterFeeder/libmeterfeeder.dylib')
meter_feeder_lib.Initialize.argtypes = c_char_p,
meter_feeder_lib.Initialize.restype = c_int
meter_feeder_lib.GetNumberGenerators.restype = c_int
meter_feeder_lib.GetByte.argtypes = c_char_p, c_char_p,
meter_feeder_lib.GetByte.restype = c_ubyte

errorReason = create_string_buffer(256)

# Initialize the driver
result = meter_feeder_lib.Initialize(errorReason)
print("MeterFeeder::Initialize: result: " + str(result) + ", errorReason:", errorReason.value)
if (len(errorReason.value) > 0):
    exit(result)
result = meter_feeder_lib.GetNumberGenerators()
print("MeterFeeder::GetNumberGenerators: result: " + str(result))

result = meter_feeder_lib.GetByte("QWR4M004", errorReason)
print("MeterFeeder::GetByte: result: " + str(result) + ", errorReason:", errorReason.value)

# Shutdown the driver
meter_feeder_lib.Shutdown()
print("MeterFeeder::Shutdown")