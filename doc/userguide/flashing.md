Flashing the development boards {#page_flashing}
============

[TOC]

This page provides information on how to flash the used development boards in
different ways. The sections `How to build the software` and
`How to build the bitstream` of the several targets also provide information
on downloading to the hardware.

> **WARNING**: Wrong wiring of the boards or previously downloaded software
> which drives the I/O pins in a different manner can destroy the hardware.
> It is recommended to do the initial flash without the wiring, just with
> the power supply and the USB programming cable connected.

# Flash of the pre-built binaries with minimum software installed {#sect_minimum_flash}

For the download of the pre-built binaries at least the software requirements
in \ref sect_requirements_sw (1) have to be met.

## Altera/Nios2 {#sect_minimum_flash_altera}

1. Make sure the board is set up as described in \ref page_altsetuphw.
2. Power up the board and connect it via USB to the computer.
3. Open Quartus Programmer
  > Open the Programmer directly, if it is installed stand-alone or
  > open Quartus Software and start the Quartus Programmer from the menu
  `Tools->Programmer`.
4. In the Programmer, click the button `Hardware Setup...` and select `USB-Blaster [USB0]`,
from the list box and close the window.
5. Click the button `Add File...`, change into the openSAFETY_DEMO directory
/bin/fpga/boards/[board]/ and select the file openSAFETY_DEMO_[fpga].jic.
6. Now that the FPGA and the flash device are added, check the boxes `Program/Configure`
and `Verify` and click the button `Start`.
7. After a successful download, a power cycle has to be performed to start the
board. After a few seconds, an on or blinking LEDG0 on the FPGA board indicates
that the flash was successful and the PCP is running.
A weak glowing LEDG0 indicates that the flash was not successful.

## STM32 NUCLEO {#sect_minimum_flash_nucleo}

1. Ensure that the related ST-Link drivers are installed.
2. Make sure the board is / boards are set up as described in the section
`How to setup the board` according to the desired single channelled or dual
channelled application.
  - NUCLEO-F401RE: \ref page_stm32f401_setuphw
3. Power up the boards before you connect them via USB to the computer.
4. As the boards are recognised as USB drives, open the folder /bin/[microprocessor],
copy the desired .bin file and paste it onto the particular USB drive.
  - single channelled: demo-sn-gpio-single.bin
  - dual channelled:
    * demo-sn-gpio-upm.bin : for the processor acting as master
    * demo-sn-gpio-ups.bin : for the processor acting as slave

5. The download is finished, when the LED of the programming adapter of the
particular board hast stopped blinking.
6. Power cycle the boards (via power cycling the FPGA board).

# Flash of the pre-built binaries with cmake {#sect_flash_prebuilt_cmake}

If downloading the pre-built bitstream or software should be done with cmake,
the software requirements of \ref sect_requirements_sw (2) have to be met.

The pre-built binaries may be downloaded via the main CMakeLists.txt in the
root directory of the openSAFETY_DEMO, or by using a stripped down version
of the file located in the directory **bin**. The latter is only for flashing
the pre-built binaries, but needs no further configuration, though.

## Altera/Nios2 {#sect_minimum_flash_altera}

1. Make sure the board is set up as described in \ref page_altsetuphw.
2. Power up the board and connect it via USB to the computer.
3. Open the Nios II 13.0sp1 Command Shell and change to the openSAFETY_DEMO
directory.
4. Change into the directory **build**. Alternatively a new build directory may
be created. Change into the newly created directory.
5. Enter

        > cmake -G"Unix Makefiles"  -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-altera-nios2-gnu.cmake ../bin

 for using the stripped down version of the cmake file or

        > cmake -G"Unix Makefiles"  -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-altera-nios2-gnu.cmake ../

 for using the main cmake file. Refer to \ref page_altbuildhw and
 \ref page_altbuildsw for further information on the configuration options.

 Alternatively enter

        > cmake-gui&

 to open the CMake GUI, set source directory to the desired CMakeLists.txt file
 and also set the desired build directory and press
 `Configure`. If configuring for the first time the selected build directory,
 a prompt will ask to specify the generator for the project. Select
 `Unix Makefiles`
 from the list and `Specify toolchain for cross-compiling` and click `Next`.
 Choose the file `toolchain-altera-nios2-gnu.cmake` from the directory **cmake**
 and click `Finish`.
 Click `Configure` and then `Generate`.

6. On the command line type

        > make help

 to list the available make targets.

7. Type

        > make flash_default_fpga_image

 to write de pre-built bitstream and software to the FPGA board.

8. After a successful download, a power cycle has to be performed to start the
board. After a few seconds, an on or blinking LEDG0 on the FPGA board indicates
that the flash was successful and the PCP is running.
A weak glowing LEDG0 indicates that the flash was not successful.


## STM32 NUCLEO {#sect_minimum_flash_nucleo}

1. Ensure that the related ST-Link drivers are installed.
2. Make sure the board is / boards are set up as described in the section
`How to setup the board` according to the desired single channelled or dual
channelled application.
  - NUCLEO-F401RE: \ref page_stm32f401_setuphw
3. Power up the boards before you connect the board you want to flash via USB
 to the computer.
 > **Note** Make sure that only one Nucleo board is connected to the computer
 > during the software download.
4. Open a shell (or mysys shell) and change to the openSAFETY_DEMO
directory.
5. Change into the directory **build**. Alternatively a new build directory may
be created. Change into the newly created directory.
5. Enter

        > cmake -G"Unix Makefiles"  -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-arm-cortexm4-gnueabi.cmake ../bin

 for using the stripped down version of the cmake file or

        > cmake -G"Unix Makefiles"  -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-arm-cortexm4-gnueabi.cmake ../

 for using the main cmake file. Refer to the section `How to build the software`
 for further information on the configuration options.
 - NUCLEO-F401RE: \ref page_stm32f401_buildsw


 Alternatively enter

        > cmake-gui&

 to open the CMake GUI, set source directory to the desired CMakeLists.txt file
 and also set the desired build directory and press
 `Configure`. If configuring for the first time the selected build directory,
 a prompt will ask to specify the generator for the project. Select
 `Unix Makefiles`
 from the list and `Specify toolchain for cross-compiling` and click `Next`.
 Choose the file `toolchain-arm-cortexm4-gnueabi.cmake` from the directory **cmake**
 and click `Finish`.
 Click `Configure` and then `Generate`.

6. On the command line type

        > make help

 to list the available make targets.

7. Type

        > make flash-default-demo-sn-gpio-[FLAVOR].bin

 where FLAVOR can be `single, upm, or ups`, according to your hardware setup
 and desired Nucleo board to flash, to write de pre-built software to the Nucleo board.



# Flashing self-compiled binaries {#sect_flash_normal}

For information on how to download binaries which have been compiled with the
toolchains, refer to the sections `How to build the software` and / or
`How to build the bitstream` of the specific targets.
