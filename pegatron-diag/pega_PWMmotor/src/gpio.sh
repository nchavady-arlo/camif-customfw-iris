echo "Unexport the GPIO from user space"

echo 63 > /sys/class/gpio/unexport
echo 64 > /sys/class/gpio/unexport


echo 44 > /sys/class/gpio/unexport
echo 59 > /sys/class/gpio/unexport
echo 58 > /sys/class/gpio/unexport


echo 17 > /sys/class/gpio/unexport
echo 55 > /sys/class/gpio/unexport
echo 54 > /sys/class/gpio/unexport

#check gpio status
cat /sys/kernel/debug/gpio
