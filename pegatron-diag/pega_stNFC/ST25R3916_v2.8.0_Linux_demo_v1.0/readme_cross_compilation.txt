Raspberry Pi Cross-compilation on Windows:
==========================================

Install cmake for Windows (https://cmake.org)

Install cross-compilation toolchain (http://gnutoolchains.com/raspberry)

Use the cross compilation file toolchain_arm-linux-gnueabihf.cmake, you may need to update the CMAKE_BUILD_RPATH with the path to the right RFAL library.


In the Git Bash shell, configure cmake, by specifying the toolchain file on the command line:

$ mkdir -p linux_demo/build
$ cd linux_demo/build
$ cmake -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=./toolchain_arm-linux-gnueabihf.cmake ..
-- The C compiler identification is GNU 6.3.0
-- The CXX compiler identification is GNU 6.3.0
-- Check for working C compiler: C:/SysGCC/raspberry/bin/arm-linux-gnueabihf-gcc.exe
-- Check for working C compiler: C:/SysGCC/raspberry/bin/arm-linux-gnueabihf-gcc.exe -- works
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Detecting C compile features
-- Detecting C compile features - done
-- Check for working CXX compiler: C:/SysGCC/raspberry/bin/arm-linux-gnueabihf-c++.exe
-- Check for working CXX compiler: C:/SysGCC/raspberry/bin/arm-linux-gnueabihf-c++.exe -- works
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Configuring done
-- Generating done
-- Build files have been written to: linux_demo/build


Then build:

$ make
Scanning dependencies of target rfal_st25r3916
$ make
Scanning dependencies of target rfal_st25r3916
[  2%] Building C object platform/st25r3916/CMakeFiles/rfal_st25r3916.dir/C_/repositories/ST25R3911_linux/rfal/source/rfal_analogConfig.c.o
[  5%] Building C object platform/st25r3916/CMakeFiles/rfal_st25r3916.dir/C_/repositories/ST25R3911_linux/rfal/source/rfal_cd.c.o
[  8%] Building C object platform/st25r3916/CMakeFiles/rfal_st25r3916.dir/C_/repositories/ST25R3911_linux/rfal/source/rfal_cdHb.c.o
[ 11%] Building C object platform/st25r3916/CMakeFiles/rfal_st25r3916.dir/C_/repositories/ST25R3911_linux/rfal/source/rfal_crc.c.o
[ 14%] Building C object platform/st25r3916/CMakeFiles/rfal_st25r3916.dir/C_/repositories/ST25R3911_linux/rfal/source/rfal_demo.c.o
[ 17%] Building C object platform/st25r3916/CMakeFiles/rfal_st25r3916.dir/C_/repositories/ST25R3911_linux/rfal/source/rfal_dpo.c.o
[ 20%] Building C object platform/st25r3916/CMakeFiles/rfal_st25r3916.dir/C_/repositories/ST25R3911_linux/rfal/source/rfal_iso15693_2.c.o
[ 22%] Building C object platform/st25r3916/CMakeFiles/rfal_st25r3916.dir/C_/repositories/ST25R3911_linux/rfal/source/rfal_isoDep.c.o
[ 25%] Building C object platform/st25r3916/CMakeFiles/rfal_st25r3916.dir/C_/repositories/ST25R3911_linux/rfal/source/rfal_nfc.c.o
[ 28%] Building C object platform/st25r3916/CMakeFiles/rfal_st25r3916.dir/C_/repositories/ST25R3911_linux/rfal/source/rfal_nfcDep.c.o
[ 31%] Building C object platform/st25r3916/CMakeFiles/rfal_st25r3916.dir/C_/repositories/ST25R3911_linux/rfal/source/rfal_nfca.c.o
[ 34%] Building C object platform/st25r3916/CMakeFiles/rfal_st25r3916.dir/C_/repositories/ST25R3911_linux/rfal/source/rfal_nfcb.c.o
[ 37%] Building C object platform/st25r3916/CMakeFiles/rfal_st25r3916.dir/C_/repositories/ST25R3911_linux/rfal/source/rfal_nfcf.c.o
[ 40%] Building C object platform/st25r3916/CMakeFiles/rfal_st25r3916.dir/C_/repositories/ST25R3911_linux/rfal/source/rfal_nfcv.c.o
[ 42%] Building C object platform/st25r3916/CMakeFiles/rfal_st25r3916.dir/C_/repositories/ST25R3911_linux/rfal/source/rfal_st25tb.c.o
[ 45%] Building C object platform/st25r3916/CMakeFiles/rfal_st25r3916.dir/C_/repositories/ST25R3911_linux/rfal/source/rfal_st25xv.c.o
[ 48%] Building C object platform/st25r3916/CMakeFiles/rfal_st25r3916.dir/C_/repositories/ST25R3911_linux/rfal/source/rfal_t1t.c.o
[ 51%] Building C object platform/st25r3916/CMakeFiles/rfal_st25r3916.dir/C_/repositories/ST25R3911_linux/rfal/source/rfal_t2t.c.o
[ 54%] Building C object platform/st25r3916/CMakeFiles/rfal_st25r3916.dir/C_/repositories/ST25R3911_linux/rfal/source/rfal_t4t.c.o
[ 57%] Building C object platform/st25r3916/CMakeFiles/rfal_st25r3916.dir/C_/repositories/ST25R3911_linux/rfal/source/st25r3916/rfal_rfst25r3916.c.o
[ 60%] Building C object platform/st25r3916/CMakeFiles/rfal_st25r3916.dir/C_/repositories/ST25R3911_linux/rfal/source/st25r3916/st25r3916.c.o
[ 62%] Building C object platform/st25r3916/CMakeFiles/rfal_st25r3916.dir/C_/repositories/ST25R3911_linux/rfal/source/st25r3916/st25r3916_aat.c.o
[ 65%] Building C object platform/st25r3916/CMakeFiles/rfal_st25r3916.dir/C_/repositories/ST25R3911_linux/rfal/source/st25r3916/st25r3916_com.c.o
[ 68%] Building C object platform/st25r3916/CMakeFiles/rfal_st25r3916.dir/C_/repositories/ST25R3911_linux/rfal/source/st25r3916/st25r3916_irq.c.o
[ 71%] Building C object platform/st25r3916/CMakeFiles/rfal_st25r3916.dir/C_/repositories/ST25R3911_linux/rfal/source/st25r3916/st25r3916_led.c.o
[ 74%] Building C object platform/st25r3916/CMakeFiles/rfal_st25r3916.dir/__/Src/pltf_gpio.c.o
[ 77%] Building C object platform/st25r3916/CMakeFiles/rfal_st25r3916.dir/__/Src/pltf_timer.c.o
[ 80%] Building C object platform/st25r3916/CMakeFiles/rfal_st25r3916.dir/__/Src/pltf_spi.c.o
[ 82%] Linking C shared library librfal_st25r3916.so
[ 82%] Built target rfal_st25r3916
Scanning dependencies of target nfc_demo_st25r3916
[ 85%] Building C object demo/CMakeFiles/nfc_demo_st25r3916.dir/Src/logger.c.o
[ 88%] Building C object demo/CMakeFiles/nfc_demo_st25r3916.dir/Src/main.c.o
[ 91%] Building C object demo/CMakeFiles/nfc_demo_st25r3916.dir/Src/rfal_analog_config_custom.c.o
[ 94%] Building C object demo/CMakeFiles/nfc_demo_st25r3916.dir/C_/repositories/ST25R3911_linux/st25r_demos/Src/demo_polling.c.o
[ 97%] Building C object demo/CMakeFiles/nfc_demo_st25r3916.dir/C_/repositories/ST25R3911_linux/st25r_demos/Src/demo_ce.c.o
[100%] Linking C executable nfc_demo_st25r3916
[100%] Built target nfc_demo_st25r3916



Start WinSCP and connect to the Raspberry Pi to transfer the file, or use a USB stick

$ chmod +x nfc_demo_<variant>
$ sudo ./demo/nfc_demo_<variant>

Pay attention to the RPATH value set with CMAKE_BUILD_RPATH, check it with /c/SysGCC/raspberry/bin/arm-linux-gnueabihf-readelf.exe -d demo/nfc_demo_st25r3916


Debugging booster:
==================
$ cmake -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=./toolchain_arm-linux-gnueabihf.cmake -DCMAKE_BUILD_TYPE=Debug ..
$ make
sudo gdbtui ./demo/nfc_demo_<variant>
or
sudo gdbtui ./demo/nfc_demo_<variant>_CE

To set a breakpoint:
b <function>
b <filename>:<linenumber>
r to run
s to step into the code (go down into the function)
up to go one level up
n to go to next line
bt to show backtrace
CTRL-C to break
c to continue
info b to list the breakpoints
del n to delete breakpoint
