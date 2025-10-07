#
# Tools makefile for Linux

OSTYPE=linux
CC =$(CROSS_COMPILE)gcc 
CONFIG_TEST_CODE=0
BUILD_PALLADIUM?=0

# If ARCH is defined, we are cross compiling, so activate SPI support
ifeq ($(ARCH),arm)
	CONFIG_SPI=1
	BUILD_SUFFIX=_imx_lnx
else
ifeq ($(BUILD_PALLADIUM),1)
	CONFIG_SPI=1
	BUILD_SUFFIX=_pdm_lnx
else
	CONFIG_SPI=0
	BUILD_SUFFIX=_linux
endif
endif

SRCDIR=../src
TESTDIR=../test
OBJDIR=build$(BUILD_SUFFIX)

FW_LOADER=fw_loader$(BUILD_SUFFIX)
CHIP_SIMULATOR_SPI=chip_simulator_spi$(BUILD_SUFFIX)


#CFLAGS=-g -O2 -Wall -Werror -I$(SRCDIR)
CFLAGS=-g -O2 -Wall -I$(SRCDIR)
ifeq ($(CONFIG_TEST_CODE),1)
	CFLAGS+=-DTEST_CODE
endif
ifeq ($(CONFIG_SPI),1)
	CFLAGS+=-DLINUX_SPI
endif
ifeq ($(BUILD_PALLADIUM),1)
	CFLAGS+=-DBUILD_PALLADIUM
endif

DEPFLAGS=-MT $@ -MMD -MP -MF $(OBJDIR)/$*.d

LDLIBS+=-lrt

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(DEPFLAGS) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.d: $(OBJDIR)/%.o

$(OBJDIR):
	mkdir -p $(OBJDIR)


FW_LOADER_OBJS=fw_loader_frames fw_loader_io_linux fw_loader_io_linux_uart fw_loader_uart
ifeq ($(CONFIG_SPI),1)
	FW_LOADER_OBJS+=fw_loader_io_linux_spi
endif
FW_LOADER_OBJS := $(addprefix $(OBJDIR)/, $(FW_LOADER_OBJS))
FW_LOADER_OBJS := $(addsuffix .o, $(FW_LOADER_OBJS))


$(FW_LOADER): $(FW_LOADER_OBJS)
	echo "Luke"
	echo $(LDLIBS)
	$(CC) $(LFLAGS) $(FW_LOADER_OBJS) $(LDLIBS) -o $@

$(FW_LOADER_OBJS): | $(OBJDIR)

#$(FW_LOADER): $(FW_LOADER_OBJS) Makefile
#	echo "Luke Done"

$(OBJDIR)/%.o: $(TESTDIR)/%.c
	$(CC) $(DEPFLAGS) $(CFLAGS) -c $< -o $@


CHIP_SIMULATOR_SPI_OBJS=fw_loader_frames fw_loader_io_linux chip_simulator_gpio chip_simulator_timer chip_simulator_spi  chip_simulator_main
CHIP_SIMULATOR_SPI_OBJS := $(addprefix $(OBJDIR)/, $(CHIP_SIMULATOR_SPI_OBJS))
CHIP_SIMULATOR_SPI_OBJS := $(addsuffix .o, $(CHIP_SIMULATOR_SPI_OBJS))


$(CHIP_SIMULATOR_SPI): $(CHIP_SIMULATOR_SPI_OBJS)
	$(CC) $(LFLAGS) $(CHIP_SIMULATOR_SPI_OBJS) $(LDLIBS) -o $@

$(CHIP_SIMULATOR_SPI_OBJS): | $(OBJDIR)

$(CHIP_SIMULATOR_SPI): $(CHIP_SIMULATOR_SPI_OBJS) Makefile

.PHONY: help
help:
	@echo "Compilation for Linux:"
	@echo "    make clean all"
	@echo "Compilation for Palladium (PC Linux with SPI):"
	@echo "    BUILD_PALLADIUM=1 make clean all"
	@echo "Compilation for Imx:"
	@echo "    . /opt/fsl-imx-xwayland/5.10-hardknott/environment-setup-cortexa53-crypto-poky-linux"
	@echo "    make clean all"


.PHONY: clean
clean:
	rm -f $(FW_LOADER) $(CHIP_SIMULATOR_SPI)
	rm -rf $(OBJDIR)
	@echo "FW_LOADER_OBJS:       $(FW_LOADER_OBJS)"
	@echo "CHIP_SIMULATOR_SPI_OBJS: $(CHIP_SIMULATOR_SPI_OBJS)"

install: 
	cp -f chip_simulator_spi_imx_lnx $(DESTDIR)
	cp -f fw_loader_imx_lnx $(DESTDIR)
	echo "NXP BT utility Installed"

.PHONY: all
ifeq ($(ARCH),arm)
all: $(FW_LOADER) $(CHIP_SIMULATOR_SPI)
else
all: $(FW_LOADER) 
endif


