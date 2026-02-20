#!/bin/sh
main()
{
	interface=$1
	mac_addr=$2

	if [ -z "$1" ] || [ -z "$2" ]; then
        exit 1
    fi

	ifconfig $interface down
	ifconfig $interface hw ether $mac_addr
}

main $1 $2