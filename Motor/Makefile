obj-m += motor.o

motor-objs := src/motor.o src/pega_gpio.o src/pega_motor_awd8833.o
KDIR := $(SDK_SRC_DIR)/kernel 
PWD := $(shell pwd)

MAKE=make
MKDIR=mkdir

EXTRA_CFLAGS += -Isrc

all: build install

build:
	@echo "=================================================="
	@echo "make motor"
	@echo "=================================================="
	@make clean
	@mkdir -p driver
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- modules
	@mv motor.ko driver
	
install:
	@cp driver/motor.ko  $(VENDOR_ROOT_DIR)/ROOTFS/awinic/motor.ko
	@cp -f   src/control_PWM_sysfs.sh  ./../../PEGA/build/bin
	@echo ">>>>install motor"
	@echo "=================================================="
	
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- clean
	@rm *.ko *.o *.mod.* *.cmd *.symvers *.order 2>/dev/null || true
	@rm -rf driver
	@rm -rf $(VENDOR_ROOT_DIR)/ROOTFS/awinic/motor.ko
