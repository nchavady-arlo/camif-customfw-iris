LOCAL_PATH := $(my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := fw_loader
OBJS = ../src/fw_loader_uart.c ../src/fw_loader_io_linux.c
LOCAL_SRC_FILES := $(OBJS)
LOCAL_CFLAGS=-g -O2 -I../src/
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)
