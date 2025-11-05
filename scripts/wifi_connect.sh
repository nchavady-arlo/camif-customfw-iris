echo SSID=$1, PSK=$2, INTERFACE=$3
if [ -n "$1" ]
then
	sed -i '2d' /config/nxp/wpa.conf
	sed -i "2issid=\"$1\"" /config/nxp/wpa.conf
	sync
else
	echo "SSID = empty"
fi

if [ -n "$2" ]
then
	sed -i '3d' /config/nxp/wpa.conf
	sed -i "3ipsk=\"$2\"" /config/nxp/wpa.conf
	sync
else
	echo "PSK = empty"
fi

if [ -n "$3" ]; then
	if [ "$3" == "mlan1" ]; then
		interface="mlan1"
	elif [ "$3" == "2" ]; then
		interface="mlan1"
	else
		interface="mlan0"
	fi
else
	interface="mlan0"
fi

# Check if the interface is up
if ifconfig $interface
then
	#echo "Interface($interface) is exist"
	ifconfig $interface down
	sleep 1
else
	echo "Interface($interface) is gone"
	exit 1
fi

#-n string is not null
#proc_id=`pidof hostapd`
#if [ -n "$proc_id" ]; then
#killall hostapd
#sleep 1
#fi

proc_id=`pidof udhcpc`
if [ -n "$proc_id" ]; then
killall udhcpc
sleep 1
fi

proc_id=`pidof wpa_supplicant`
if [ -n "$proc_id" ]; then
killall wpa_supplicant
sleep 1
fi

ifconfig $interface up
sleep 1
wpa_supplicant -Dnl80211 -i $interface -c /config/nxp/wpa.conf -B
sleep 1
udhcpc -i $interface -s /etc/init.d/udhcpc.script -b

