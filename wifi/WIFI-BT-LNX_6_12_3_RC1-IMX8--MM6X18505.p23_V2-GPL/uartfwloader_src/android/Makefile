#
# Macro for NDK path
NDK_PATH := /usr/local/arm/android-ndk-r23-beta5

# Declare Variables
OSTYPE := android
CC := $(NDK_PATH)/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android31-clang
BUILD_SUFFIX := _imx_android
SRCDIR := ../src
OBJDIR := build$(BUILD_SUFFIX)
FW_LOADER := fw_loader$(BUILD_SUFFIX)

# Compiler and linker flags
CFLAGS=-g -O2 -Wall -I$(SRCDIR)
DEPFLAGS=-MT $@ -MMD -MP -MF $(OBJDIR)/$*.d

# List of object files for fw_loader
FW_LOADER_OBJS=fw_loader_frames fw_loader_io_linux fw_loader_io_linux_uart fw_loader_uart
 
# Targets
.PHONY: all clean help

all: $(FW_LOADER)

# Rule to compile Object file
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(DEPFLAGS) $(CFLAGS) -c $< -o $@
$(OBJDIR)/%.d: $(OBJDIR)/%.o

# Rule to create build directory.
$(OBJDIR):
	mkdir -p $(OBJDIR)

#Convert source files to correspoding object files in OBJDIR
FW_LOADER_OBJS := $(addprefix $(OBJDIR)/, $(FW_LOADER_OBJS))
FW_LOADER_OBJS := $(addsuffix .o, $(FW_LOADER_OBJS))

#Target to build fw_loader binary
$(FW_LOADER): $(FW_LOADER_OBJS)
	$(CC) $(LFLAGS) $(FW_LOADER_OBJS) $(LDLIBS) -o $@

#Target Depedancies
$(FW_LOADER_OBJS): | $(OBJDIR)
$(FW_LOADER): $(FW_LOADER_OBJS) Makefile

clean:
	rm -f $(FW_LOADER)
	rm -rf $(OBJDIR)
	@echo "FW_LOADER_OBJS:       $(FW_LOADER_OBJS)"
 
help:
	@echo "Compilation for Imx:android"
	@echo "make clean all"
	@echo ".$(CC)"
