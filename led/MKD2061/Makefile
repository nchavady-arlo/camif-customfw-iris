obj-m += leds-ktd2061.o
leds-ktd2061-objs := led_src/leds-ktd2061.o

#KDIR := $(SDK_SRC_DIR)/kernel
#PWD := $(shell pwd)
#GEN_DIR = Build
#
#MAKE=make
#MKDIR=mkdir
#
#all: build install
#
#build:
#	@echo "=================================================="
#	@echo "make leds-ktd2061"
#	@echo "=================================================="
#	@make clean
#	@mkdir -p driver
#	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- modules
#	@mv leds-ktd2061.ko driver
#	
#install:
#	@cp driver/leds-ktd2061.ko  $(VENDOR_ROOT_DIR)/ROOTFS/kinetic/driver
#	@echo ">>>>install leds-ktd2061"
#	@echo "=================================================="
#	
#clean:
#	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- clean
#	@rm *.ko *.o *.mod.* *.cmd *.symvers *.order 2>/dev/null || true
#	@rm -rf driver
