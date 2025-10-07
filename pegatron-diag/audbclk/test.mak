TARGET_NAME	= audblk
LDDEPS =-lmi_ai -lpthread -lmi_sys -lcam_os_wrapper -lmi_common  

LIBS		+=	-L$(SDK_SRC_DIR)/project/release/chip/iford/ipc/common/glibc/11.1.0/release/mi_libs/static \
			-L$(SDK_SRC_DIR)/project/release/chip/iford/sigma_common_libs/glibc/11.1.0/release/static

SRC			= \
	src/main.c
	
	
INCLUDE		+= -I$(SDK_SRC_DIR)/project/release/include

include $(BUILD_ROOT_DIR)/make/makefile.mk
