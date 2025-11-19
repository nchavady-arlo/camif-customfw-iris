obj-m += leds-aw2026.o

leds-aw2026-objs := src/leds-aw2026.o
KDIR := $(SDK_SRC_DIR)/kernel 
PWD := $(shell pwd)

MAKE=make
MKDIR=mkdir

EXTRA_CFLAGS += -Isrc

all: build install

build:
	@echo "=================================================="
	@echo "make AW2026"
	@echo "=================================================="
	@make clean
	@mkdir -p driver
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- modules
	@mv leds-aw2026.ko driver
	
install:
	@cp driver/leds-aw2026.ko  $(VENDOR_ROOT_DIR)/ROOTFS/awinic/leds-aw2026.ko
	@echo ">>>>install leds-aw2026"
	@echo "=================================================="
	
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- clean
	@rm *.ko *.o *.mod.* *.cmd *.symvers *.order 2>/dev/null || true
	@rm -rf driver
	@rm -rf $(VENDOR_ROOT_DIR)/ROOTFS/awinic/leds-aw2026.ko
