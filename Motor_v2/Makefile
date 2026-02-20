ccflags-y += -I$(PWD)/../../../kernel/drivers/ifs/include

obj-m += motor_v2.o
motor_v2-objs := src/drv_motor_awinic.o src/drv_motor_gpio.o src/drv_motor_awd8833.o

KDIR := $(SDK_SRC_DIR)/kernel 
PWD := $(shell pwd)

MAKE=make
MKDIR=mkdir

all: build install

build:
	@echo "=================================================="
	@echo "make motor"
	@echo "=================================================="
	@make clean
	@mkdir -p driver
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules
	@mv motor_v2.ko driver
	
install:
	@cp driver/motor_v2.ko  $(VENDOR_ROOT_DIR)/ROOTFS/awinic/motor_v2.ko
	@echo ">>>>install motor_v2"
	@echo "=================================================="
	
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) clean
	@rm *.ko *.o *.mod.* *.cmd *.symvers *.order 2>/dev/null || true
	@rm -rf driver
	@rm -rf $(VENDOR_ROOT_DIR)/ROOTFS/awinic/motor_v2.ko
