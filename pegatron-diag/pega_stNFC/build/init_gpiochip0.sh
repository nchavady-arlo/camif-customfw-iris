#!/bin/sh

echo "== GPIO Setup Script =="

# Exit if gpiochip0 exit
if [ -e /dev/gpiochip0 ]; then
    echo "/dev/gpiochip0 exit，skip"
    echo "== Done =="
    exit 0
fi

# 1. Unexport GPIO 68 (if exported)
if [ -d /sys/class/gpio/gpio68 ]; then
    echo "Unexporting GPIO 68..."
    echo 68 > /sys/class/gpio/unexport
else
    echo "GPIO 68 not currently exported. Skipping unexport."
fi

# 2. Show GPIO chip device major:minor
if [ -f /sys/class/gpio/gpiochip0/device/dev ]; then
    echo "GPIO chip device numbers:"
    cat /sys/class/gpio/gpiochip0/device/dev
else
    echo "ERROR: gpiochip0 device node not found!"
fi

# 3. Create device node manually (usually handled by udev)
echo "Creating /dev/gpiochip0 manually..."
mknod /dev/gpiochip0 c 254 0 2>/dev/null
chmod 666 /dev/gpiochip0

echo "== Done =="
