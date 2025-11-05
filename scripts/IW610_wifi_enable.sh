#!/bin/sh
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

	rmmod -r moal

if ifconfig $interface
	then
		echo "Interface($interface) is exist"
else
	cd /config/modules/5.10
	modprobe moal mod_para=nxp/wifi_mod_para.conf

	compare_interface_mac
fi
}

main