#!/bin/sh
export DBUS_SYSTEM_BUS_ADDRESS=unix:path=/tmp/system_bus_socket
dbus-daemon --config-file=/etc/dbus-1/system.conf &
sleep 1
#bluetoothd -n -d -f /etc/bluetooth/main.conf & //debug enable
bluetoothd -n -f /etc/bluetooth/main.conf &
sleep 1

