# Cross compilation file for the Raspberry Pi

# Specify the target OS name
set(CMAKE_SYSTEM_NAME Linux)

# Specify the archiving tool for static libraries
#set(CMAKE_AR          C:/SysGCC/raspberry/bin/arm-linux-gnueabihf-ar.exe)
set(CMAKE_AR	$ENV{GCC_PATH}/bin/arm-linux-gnueabihf-11.1.0-ar)

# Specify the Cross Compiler
#set(CMAKE_C_COMPILER  C:/SysGCC/raspberry/bin/arm-linux-gnueabihf-gcc.exe)
set(CMAKE_C_COMPILER	$ENV{GCC_PATH}/bin/arm-linux-gnueabihf-11.1.0-gcc)

# Specify sysroot path
#set(CMAKE_SYSROOT     C:/SysGCC/raspberry/arm-linux-gnueabihf/sysroot)
set(CMAKE_SYSROOT	$ENV{GCC_PATH}/arm-linux-gnueabihf/libc)

# Specify RPATH to locate .so path on the target
set(CMAKE_BUILD_RPATH rfal/${RFAL_VARIANT})
