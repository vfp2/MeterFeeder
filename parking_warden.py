##
 # Parking Warden - a graphical graph implementation of the MeterFeeder Library
 # 
 # by fp2.dev
 ##

import platform
from ctypes import *
import numpy as np
import matplotlib.pyplot as plt

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


x = [2, 4, 6]
y = [1, 3, 5]
plt.plot(x, y)
x = [4, 8, 12]
y = [2, 6, 10]
plt.plot(x, y)
plt.show()

# plt.axis([0, 10, 0, 1])

# for i in range(10):
#     y = np.random.random()
#     plt.plot(i, y)
#     plt.pause(0.5)

# plt.show()




# plt.axis([0, 100, 0, 256])

# for i in range(100):
#     y = np.random.random()
#     result = meter_feeder_lib.GetByte("QWR4M004", errorReason)
#     print("MeterFeeder::GetByte: result: " + str(result) + ", errorReason:", errorReason.value)
#     y = result
#     plt.plot(i, y)
#     print("i:" + str(i) + ", y:" + str(y))
#     plt.pause(0.05)


# y = np.linspace(-10, 10, 1000)

# # calculate the x value for each element of the y vector
# x = y**2 + 2*y + 2  

# fig, ax = plt.subplots()
# ax.plot(x, y)

# plt.show()

# Shutdown the driver
meter_feeder_lib.Shutdown()
print("MeterFeeder::Shutdown")