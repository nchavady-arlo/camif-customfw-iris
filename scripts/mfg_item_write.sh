#!/bin/sh
main()
{
	output_file=/data/arlo_cert/dts_config.txt
	#sed -i "s/^MAC1=.*/MAC1=A1:BB:CC:DD:EE:FF/" /data/arlo_cert/config.txt
	#sed -i "s/^$1=.*/$1=$2/" $3
	sed -i "s/^$1=.*/$1=$2/" $output_file
}

# /scripts/mfg_item_write.sh MAC1 "A1:BB:CC:DD:EE:01"
# /scripts/mfg_item_write.sh MAC2 "A1:BB:CC:DD:EE:03"
# /scripts/mfg_item_write.sh SERIAL "AB5U247HA003D"
# /scripts/mfg_item_write.sh MODEL "VMC6060"
# /scripts/mfg_item_write.sh HW_VERSION "0.1"
# /scripts/mfg_item_write.sh WIFI_CC "US"
# /scripts/mfg_item_write.sh SKU "NAS-1001"
# /scripts/mfg_item_write.sh UUID "12345678-1234-1234-1234-123456789acc"
# /scripts/mfg_item_write.sh PARTNER_ID "0"

# diag mfg mac1 "A1:BB:CC:DD:EE:01"
# diag mfg mac2 "A1:BB:CC:DD:EE:03"
# diag mfg sn "ABCDHA100003D"
# diag mfg model "VMC5060"
# diag mfg hwver "0.1"
# diag mfg wifi_cc "US"
# diag mfg sku "NAS-1001"
# diag mfg uuid xx
# diag mfg partnerid "0"
# diag mfg json_dl 192.168.50.103 dsc4.json
# diag mfg dtbo

main $1 $2

