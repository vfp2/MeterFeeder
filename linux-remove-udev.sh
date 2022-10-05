#!/bin/sh

echo 'removing udev rules for Comscire devices'

if [ ! -f "/etc/udev/rules.d/45-libqwqng.rules" ]; then
    if [ -f "./udev/45-libqwqng_rules.backup" ]; then
        sudo mv ./udev/45-libqwqng_rules.backup /etc/udev/rules.d/45-libqwqng.rules
    fi
fi

if [ -f "/etc/udev/rules.d/99-meterfeeder.rules" ]; then
    sudo rm /etc/udev/rules.d/99-meterfeeder.rules
fi

echo "done!"