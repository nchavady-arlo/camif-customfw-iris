# 編譯器與旗標
CC := arm-linux-gnueabihf-gcc
CHIP := ST25R3916
INTERFACE := RFAL_USE_I2C
CFLAGS := -fPIC -Isrc -INFC/rfal/include -INFC/src -Wall -Wextra -O2
LDFLAGS := -shared


# 檔案與資料夾
SRCDIR := NFC/rfal/source
OBJDIR := build/obj
LIBDIR := build/lib
LIB := $(LIBDIR)/librfal.so

SOURCES := $(wildcard $(SRCDIR)/*.c)
OBJECTS := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SOURCES))

# ========== 主要目標 ==========
all: info clean build 

info :
	@echo "======================================"
	@echo " build : librfal.so"
	@echo "======================================"
build :	$(LIB)
	@cp -f ./$(LIB) ./../../ROOTFS/lib

# 建立 .so
$(LIB): $(OBJECTS) | $(LIBDIR)
	@echo "[LIB] Building shared library: $@"
	@$(CC) $(LDFLAGS) -D$(CHIP) -D$(INTERFACE) -o $@ $^

# 建立 .o
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	@$(CC) $(CFLAGS) -D$(CHIP) -D$(INTERFACE) -c $< -o $@

# 目錄確保
$(OBJDIR) $(LIBDIR):
	@mkdir -p $@

# 清理
clean:
	@rm -rf $(OBJDIR) $(LIBDIR) $(APP_BINDIR)

.PHONY: all clean lib
