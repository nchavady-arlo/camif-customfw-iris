#!/bin/sh
set -e   #error will return 1 
echo "DIAG for NFC"
echo "arg $1"

case "$1" in 
	i2c_check)
		pega_nfc

	;;			
	scan)
		nfc_demo_st25r3916&

	;;				
	*)
		echo "Nothing to do!"
		exit 1
	;;
esac
echo "Exit"
exit 0
