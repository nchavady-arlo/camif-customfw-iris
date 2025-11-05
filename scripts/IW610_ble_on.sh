#!/bin/sh
export DBUS_SYSTEM_BUS_ADDRESS=unix:path=/tmp/system_bus_socket
hciattach /dev/ttyS2 any 3000000 flow
sleep 2
hciconfig hci0 reset
