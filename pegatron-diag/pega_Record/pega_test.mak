TARGET_NAME	= pega_record

CFLAGS  += -Isrc -I$(PRJDIR)/release/include -I$(KERNELDIR)/drivers/ifs/include
LDDEPS  = -Lsrc \
		  -L$(SDK_SRC_DIR)/project/release/chip/iford/ipc/common/glibc/11.1.0/release/mi_libs/static \
          -L$(SDK_SRC_DIR)/project/release/chip/iford/sigma_common_libs/glibc/11.1.0/release/static \
		  -lBF100_LINUX -lSSL_2MIC_LINUX  -lmi_ai -lmi_ao -lmi_common -lmi_sys -lcam_os_wrapper  -lm -lpthread -lcam_fs_wrapper

SRC			= \
	src/main.c \

include $(BUILD_ROOT_DIR)/make/makefile.mk
