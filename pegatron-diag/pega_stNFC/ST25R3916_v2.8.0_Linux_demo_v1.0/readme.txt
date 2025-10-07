Raspberry Pi setup:
===================

How to build:
=============

Check that the correct RFAL variant is set in CMakeLists.txt, e.g.:
    set(RFAL_VARIANT "st25r3916" CACHE STRING "Select the RFAL library")

Then

Go to the build directory
$ cd linux_demo/build

Choose the demo to run:
  1. For the Reader/Writer demo:
  $ cmake ..

    or to build for another variant:
    $ cmake -DRFAL_VARIANT=<variant> ..
    
    or to build for I2C:
    $ cmake -DBUILD_I2C=ON ..

  2. For the Card Emulation-only demo:
  $ cmake -DCARD_EMULATION_ONLY=ON ..

Then compile it:
$ make


= I2C communication =

To use the I2C communication:
- Edit the Raspberry Pi Configuration, go the interfaces pane:
  enable the I2C interface (disable the SPI and UART interfaces).

- Edit /boot/config.txt and make sure the following parameters are set:
  # Set a fixed core frequency to avoid frequency changes depending on processor load, I2C clock is derived from this one.
  core_freq=250

  # Set Fast mode I2C (400kHz)
  dtparam=i2c_arm=on,i2x_arm_baudrate=400000



How to run from the build folder:
=================================

Go to the build folder
$ cd linux_demo/build
$ sudo ./demo/nfc_demo_<variant>
or
$ sudo ./demo/nfc_demo_<variant>_CE
Welcome to the ST25R NFC Demo on Linux.
Scanning for NFC technologies...



Note on the Linux SPI driver:
=============================
In case the demo cannot start, the error message may be:
Welcome to the ST25R NFC Demo on Linux.
Scanning for NFC technologies...
Failed to get line event for GPIO pin7 (ret -1)
GPIO init failed

This happens with latest Linux kernel and SPI driver (e.g $ modinfo spidev -> v5.5.61)
because by default, the driver controls GPIO8 for SPI0 CS0 and GPIO7 for SPI0 CS1.
There is a conflict with the demo that uses ST25R_INT_PIN on GPIO7 as interrupt line between
the reader and the microcontroller.

To overcome this, configure the SPI driver to not control GPIO7 for the Chip Select signal.
This can be done with the dtoverlay command.

$ dtoverlay -a ; to list all overlay available

$ dtoverlay -h spi0-1cs
Name:   spi0-1cs

Info:   Only use one CS pin for SPI0

Usage:  dtoverlay=spi0-1cs,<param>=<val>

Params: cs0_pin                 GPIO pin for CS0 (default 8)
        no_miso                 Don't claim and use the MISO pin (9), freeing
                                it for other uses.


Edit the boot configuration to use this overlay, add the following line:
$ sudo /boot/config.txt
dtoverlay=spi0-1cs
$ reboot

The parameter cs0_pin may be useful in case the driver assigns CE0 on another pin.
e.g. dtoverlay spi0-1cs,cs0_pin=6


Inspect GPIO configuration
==========================

See http://wiringpi.com/ and use their convenient tool to inspect the GPIO:
$ gpio readall

 +-----+-----+---------+------+---+---Pi 4B--+---+------+---------+-----+-----+
 | BCM | wPi |   Name  | Mode | V | Physical | V | Mode | Name    | wPi | BCM |
 +-----+-----+---------+------+---+----++----+---+------+---------+-----+-----+
 |     |     |    3.3v |      |   |  1 || 2  |   |      | 5v      |     |     |
 |   2 |   8 |   SDA.1 |   IN | 1 |  3 || 4  |   |      | 5v      |     |     |
 |   3 |   9 |   SCL.1 |   IN | 1 |  5 || 6  |   |      | 0v      |     |     |
 |   4 |   7 | GPIO. 7 |   IN | 1 |  7 || 8  | 1 | IN   | TxD     | 15  | 14  |
 |     |     |      0v |      |   |  9 || 10 | 1 | IN   | RxD     | 16  | 15  |
 |  17 |   0 | GPIO. 0 |   IN | 0 | 11 || 12 | 0 | IN   | GPIO. 1 | 1   | 18  |
 |  27 |   2 | GPIO. 2 |   IN | 0 | 13 || 14 |   |      | 0v      |     |     |
 |  22 |   3 | GPIO. 3 |   IN | 0 | 15 || 16 | 0 | IN   | GPIO. 4 | 4   | 23  |
 |     |     |    3.3v |      |   | 17 || 18 | 0 | IN   | GPIO. 5 | 5   | 24  |
 |  10 |  12 |    MOSI | ALT0 | 0 | 19 || 20 |   |      | 0v      |     |     |
 |   9 |  13 |    MISO | ALT0 | 0 | 21 || 22 | 0 | IN   | GPIO. 6 | 6   | 25  |
 |  11 |  14 |    SCLK | ALT0 | 0 | 23 || 24 | 1 | OUT  | CE0     | 10  | 8   |
 |     |     |      0v |      |   | 25 || 26 | 0 | IN   | CE1     | 11  | 7   | // Check that 'CE1' GPIO pin 7 shows direction 'IN', to receive interrupt signal
 |   0 |  30 |   SDA.0 |   IN | 1 | 27 || 28 | 1 | IN   | SCL.0   | 31  | 1   |
 |   5 |  21 | GPIO.21 |   IN | 1 | 29 || 30 |   |      | 0v      |     |     |
 |   6 |  22 | GPIO.22 |   IN | 1 | 31 || 32 | 0 | IN   | GPIO.26 | 26  | 12  |
 |  13 |  23 | GPIO.23 |   IN | 0 | 33 || 34 |   |      | 0v      |     |     |
 |  19 |  24 | GPIO.24 |   IN | 0 | 35 || 36 | 0 | IN   | GPIO.27 | 27  | 16  |
 |  26 |  25 | GPIO.25 |   IN | 0 | 37 || 38 | 0 | IN   | GPIO.28 | 28  | 20  |
 |     |     |      0v |      |   | 39 || 40 | 1 | IN   | GPIO.29 | 29  | 21  |
 +-----+-----+---------+------+---+----++----+---+------+---------+-----+-----+
 | BCM | wPi |   Name  | Mode | V | Physical | V | Mode | Name    | wPi | BCM |
 +-----+-----+---------+------+---+---Pi 4B--+---+------+---------+-----+-----+


Debugging booster:
==================
Turn on symbol debugging with
$ cmake -DCMAKE_BUILD_TYPE=Debug ..
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
