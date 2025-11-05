#!/bin/sh
wifi_hardware_reset()
{
	echo "@wifi_hardware_reset()"
	#IO_O_IW610F_PDn
	echo 14 > /sys/class/gpio/export
	echo out > /sys/class/gpio/gpio14/direction
	echo 0 > /sys/class/gpio/gpio14/value
	#IO_O_IW610G_PDn
	echo 15 > /sys/class/gpio/export
	echo out > /sys/class/gpio/gpio15/direction
	echo 0 > /sys/class/gpio/gpio15/value

	sleep 1

	echo 1 > /sys/class/gpio/gpio14/value
	echo 1 > /sys/class/gpio/gpio15/value
	
	sleep 1
}

wifi_driver_check()
{
	#MODULE_NAME="moal"
	if lsmod | grep -q $1; then
		echo "Module $1 is loaded."
	else
		echo "Module $1 is NOT loaded."
	fi
}

generate_random_mac_addr()
{
    RAN=$(od -An -N1 -x /dev/urandom | awk '{print $1}' | cut -b3-)
}

compare_interface_mac()
{
    MAC_0=$(ifconfig -a|grep mlan0|awk '{print $5}')
    MAC_1=$(ifconfig -a|grep mlan1|awk '{print $5}')

    while [ "$MAC_0" == "$MAC_1" ]
    do
        generate_random_mac_addr
        MAC_1="$(echo $MAC_1|awk -F ':' '{print $1":"$2":"$3":"$4":"$5":"}')$RAN"
        ifconfig mlan1 down
        ifconfig mlan1 hw ether $MAC_1
        #ifconfig mlan1 up
    done
}

main()
{
	interface="mlan0"

	fw_path=/nxp
	moal_ko=/lib/moal.ko

	rmmod -r moal

if ifconfig $interface
	then
		echo "Interface($interface) is exist"
else
	#can not control PDn to prevent loading fw failed.
	#wifi_hardware_reset

	#-e file: True if file exists (can be a file, directory, or other type)
	if [ -e $moal_ko ]; then

	echo "@Load IW610 WiFI fw($1)...."

	case "$1" in
	wifi_ble)
		insmod $moal_ko fw_name=$fw_path/sduart_iw610.bin.se cal_data_cfg=$fw_path/WlanCalData_ext.conf
		;;
	mfg)
		insmod $moal_ko fw_name=$fw_path/sduartspi_iw610_combo_mfg.bin.se mfg_mode=1 cal_data_cfg=none drv_mode=1 auto_ds=2 ps_mode=2
		;;
	old)
		insmod $moal_ko fw_name=$fw_path/sduartspi_iw610_old.bin.se cal_data_cfg=$fw_path/WlanCalData_ext.conf
		;;
	all)
		insmod $moal_ko fw_name=$fw_path/sduartspi_iw610.bin.se cal_data_cfg=$fw_path/WlanCalData_ext.conf
		;;
	*)
		insmod $moal_ko fw_name=$fw_path/sduartspi_iw610.bin.se cal_data_cfg=$fw_path/WlanCalData_ext.conf
		;;
	esac

	compare_interface_mac

	fi
fi
}

main $1
