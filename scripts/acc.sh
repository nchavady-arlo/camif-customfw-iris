#!/bin/sh
set -e   #error will return 1 
echo "Control Acc via sysfs"
echo "arg $1"

lis2dw12="/sys/devices/platform/soc/1f007c00.i2c/i2c-3/3-0019"

if ! ls -d "$lis2dw12"/iio* 2>/dev/null | grep -q .; then
    echo "Acc module not found. Please insmod first. Exiting..."
    exit 1
fi

case "$1" in 
  	acc_raw)
        cd "$lis2dw12"/iio:device0 || exit 1
        echo "X, Y, Z Raw data:"
        cat in_accel_x_raw
        cat in_accel_y_raw
        cat in_accel_z_raw
        
	;;
 	acc_scale)
        cd "$lis2dw12"/iio:device0 || exit 1
        echo "X, Y, Z Scale:"
        cat in_accel_x_scale
        cat in_accel_y_scale
        cat in_accel_z_scale
    ;;
    int_6D_on)
		cd "$lis2dw12"/iio:device0/events || exit 1
       	echo "enable 6D event:"
        echo 1 > in_accel0_change_either_en
        	
	;;
	int_6D_off)
       	cd "$lis2dw12"/iio:device0/events || exit 1
       	echo "disable 6D event:"
        echo 0 > in_accel0_change_either_en
        	
	;;
	int_freefall_on)
       	cd "$lis2dw12"/iio:device0/events || exit 1
       	echo "enable freefall event:"
        echo 1 > in_accel0_thresh_falling_en
        	
	;;
	int_freefall_off)
       	cd "$lis2dw12"/iio:device0/events || exit 1
       	echo "disable freefall event:"
        echo 0 > in_accel0_thresh_falling_en
        	
	;;
 	int_wake_on)
       	cd "$lis2dw12"/iio:device0/events || exit 1
       	echo "enable wake event:"
        echo 1 > in_accel0_thresh_rising_en
        	
	;;
	int_wake_off)
       	cd "$lis2dw12"/iio:device0/events || exit 1
       	echo "disable wake event:"
        echo 0 > in_accel0_thresh_rising_en
        	
	;;								
	*)
		echo "Nothing to do!"
		exit 1
	;;
esac
echo "Exit"
exit 0
