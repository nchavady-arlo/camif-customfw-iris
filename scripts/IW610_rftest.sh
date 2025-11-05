#!/bin/sh

disable_wifi1()
{
	echo 14 > /sys/class/gpio/export
	echo out > /sys/class/gpio/gpio14/direction
	echo 0 > /sys/class/gpio/gpio14/value
}

disable_wifi2()
{
	echo 15 > /sys/class/gpio/export
	echo out > /sys/class/gpio/gpio15/direction
	echo 0 > /sys/class/gpio/gpio15/value
}

mfgmfgbridge()
{
	mknod /dev/gpiochip0 c 254 0
	chmod 666 /dev/gpiochip0

	sleep 1

	cd mfgbridge
	./mfgbridge
}

wait_interface()
{
    wifi_ifs=$(ifconfig -a|grep mlan0|awk '{print $1}')

    while [ "$wifi_ifs" != "mlan0" ]
    do
		echo "wifi_ifs($wifi_ifs)"
		wifi_ifs=$(ifconfig -a|grep mlan0|awk '{print $1}')
    done

	sleep 1
}

main()
{
	echo "WiFi device ($1)."

	interface="mlan0"

	fw_path=/nxp
	moal_ko=/lib/moal.ko
	mlan=/lib/mlan.ko

	rmmod -r moal

if ifconfig $interface
	then
		echo "Interface($interface) is exist"
else
	#can not control PDn to prevent loading fw failed.
	#wifi_hardware_reset
	if [ "$1" == "wifi1" ]; then
		disable_wifi2
	else
		disable_wifi1
	fi

	if [ -e $mlan ]; then
		insmod $mlan
	fi
	#-e file: True if file exists (can be a file, directory, or other type)
	if [ -e $moal_ko ]; then
		echo "@Load IW610 WiFI fw($1)...."
		insmod $moal_ko fw_name=$fw_path/sduartspi_iw610_combo_mfg.bin.se mfg_mode=1 cal_data_cfg=none drv_mode=1 auto_ds=2 ps_mode=2
	fi

	wait_interface

	if [ "$1" == "wifi1" ]; then
		#hciattach /dev/ttyS2 any 3000000 flow
		#Uart baudrate:115200 for Labtool
		hciattach /dev/ttyS2 any 115200 flow
		sleep 1
		hciconfig hci0 reset
		sleep 1
	fi

	mfgmfgbridge
fi
}

main $1
