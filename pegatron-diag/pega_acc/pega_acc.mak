TARGET_NAME	= pega_acc


LDDEPS  = -lpthread -lrt

SRC	=	\
	src/main.c 		

include $(BUILD_ROOT_DIR)/make/makefile.mk
