#!/bin/sh

ot-daemon 'spinel+spi:///dev/spidev0.0?gpio-reset-device=/dev/gpiochip0&gpio-int-device=/dev/gpiochip0&gpio-int-line=71&gpio-reset-line=19&spi-mode=0&spi-speed=1500000&spi-reset-delay=0' &
sleep 5

ot-ctl dataset init new
sleep 0.5
ot-ctl dataset commit active
sleep 0.5
ot-ctl ifconfig up
sleep 0.5
ot-ctl thread start
sleep 10
ot-ctl dataset