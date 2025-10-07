#!/bin/sh
eth_if=eth0
eth_ip=192.168.50.119

if [ -n "$1" ]; then
	eth_if=$1
fi

if [ -n "$2" ]; then
	eth_ip=$2
fi

ifconfig $eth_if up

ifconfig $eth_if $eth_ip
