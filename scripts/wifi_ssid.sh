echo SSID=$1
if [ -n "$1" ]
then
	sed -i '2d' /config/nxp/wpa.conf
	sed -i "2issid=\"$1\"" /config/nxp/wpa.conf
	sync
fi

