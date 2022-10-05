#!/bin/sh

# Manage udev rules

echo 'setting up udev rules for Comscire devices'

if [ -f "/etc/udev/rules.d/45-libqwqng.rules" ]; then
    echo "backing up Comscire's 45-libqwqng.rules to ./udev folder"
    sudo mv /etc/udev/rules.d/45-libqwqng.rules ./udev/45-libqwqng_rules.backup
fi

if [ ! -d "/etc/udev/scripts" ]; then
    echo "creating /etc/udev/scripts folder"
    sudo mkdir /etc/udev/scripts
fi

if [ ! -f "/etc/udev/scripts/ftdi_unbind.sh" ]; then
    echo "copying ftdi_si unbind script to /etc/udev/scripts"
    sudo cp ./udev/ftdi_unbind.sh /etc/udev/scripts/
fi

if [ ! -f "/etc/udev/rules.d/99-meterfeeder.rules" ]; then
    echo "copying udev rules and reloading udev"
    sudo cp ./udev/99-meterfeeder.rules /etc/udev/rules.d/
    sudo udevadm control --reload-rules
    echo "triggering udev rules"
    sudo udevadm trigger
fi

echo 'udev rule setup finished!'
