# 編譯器與旗標
CC := arm-linux-gnueabihf-gcc
CHIP := ST25R3916
INTERFACE := RFAL_USE_I2C
MFLAGS := -Iinclude -Irfal/include -Iinc -lpthread -lrt -Wall -Wextra -O2
CFLAGS := -fPIC -Iinclude -Irfal/include -Iinc -Wall -Wextra -O2
LDFLAGS := -shared


# 檔案與資料夾
SRCDIR := rfal/source
APP_SRCDIR := src
APP_BINDIR := build/bin
OBJDIR := build/obj
LIBDIR := build/lib
TARGET := $(LIBDIR)/librfal.so


APP_SRC := $(wildcard $(APP_SRCDIR)/*.c) 
APP_BIN := $(APP_BINDIR)/pega_nfc

SOURCES := $(wildcard $(SRCDIR)/*.c)
OBJECTS := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SOURCES))

# ========== 主要目標 ==========
all: info clean build install

info :
	@echo "======================================"
	@echo " build : pega_nfc and librfal.so"
	@echo "======================================"
build : $(APP_BIN)

# App 需要 lib
$(APP_BIN): $(TARGET) | $(APP_BINDIR)
	@echo "[APP] Linking App"
	@$(CC) $(APP_SRC) $(MFLAGS) -D$(CHIP) -D$(INTERFACE) -L$(LIBDIR) -lrfal -o $@

# 獨立出 so 的建構目標
lib: $(TARGET)

# 建立 .so
$(TARGET): $(OBJECTS) | $(LIBDIR)
	@echo "[LIB] Building shared library: $@"
	@$(CC) $(LDFLAGS) -D$(CHIP) -D$(INTERFACE) -o $@ $^

# 建立 .o
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	@$(CC) $(CFLAGS) -D$(CHIP) -D$(INTERFACE) -c $< -o $@

# 目錄確保
$(OBJDIR) $(LIBDIR) $(APP_BINDIR):

	@mkdir -p $@


install:
	@cp -f ./src/init_gpiochip0.sh ./../build/bin
	@cp -f ./$(APP_BIN) ./../build/bin
	@cp -f ./$(TARGET) ./../../ROOTFS/lib

# 清理
clean:
	@rm -rf $(OBJDIR) $(LIBDIR) $(APP_BINDIR)

.PHONY: all clean lib
