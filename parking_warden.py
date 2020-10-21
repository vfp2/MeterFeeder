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

meter_feeder_lib = cdll.LoadLibrary('./libmeterfeeder.dylib')

meter_feeder_lib.Initialize.argtypes = c_char_p,
meter_feeder_lib.Initialize.restype = c_int

buf = create_string_buffer(256)
buf.value = b'bikkurime'
res=-1
meter_feeder_lib.Initialize(buf)
print("res: " + str(res) + ", print on python side:", buf.value.decode("utf-8"))


# meter_feeder.Initialize.argtypes = [c_char_p]
# meter_feeder.Initialize.restype = c_int
# errorReason = "nothing"
# errorReason_c = POINTER(errorReason)
# res = meter_feeder.Initialize(errorReason_c)
# print(errorReason_c + " " + str(res))
        # self._qwqng_wrapper.RandUniform.argtypes = [c_void_p]
        # self._qwqng_wrapper.RandUniform.restype = c_double
        # self._qwqng_wrapper.RandNormal.argtypes = [c_void_p]
        # self._qwqng_wrapper.RandNormal.restype = c_double
        # self._qwqng_wrapper.RandBytes.argtypes = [c_void_p, c_int]
        # self._qwqng_wrapper.RandBytes.restype = POINTER(c_char)
        # self._qwqng_wrapper.Clear.argtypes = [c_void_p]
        # self._qwqng_wrapper.Reset.argtypes = [c_void_p]
        # self._qng_pointer = self._qwqng_wrapper.GetQwqngInstance()

# def randint32(self):
    # try:
    #     return self._qwqng_wrapper.RandInt32(self._qng_pointer)
    # except:
    #     self._qwqng_wrapper.Reset(self._qng_pointer)
    #     return self._qwqng_wrapper.RandInt32(self._qng_pointer)

    # def randuniform(self):
    #     try:
    #         return self._qwqng_wrapper.RandUniform(self._qng_pointer)
    #     except:
    #         self._qwqng_wrapper.Reset(self._qng_pointer)
    #         return self._qwqng_wrapper.RandUniform(self._qng_pointer)

    # def randnormal(self):
    #     try:
    #         return self._qwqng_wrapper.RandNormal(self._qng_pointer)
    #     except:
    #         self._qwqng_wrapper.Reset(self._qng_pointer)
    #         return self._qwqng_wrapper.RandNormal(self._qng_pointer)

    # def randbytes(self, length):
    #     try:
    #         return self._randbytes_arbitrary_length(length)
    #     except:
    #         self._qwqng_wrapper.Reset(self._qng_pointer)
    #         return self._randbytes_arbitrary_length(length)

    # def _randbytes_arbitrary_length(self, length):
    #     if length <= 8192:
    #         return self._qwqng_wrapper.RandBytes(self._qng_pointer, length)[:length]
    #     else:
    #         data = bytearray()
    #         for x in range(length // 8192):
    #             data.extend(self._qwqng_wrapper.RandBytes(self._qng_pointer, 8192)[:8192])
    #         bytes_needed = length % 8192
    #         if bytes_needed != 0:
    #             data.extend(self._qwqng_wrapper.RandBytes(self._qng_pointer, bytes_needed)[:bytes_needed])
    #         return bytes(data)

    # def clear(self):
    #     self._qwqng_wrapper.Clear(self._qng_pointer)