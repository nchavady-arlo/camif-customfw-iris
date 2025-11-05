#!/bin/sh
main()
{
	case "$1" in
	on)
		chip-tool onoff on 2 1 --storage-directory /config/chip-tool
		;;
	off)
		chip-tool onoff off 2 1 --storage-directory /config/chip-tool
		;;
	*)
		echo "Matter Plug command error $1!"
		;;
	esac
}

main $1
