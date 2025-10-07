echo PSK=$1
if [ -n "$1" ]
then
	sed -i '3d' /config/nxp/wpa.conf
	sed -i '3ipsk="'$1'"' /config/nxp/wpa.conf
	sync
else
	echo "PSK = empty"
fi

