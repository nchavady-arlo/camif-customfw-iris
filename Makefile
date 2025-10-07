.PHONY: all wifi

all: wifi
	@make -C pegatron-cli
	@make -C led/MKD2061
	@make -C accelerator
	@make -C motor

wifi:
	@make -C wifi/WIFI-BT-LNX_6_12_3_RC1-IMX8--MM6X18505.p23_V2-GPL/wlan_src
	$(MAKE) -C $(KERNELDIR) M=$(PWD)/SourceCode/VENDOR/NXP/WIFI-BT-LNX_6_12_3_RC1-IMX8--MM6X18505.p23_V2-GPL/wlan_src ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules



clean:
	@make clean -C wifi/WIFI-BT-LNX_6_12_3_RC1-IMX8--MM6X18505.p23_V2-GPL/wlan_src
	@make -C wifi/WIFI-BT-LNX_6_12_3_RC1-IMX8--MM6X18505.p23_V2-GPL/wlan_src clean
	@make -C wifi/WIFI-BT-LNX_6_12_3_RC1-IMX8--MM6X18505.p23_V2-GPL/wlan_src/mapp/libcsi clean
	@make -C wifi/WIFI-BT-LNX_6_12_3_RC1-IMX8--MM6X18505.p23_V2-GPL/wlan_src/mapp/nanapp clean
	@make -C wifi/WIFI-BT-LNX_6_12_3_RC1-IMX8--MM6X18505.p23_V2-GPL/uartfwloader_src/linux clean all
	@make cleanall -C PEGA
	@make -C Kinetic/MKD2061 clean
	@make -C ST/Accelerator clean
	@make -C Motor clean

install:
	@make -C wifi/WIFI-BT-LNX_6_12_3_RC1-IMX8--MM6X18505.p23_V2-GPL/wlan_src install BINDIR=$(DESTDIR)/usr/lib/modules
	@make -C wifi/WIFI-BT-LNX_6_12_3_RC1-IMX8--MM6X18505.p23_V2-GPL/uartfwloader_src/linux install DESTDIR=$(DESTDIR)/usr/sbin/
