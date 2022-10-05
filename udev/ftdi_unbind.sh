#!/bin/sh

# waits until the device ID is available and then unbinds it from ftdi_sio

DEVICEID="$1"
while [ ! -d /sys/bus/usb/devices/$DEVICEID* ]
do
    sleep 1
    echo $DEVICEID > /home/flux/bound
done
ls /sys/bus/usb/drivers/ftdi_sio | grep $DEVICEID > /sys/bus/usb/drivers/ftdi_sio/unbind
