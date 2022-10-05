#!/bin/sh

# find the USB ID of the USB device which is bound as VCP and unbind it
DEVICEID="$1"
ls /sys/bus/usb/drivers/ftdi_sio | grep "$DEVICEID" > /sys/bus/usb/drivers/ftdi_sio/unbind