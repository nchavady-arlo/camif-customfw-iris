#!/bin/sh
hciattach /dev/ttyS2 any 3000000 flow
sleep 2
hciconfig hci0 reset