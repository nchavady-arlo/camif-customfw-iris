#!/bin/sh
TTY=ttyGS0
USB_DEVICE_DIR=/sys/kernel/config/usb_gadget/ifs/
USB_CONFIGS_DIR=/sys/kernel/config/usb_gadget/ifs/configs/default.1
USB_FUNCTIONS_DIR=/sys/kernel/config/usb_gadget/ifs/functions

insmod_cdc_acm()
{
	# /dev/ttyGS0
	insmod /config/modules/5.10/udc-core.ko
	insmod /config/modules/5.10/u_serial.ko
	insmod /config/modules/5.10/libcomposite.ko
	insmod /config/modules/5.10/usb_f_acm.ko
	insmod /config/modules/5.10/udc-msb250x.ko
	# Do not insert g_serial and configre configfs instead
	#insmod /config/modules/5.10/g_serial.ko
}

init_cdc_acm_configfs()
{
	if [ -d /sys/kernel/config/usb_gadget ]; then
		umount /sys/kernel/config
	fi

	mount -t configfs none /sys/kernel/config
	#mkdir -p ${USB_DEVICE_DIR}
	#mkdir -p ${USB_CONFIGS_DIR}

	# configs
	mkdir -p ${USB_DEVICE_DIR}/strings/0x409
	mkdir -p ${USB_CONFIGS_DIR}/strings/0x409

	echo 0x02 > ${USB_CONFIGS_DIR}/MaxPower
	echo 0xC0 > ${USB_CONFIGS_DIR}/bmAttributes

	echo "Linux Foundation" > ${USB_DEVICE_DIR}/strings/0x409/manufacturer
	echo "ACM gadget" > ${USB_DEVICE_DIR}/strings/0x409/product
	echo "0123" > ${USB_DEVICE_DIR}/strings/0x409/serialnumber
	echo "ACM" > ${USB_CONFIGS_DIR}/strings/0x409/configuration

	# functions
	mkdir ${USB_FUNCTIONS_DIR}/acm.instance0
	ln -s ${USB_FUNCTIONS_DIR}/acm.instance0 ${USB_CONFIGS_DIR}/acm.instance0

	# device
	echo 0xef > ${USB_DEVICE_DIR}/bDeviceClass
	echo 0x01 > ${USB_DEVICE_DIR}/bDeviceProtocol
	echo 0x02 > ${USB_DEVICE_DIR}/bDeviceSubClass
	echo 0x00 > ${USB_DEVICE_DIR}/bMaxPacketSize0
	echo 0x0419 > ${USB_DEVICE_DIR}/bcdDevice
	echo 0x0200 > ${USB_DEVICE_DIR}/bcdUSB
	echo 0x0102 > ${USB_DEVICE_DIR}/idProduct
	echo 0x1d6b > ${USB_DEVICE_DIR}/idVendor
	UDC=`ls /sys/class/udc/ | awk '{print $1}'`
	echo $UDC > ${USB_DEVICE_DIR}/UDC
}


if [ -e /dev/$TTY ]; then
	exit 1
fi

insmod_cdc_acm
init_cdc_acm_configfs
