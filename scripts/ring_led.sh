#!/bin/sh
set -e   #error will return 1 
echo "Control Ring LED via sysfs"
echo "arg $1"

led_check=$(ls /sys/class/leds)

if [ -z "$led_check" ] ; then
    echo "LED module not found and init first. Exiting..."
    exit 1
fi

case "$1" in 
	mode_on)
		cd /sys/class/leds/ktd2058/
		echo 2 > mode

	;;
	mode_off)
		cd /sys/class/leds/ktd2058/
		echo 0 > mode

	;;
	mode_default)
		cd /sys/class/leds/ktd2058/
		echo 3 > mode
		
	;;
	mode_night)
		cd /sys/class/leds/ktd2058/
		echo 1 > mode

	;;
	be_on)
		cd /sys/class/leds/ktd2058/
		echo 1 > brightextend
	
	;;
	be_off)
		cd /sys/class/leds/ktd2058/
		echo 0 > brightextend

	;;
	ce_set)
		cd /sys/class/leds/ktd2058/
		echo 1 > ce_temp
	
	;;
	ce_get)
		cd /sys/class/leds/ktd2058/
		cat ce_temp
	
	;;
	fr_set)
		cd /sys/class/leds/ktd2058/
		echo 1 > fade_rate
	
	;;
	fr_get)
		cd /sys/class/leds/ktd2058/
		cat fade_rate
	
	;;
	breathing)
		cd /sys/class/leds/ktd2058/
		echo 5 > breathing

	;;
	br2colors)
		cd /sys/class/leds/ktd2058/
		echo 3 > br2colors
	;;
	br_demo)
		cd /sys/class/leds/ktd2058/	
		echo 1 > br_demo

	;;
	flower)
		cd /sys/class/leds/ktd2058/		
		echo 1 > flower

	;;	
	chase)
		cd /sys/class/leds/ktd2058/		
		echo 1 > chase
		
	;;
	chase_demo)
		cd /sys/class/leds/ktd2058/		
		echo 1 > chasing_demo

	;;	
	racing)
		cd /sys/class/leds/ktd2058/		
		echo 1 > racing

	;;
	brlogscale)
		cd /sys/class/leds/ktd2058/		
		echo 1 > brlogscale

	;;
	br_logscale_demo)
		cd /sys/class/leds/ktd2058/		
		echo 1 > br_logscale_demo


	;;
	on_breath)
		cd /sys/class/leds/ktd2058/		
		echo 1 > br_white

	;;		
		
	off_breath)
		cd /sys/class/leds/ktd2058/		
		echo 0 > br_white

	;;
	on_2stage_breath)
		cd /sys/class/leds/ktd2058/		
		echo 2 > br_white
			
	;;
	*)
		echo "Nothing to do!"
		exit 1
	;;
esac
echo "Exit"
exit 0
