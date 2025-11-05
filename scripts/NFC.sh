#!/bin/sh
set -e   #error will return 1 
echo "DIAG for NFC"
echo "arg $1"

case "$1" in 	
	scan)
		pega_nfc&

	;;				
	*)
		echo "Nothing to do!"
		exit 1
	;;
esac
echo "Exit"
exit 0
