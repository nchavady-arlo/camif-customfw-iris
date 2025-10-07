#!/bin/sh
declare -x ARCH="arm"
declare -x CROSS_COMPILE="arm-linux-gnueabihf-"
declare -x PATH=$(pwd)/../../../../../ToolChain/gcc-11.1.0-20250211-linaro-glibc-x86_64_arm-linux-gnueabihf/bin:$PATH
declare -x KERNELDIR=$(pwd)/../../../../kernel

