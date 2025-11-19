DEMO_DIR := ./ST25R3916_v2.8.0_Linux_demo_v1.0/linux_demo/build

include config

BUILD_PROG_HOME=$(shell pwd)
export BUILD_PROG_HOME

.PHONY: build clean all full package

ifeq (y,$(BUILD_IN))
all: build package
else
all:
endif

full:
ifeq (y,$(BUILD_IN))
	@$(MAKE) all
endif

build:
	@echo "===================================================================="
	@echo "Build NFC Sample Code"
	@echo "===================================================================="
	@rm -rf $(DEMO_DIR)
	@mkdir $(DEMO_DIR)
	
	@cd $(DEMO_DIR) && pwd && cmake -DBUILD_I2C=ON -DRFAL_VARIANT=st25r3916 -DRFAL_ANALOG_CONFIG_CUSTOM=ON -DCMAKE_TOOLCHAIN_FILE=../toolchain_arm-linux-gnueabihf.cmake ..
	@cd $(DEMO_DIR) && make
	
	@cp -f $(DEMO_DIR)/platform/st25r3916/librfal_st25r3916.so ./build/librfal_st25r3916.so
	@cp -f $(DEMO_DIR)/demo/nfc_demo_st25r3916 ./build/nfc_demo_st25r3916

	
package:
	@echo "===================================================================="
	@echo "Install NFC Demo"
	@echo "===================================================================="
	
	@cp -f ./build/init_gpiochip0.sh ./../build/bin
	@cp -f ./build/nfc_demo_st25r3916 ./../build/bin
	@cp -f ./build/librfal_st25r3916.so ./../../ROOTFS/lib

clean:
	rm -rf $(DEMO_DIR)

