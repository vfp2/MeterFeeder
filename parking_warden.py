##
 # Parking Warden - a graphical graph implementation of the MeterFeeder Library
 # 
 # by fp2.dev
 ##

import os
import sys
import time
import platform
from ctypes import *
import numpy as np
import matplotlib.backends.backend_tkagg
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from matplotlib.widgets import Button
import threading
import queue

import datetime

# Number of bytes of randomness to get per read call on a device
ENTROPY_BUFFER_LEN = 1024

MAX_X_AXIS = 300000

GRAPH_LINE_COLORS = ('b-','g-','r-','c-','m-','y-','k-','w-') # implicit max of 8 devices max TODO: need support for more??

# FIFO queue for random bits read in from devices
fq = {}

# List of connected devices with serial numbers (key) and descriptions (value)
devices = {}

# A crude multi-threaded messaging system for toggling continuous/user-initiated mode and triggering resets/grabs
# key: device serial number
# values: (commands):
# 1 - continuous mode toggle
# 2 - continuous mode reset
# 3 - user-initiated mode toggle
# 4 - user-initiated mode grab
thread_messages = {} 

# Maximum and minimum graphed walk bounds per device
maxs = {}
mins = {}

def load_library():
    # Load the MeterFeeter library
    global METER_FEEDER_LIB
    if os.name == 'nt': # windows
        METER_FEEDER_LIB = cdll.LoadLibrary('meterfeeder.dll')
    else: # mac. TODO: add linux one day
        METER_FEEDER_LIB = cdll.LoadLibrary(os.getcwd() + '/libmeterfeeder.dylib')
    METER_FEEDER_LIB.MF_Initialize.argtypes = c_char_p,
    METER_FEEDER_LIB.MF_Initialize.restype = c_int
    METER_FEEDER_LIB.MF_GetNumberGenerators.restype = c_int
    # METER_FEEDER_LIB.MF_GetListGenerators.argtypes = [POINTER(c_char_p)]
    METER_FEEDER_LIB.MF_GetBytes.argtypes = c_int, POINTER(c_ubyte), c_char_p, c_char_p,

    # Make driver initialize all the connected devices
    global med_error_reason
    med_error_reason = create_string_buffer(256)
    result = METER_FEEDER_LIB.MF_Initialize(med_error_reason)
    print("MeterFeeder::MF_Initialize: result: " + str(result) + ", error (if any): ", med_error_reason.value)
    if (len(med_error_reason.value) > 0):
        exit(result)

def get_devices():
    # Get the number of connected devices
    numGenerators = METER_FEEDER_LIB.MF_GetNumberGenerators()
    print("MeterFeeder::MF_GetNumberGenerators: " + str(numGenerators) + " connected device(s)")

    # Get the list of connected devices
    generatorsListBuffers = [create_string_buffer(58) for i in range(numGenerators)]
    generatorsListBufferPointers = (c_char_p*4)(*map(addressof, generatorsListBuffers))
    METER_FEEDER_LIB.MF_GetListGenerators(generatorsListBufferPointers)
    generatorsList = [str(s.value, 'utf-8') for s in generatorsListBuffers]
    print("MeterFeeder::MF_GetListGenerators: Device serial numbers and descriptions:")
    for i in range(numGenerators):
        kvs = generatorsList[i].split("|")
        devices[kvs[0]] = kvs[1]
        thread_messages[kvs[0]] = 1 # continuous mode
        mins[kvs[0]] = 0
        maxs[kvs[0]] = 0
        print("\t" + str(kvs[0]) + "->" + kvs[1])

def bin_array(num): # source: https://stackoverflow.com/a/47521145/1103264
    """Convert a positive integer num into an 8-bit bit vector"""
    return np.array(list(np.binary_repr(num).zfill(8))).astype(np.int8)

def get_entropies(serialNumber):
    global METER_FEEDER_LIB

    print(threading.currentThread().getName(), "entropy gathering thread starting")

    print("READ TIME\t\tDRAW TIME")

    # Read in entropy from MED device
    fq[serialNumber] = queue.Queue()
    ubuffer = (c_ubyte * ENTROPY_BUFFER_LEN).from_buffer(bytearray(ENTROPY_BUFFER_LEN))
    counter = 0
    walker = []
    mode = 1 # starts of in continuous mode

    while True: # continually read in entropy
        # Continuous/user-initiated mode toggling/resetting/grabbing logic
        control_message = thread_messages[serialNumber]
        if (mode == 3 and control_message == 3): # in user-initiated mode but no grab command so don't do any entropy reading/processing
            time.sleep(.001)
            fq[serialNumber].put_nowait(walker) # so graph frame updater doesn't get it's IO blocked waiting for something to go into the queue, even if it's nothing
            continue
            # continue # loop again
        elif (mode == 3 and control_message == 4): # in user-initiated mode with grab command
            thread_messages[serialNumber] = 3
            print(serialNumber + " doing grab")
            # ... continue below and do 1 grab (entropy reading/processing)
        elif (mode != 3 and control_message == 4): # toggle into user-initiated mode
            mode = thread_messages[serialNumber] = 3
            fq[serialNumber].empty()
            mins[serialNumber] = 0
            maxs[serialNumber] = 0
            walker.clear()
            counter = 0
            print(serialNumber + " toggled to user-initiated mode")
            continue # press once more for a grab
        elif (control_message == 2): # continuous mode reset
            mode = thread_messages[serialNumber] = 1 # continuous mode toggle
            fq[serialNumber].empty()
            mins[serialNumber] = 0
            maxs[serialNumber] = 0
            walker.clear()
            counter = 0
            print(serialNumber + " continuous mode toggled/resetted")

        # Reset graph when reach the right-side of graph
        if (len(walker) > MAX_X_AXIS):
            walker.clear()

        tic = time.perf_counter()
        METER_FEEDER_LIB.MF_GetBytes(ENTROPY_BUFFER_LEN, ubuffer, serialNumber.encode("utf-8"), med_error_reason)
        for i in range(ENTROPY_BUFFER_LEN):
            # print(ubuffer[i])
            bits = bin_array(ubuffer[i])
            for i in range(8):
                if (bits[i] == 1):
                    counter += 1
                    walker.append(counter)
                    if (counter > maxs[serialNumber]):
                        maxs[serialNumber] = counter
                else:
                    counter -= 1
                    walker.append(counter)
                    if (counter < mins[serialNumber]):
                        mins[serialNumber] = counter

        print(f"{(time.perf_counter() - tic)*1000:0.0f}ms")
        fq[serialNumber].put_nowait(walker)

def handle_close(event):
    # Shutdown the driver
    METER_FEEDER_LIB.MF_Shutdown()

    # There's no doubt a more graceful way to quit but matplotlib turned out to be not thread safe and
    # I didn't intend to spend more than 2 hours look into ways to make it's window's X button nicely
    # stop the entropy-collecting thread
    sys.exit()

def cont_mode_reset_callback(event):
    # Message each device's message thread to reset/toggle its continuous mode
    for key in devices.keys():
        thread_messages[key] = 2

def user_init_mode_grab_callback(event):
    # Message each device's message thread to grab/toggle its user-initiated mode
    for key in devices.keys():
        thread_messages[key] = 4

def update(frame):
    global stime_now
    stime_now = time.perf_counter()

    # Clear legend each draw
    ax.cla()

    # y=0 axis
    ax.axhline(y=0, color='k')
    
    # Max axis sizes
    ax.set_xlim(0, MAX_X_AXIS)
    ax.set_ylim(-2000, 2000)

    # Graph entropy in the FIFO queue(s)
    ci = 0 # color index
    for key, value in devices.items():
        # Plot from the queue
        ys = fq[key].get_nowait()
        if (len(ys)) == 0:
            continue
        lastY = ys[-1]
        # print(f"\t\t\t: ys len:{len(ys)}")
        ax.plot(ys, GRAPH_LINE_COLORS[ci], label=key + " " + value + " [" + str(mins[key]) + "," + str(maxs[key]) + "," + str(lastY) +"]")
        ci += 1
        ax.legend(loc=2)

    print(f"\t\t\t{(time.perf_counter() - stime_now)*1000:0.0f}ms")

if __name__ == "__main__":
    # Init stuff
    load_library()
    get_devices()

    # Spawn individual threads for each connected devices to read in the entropy
    for key in devices.keys():
        w = threading.Thread(name=key, target=get_entropies, args=(key,))
        w.daemon = True
        w.start()

    # Setup graph and call to frame update function
    fig = plt.figure()
    fig.canvas.mpl_connect('close_event', handle_close) # For when pressing X to close window
    ax = fig.add_subplot(111, xlim=(0, MAX_X_AXIS), ylim=(-2000, 2000))

    # Button to reset graph in continuous mode or to toggle back into continuous mode from user-initiated mode
    cont_mode_btn_ax = fig.add_axes((0.12, 0.01, 0.28, 0.04))
    cont_mode_btn = Button(cont_mode_btn_ax, 'Continuous Mode/Reset')
    cont_mode_btn.on_clicked(cont_mode_reset_callback)

    # Button to toggle to user-initiated mode or grab entropy upon button presses (when already in user-initiated mode)
    user_init_mode_btn_ax = fig.add_axes((0.61, 0.01, 0.29, 0.04))
    user_init_mode_btn = Button(user_init_mode_btn_ax, 'User-initiated Mode/Grab')
    user_init_mode_btn.on_clicked(user_init_mode_grab_callback)
    
    ani = FuncAnimation(fig, update, interval=1)
    plt.show()