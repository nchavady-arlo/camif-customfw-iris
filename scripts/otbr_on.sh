#!/bin/sh

# Default Log Level
OTBR_LOG_LEVEL="${*:- -d1}"

# Bring up BLE and firewall settings
bluetoothd -n -f /etc/bluetooth/main.conf &
sleep 1
hciconfig
sleep 1
bluetoothctl show
sleep 1
hciconfig hci0 up
sleep 1

echo 1 > /proc/sys/net/ipv4/ip_forward
echo 1 > /proc/sys/net/ipv6/conf/all/forwarding
echo 2 > /proc/sys/net/ipv6/conf/mlan1/accept_ra

iptables-legacy -A FORWARD -i mlan1 -o wpan0 -j ACCEPT
iptables-legacy -A FORWARD -i wpan0 -o mlan1 -j ACCEPT
ipset create -exist otbr-ingress-deny-src hash:net family inet6
ipset create -exist otbr-ingress-deny-src-swap hash:net family inet6
ipset create -exist otbr-ingress-allow-dst hash:net family inet6
ipset create -exist otbr-ingress-allow-dst-swap hash:net family inet6

# Use the parsed LOG_LEVEL variable
otbr-agent $OTBR_LOG_LEVEL -I wpan0 -B mlan1 'spinel+spi:///dev/spidev0.0?gpio-reset-device=/dev/gpiochip0&gpio-int-device=/dev/gpiochip0&gpio-int-line=71&gpio-reset-line=19&spi-mode=0&spi-speed=1500000&spi-reset-delay=0' &
