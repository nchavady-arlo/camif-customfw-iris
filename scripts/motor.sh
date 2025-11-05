#!/bin/sh
set -e   #error will return 1 
echo "Control motor via sysfs"

awd8833="/sys/devices/platform/soc/soc:motor"

echo "Unexport the GPIO from user space"
for gpio in 63 64 44 59 58 17 55 54; do
    echo "$gpio" > /sys/class/gpio/unexport 2>/dev/null || true
done

if ! lsmod | grep -q '^motor'; then
    insmod /lib/motor.ko
fi

case "$1" in 
	info)
	
	cd "$awd8833"
	cat info
	
	;;
	step_mode)
	
	printf "input motor: PAN(0)/Rising(1)\n"	
	read motor
	printf "input dir: Reverse(0)/Forward(1)\n" 
	read dir
	printf "input speed: 0~9\n"
	read  speed
	printf "input steps: 0~65535\n"
	read  steps
		
        cd "$awd8833"
        ls
        echo $motor > motor
        echo $dir > dir
        echo $speed > speed
        echo $steps > step
        echo 1 > start_by_step
        	
	;;
	degree_mode)
	printf "input motor: PAN(0)/Rising(1)\n"	
	read motor
	printf "input dir: Reverse(0)/Forward(1)\n" 
	read dir
	printf "input speed: 0~9\n"
	read speed
	printf "input degree: 0~65535\n"
	read degree
		
        cd "$awd8833"
        echo $motor > motor
        echo $dir > dir
        echo $speed > speed
        echo $degree > degree
        echo 1 > start_by_degree
        	
	;;
	
	timer_mode)
	
	printf "input motor: PAN(0)/Rising(1)\n"	
	read motor
	printf "input dir: Reverse(0)/Forward(1)\n" 
	read dir
	printf "input speed: 0~9\n"
	read speed
	printf "input duration: 0~65535\n"
	read duration
		
        cd "$awd8833"
        echo $motor > motor
        echo $dir > dir
        echo $speed > speed
        echo $duration > duration
        echo 1 > start_by_timer
        	
	;;		
	*)
	echo "Nothing to do!"
	exit 1
	;;
esac

echo "Exit"
exit 0
