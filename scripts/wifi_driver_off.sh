#!/bin/sh
ifconfig mlan0 down
ifconfig mlan1 down
ifconfig uap0 down
ifconfig uap1 down

rmmod -r moal
#rmmod -r mlan

