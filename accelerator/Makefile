obj-m += st_lis2dw12_i2c.o

st_lis2dw12_i2c-objs := src/st_lis2dw12_i2c.o src/st_lis2dw12_core.o src/st_lis2dw12_buffer.o src/st_lis2dw12_events.o
#KDIR := $(SDK_SRC_DIR)/kernel
#PWD := $(shell pwd)
#GEN_DIR = Build
#
#EXTRA_CFLAGS += -Isrc
#
#MAKE=make
#MKDIR=mkdir
#
#all: build install
#
#build:
#	@echo "=================================================="
#	@echo "make lis2dw12"
#	@echo "=================================================="
#	@make clean
#	@mkdir -p driver
#	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- modules
#	@mv st_lis2dw12_i2c.ko driver
#	
#install:
#	@cp driver/st_lis2dw12_i2c.ko  $(VENDOR_ROOT_DIR)/ROOTFS/st/lis2dw12/driver
#	@echo ">>>>install lis2dw12"
#	@echo "=================================================="
#	
#clean:
#	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- clean
#	@rm *.ko *.o *.mod.* *.cmd *.symvers *.order 2>/dev/null || true
#	@rm -rf driver
#
