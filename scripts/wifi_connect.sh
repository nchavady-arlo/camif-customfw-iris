#!/bin/sh
WPA_CONF=/etc/arlo/wpa.conf
echo SSID=$1, PSK=$2, INTERFACE=$3

# Check if network block already exists in the file
has_existing_config() {
    grep -q "network={" "$WPA_CONF" 2>/dev/null
}

if [ -n "$1" ] && [ -n "$2" ]; then
    # Both SSID and PSK provided - update config
    echo "Updating network configuration..."
    sed -i '/network={/,/^}/d' "$WPA_CONF"

    cat >> "$WPA_CONF" << EOF
network={
ssid="$1"
psk="$2"
}
EOF
    sync
    echo "Network configuration updated in $WPA_CONF"
elif has_existing_config; then
    echo "Using existing credentials"
else
    echo "ERROR: No SSID/PSK provided and no existing configuration found"
    exit 1
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

<<<<<<< HEAD
#-n string is not null
#proc_id=`pidof hostapd`
#if [ -n "$proc_id" ]; then
#killall hostapd
#sleep 1
#fi

=======
>>>>>>> update wifi_connect.sh to work with /etc/arlo/wpa.conf
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
wpa_supplicant -Dnl80211 -i $interface -c $WPA_CONF -B
sleep 1
udhcpc -i $interface -s /etc/init.d/udhcpc.script -b

