#!/bin/sh
echo "re-init wifi"
ifconfig mlan0 down
ifconfig mlan1 down
ifconfig uap0 down
ifconfig uap1 down
sleep 100000
#echo 0 > /sys/class/gpio/gpio101/value
#usleep 100000
sh /scripts/IW610_wifi_on.sh

