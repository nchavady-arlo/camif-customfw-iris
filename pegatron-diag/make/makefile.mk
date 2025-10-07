include config
include $(BUILD_ROOT_DIR)/make/config.mk
include $(BUILD_ROOT_DIR)/make/tools.mk

ifndef verbose
	SILENT = @
endif

ifeq (Debug, $(BUILD_MODE))
	DEFINES		+= -DDEBUG
else
	DEFINES		+= -DRELEASE
endif

ifneq (, $(VERSION))
	DEFINES		+= -DVERSION=\"$(VERSION)\"
endif

ifeq (.so, $(suffix $(TARGET_NAME)))
	CFLAGS		+= -fPIC
endif

ifeq (Debug, $(BUILD_MODE))
	CFLAGS		+= -g
else
	CFLAGS		+= -g -O2 -Werror
endif
CFLAGS		+= -Wno-psabi $(DEFINES)
CXXFLAGS	+= $(CFLAGS)
LDFLAGS		+= $(LDDEPS) -L$(LIB_DIR) -lm
ARFLAGS		= -crsv
OBJECTS		= $(SRC:.c=.o)

ifeq (".a", "$(suffix $(TARGET_NAME))")
	OUTPUT_TARGET_DIR = $(OUTPUT_LIB_PATH)
else ifeq (".so", "$(suffix $(TARGET_NAME))")
	OUTPUT_TARGET_DIR = $(OUTPUT_LIB_PATH)
else
	OUTPUT_TARGET_DIR = $(OUTPUT_BIN_PATH)
endif
OUTPUT_TARGET_FILE    = $(OUTPUT_TARGET_DIR)/$(TARGET_NAME)

all: info objs
	@if [ ! -d $(OUTPUT_TARGET_DIR) ]; then \
		$(MKDIR) -p $(OUTPUT_TARGET_DIR);	\
	fi

	@echo "Generating $(notdir $(OUTPUT_TARGET_FILE))"
ifeq (.a, $(suffix $(TARGET_NAME)))
	@$(AR) $(ARFLAGS) $(OUTPUT_TARGET_FILE) $(OBJECTS)
else ifeq (.so, $(suffix $(TARGET_NAME)))
	@$(CC) -shared -o $(OUTPUT_TARGET_FILE) $(OBJECTS)
else
	@$(CC) -o $(OUTPUT_TARGET_FILE) $(OBJECTS) $(LIBS) $(LDFLAGS)
endif

ifneq (.a, $(suffix $(TARGET_NAME)))
ifneq (.so, $(suffix $(TARGET_NAME)))
	@$(CP) $(OUTPUT_TARGET_FILE) $(dir $(OUTPUT_TARGET_FILE))/unstripped_$(notdir $(OUTPUT_TARGET_FILE))
endif
ifeq (Release, $(BUILD_MODE))
	@$(STRIP) --strip-unneeded $(OUTPUT_TARGET_FILE)
else ifeq (Release, $(BINARY_MODE))
	@$(STRIP) --strip-unneeded $(OUTPUT_TARGET_FILE)
endif
ifneq (.so, $(suffix $(TARGET_NAME)))
	@echo "Copying $(notdir $(OUTPUT_TARGET_FILE)) to $(BUILD_ROOT_DIR)/$(GEN_DIR)/bin"
	@$(CP) $(OUTPUT_TARGET_FILE) $(BUILD_ROOT_DIR)/$(GEN_DIR)/bin
else
	@echo "Copying $(notdir $(OUTPUT_TARGET_FILE)) to $(BUILD_ROOT_DIR)/$(GEN_DIR)/lib"
	@$(CP) $(OUTPUT_TARGET_FILE) $(BUILD_ROOT_DIR)/$(GEN_DIR)/lib
endif
else
	@echo "Copying $(notdir $(OUTPUT_TARGET_FILE)) to $(BUILD_ROOT_DIR)/$(GEN_DIR)/lib"
	@$(CP) $(OUTPUT_TARGET_FILE) $(BUILD_ROOT_DIR)/$(GEN_DIR)/lib
endif
	@echo ""

info:
	@echo "====================================="
	@echo "  TARGET : $(TARGET_NAME)"
	@echo "  CC     : $(CC)"
	@echo "  BUILD  : $(BUILD_MODE)"
ifneq "$(VERSION)" ""
	@echo "  VERSION: $(VERSION)"
endif
	@echo "====================================="

objs: $(OBJECTS)

%.o: %.c
	@echo "Compiling $@"
	@$(CC) $(INCLUDE) $(CXXFLAGS) -c $< -o $@

clean:
	@$(RM) -f $(OBJECTS) $(OUTPUT_TARGET_FILE)

clean_obj:
	@$(RM) -f $(OBJECTS)
