#!/bin/sh
fw_path=/nxp
moal_ko=/lib/moal.ko

rmmod -r moal

if [ -e $moal_ko ]; then
	#insmod $moal_ko fw_name=$fw_path/sduartspi_iw610.bin.se
	insmod $moal_ko fw_name=$fw_path/sduartspi_iw610.bin.se cal_data_cfg=$fw_path/WlanCalData_ext_thread.conf
fi