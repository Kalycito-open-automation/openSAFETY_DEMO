Downloading the FPGA bitstream and software {#page_download_fpga}
============

This page provides information about downloading the FPGA bitstream
and executable.

\warning Wrong wiring of the boards or previously downloaded software
which drives the I/O pins in a different manner can destroy the hardware.
In case of doubt, do the initial flash without the wiring, just with
the power supply and the USB programming cable connected.

[TOC]

# Erasing the Flash {#sect_gs_erase_fpga_flash}

The flash memory of the FPGA board may be erased to prevent mixup between
FPGA configuration in volatile and non-volatile memory or to ensure a safe state
of the output pins of the board prior connecting the Nucleo boards.

To erase the flash memory with the `Quartus Programmer`, carry out the following
steps:

1. Make sure the board is set up as described in \ref sect_gs_fpgaboard_setup.
2. Power up the board and connect it via USB to the computer.
3. Open Quartus Programmer
  > Open the Programmer directly, if it is installed stand-alone or
  > open Quartus Software and start the Quartus Programmer from the menu
  > `Tools->Programmer`.
4. If `No Hardware` is shown in the top left area, click the button
   `Hardware Setup...` and select `USB-Blaster [USB0]`,
   from the list box and close the window.
5. Click the button `Auto Detect` and select the device `EP4CE115`
6. Right-click onto the added device and select `Attach Flash Device...`,
   select ASC devices and the device with the name `EPCS64` and click `OK`.
7. Check the boxes `Erase` and `Blank-Check` of the added flash device.
8. Click the `Start` button to erase the Flash of the FPGA board.

# Flashing the pre-built binaries {#sect_gs_flash_fpga_default_image}

## Using the Quartus Programmer {#sect_gs_flash_fpga_default_image_quartus}

1. Make sure the board is set up as described in \ref sect_gs_fpgaboard_setup.
2. Power up the board and connect it via USB to the computer.
3. Open Quartus Programmer
  > Open the Programmer directly, if it is installed stand-alone or
  > open Quartus Software and start the Quartus Programmer from the menu
  > `Tools->Programmer`.
4. If `No Hardware` is shown in the top left area, click the button
   `Hardware Setup...` and select `USB-Blaster [USB0]`,
   from the list box and close the window.
5. Click the button `Add File...`, change into the openSAFETY_DEMO directory
/bin/fpga/boards/[board]/ and select the file openSAFETY_DEMO_[fpga].jic.
6. Now that the FPGA and the flash device are added, check the boxes
   `Program/Configure`
and `Verify` and click the button `Start`.
7. After a successful download, a power cycle has to be performed to start the
board. After a few seconds, an on or blinking LEDG0 on the FPGA board indicates
that the flash was successful and the PCP is running.
A weak glowing LEDG0 indicates that the flash was not successful


## Using a CMake target {#sect_gs_flash_fpga_default_image_cmake}

1. Make sure the board is set up as described in \ref sect_gs_fpgaboard_setup.
2. Power up the board and connect it via USB to the computer.
3. Open the Nios II 13.0sp1 Command Shell and change to the openSAFETY_DEMO
   directory.
4. Change into the directory **build**. Alternatively a new build directory may
be created. Change into the newly created directory.
5. Enter

        > cmake -G"Unix Makefiles"  -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-altera-nios2-gnu.cmake ../bin

 for using the stripped down version of the cmake file or

        > cmake -G"Unix Makefiles"  -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-altera-nios2-gnu.cmake ../

 for using the main cmake file. Refer to \ref page_build_fpga_hw and
 \ref page_build_fpga_sw for further information on the configuration options.

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

 to write the pre-built bitstream and software to the FPGA board.

8. After a successful download, a power cycle has to be performed to start the
board. After a few seconds, an on or blinking LEDG0 on the FPGA board indicates
that the flash was successful and the PCP is running.
A weak glowing LEDG0 indicates that the flash was not successful.

# Downloading locally generated binaries to FPGA {#sect_gs_download_fpga_sw}

## Licensed Altera Quartus Version {#sect_gs_download_fpga_sw_licensed}

The FPGA configuration and application software can be downloaded to the
hardware by using predefined make targets.
The following steps require that the CMake configuration and generation has
been done (see \ref sect_gs_build_fpga_sw) and the FPGA Design Compilation was
successful. Make sure the board is set up as described in \ref sect_gs_fpgaboard_setup.
Power up the board and connect it via USB to the computer.

- Open a `Nios II Command Shell` or a shell where the path to Quartus II
  toolchain is set.

- To list all available make targets, type:

        > make help

- The following targets are available:

  Target                    | Description
  --------------------------|------------------------------
  download-bits-pcppsi | Download the FPGA configuration to the target's volatile memory.
  download-elf-pcppsi  | Download the software binary to the target's volatile memory.
  program-nvm-pcppsi   | Generate a flash image with the FPGA configuration and the software binary and write it to the local flash.
  erase-nvm-pcppsi     | Erase the local flash.
  flash_default_fpga_image | Write the pre-built image contained in the **bin** directory to the local flash.

- Type the following to download the bitstream and the software to the target's
  volatile memory.

        > make download-bits-pcppsi
        > make download-elf-pcppsi

- Type the following to download the bitstream and the software to the flash memory.

        > make program-nvm-pcppsi

  > **Note:** Make targets download-bits-pcppsi and download-elf-pcppsi
  > download configuration and software to volatile memory, which are therefore
  > lost after a power cycle of the FPGA board. To avoid a possible mixup for
  > configurations and software in volatile and non-volatile memory, flash memory
  > may be erased before downloading to volatile memory.

## Altera Quartus Web-Edition {#sect_gs_download_fpga_sw_web}

The Web-Edition of Quartus Software generates a time-limited
bitstream. It is therefore not possible to use the target `program-nvm-pcppsi`
to download the bitstream and the software to the non-volatile memory.

> **Note:**
> - Downloading the pre-built bitstream and software is not affected
>   by the Web-Edition of Quartus and can be done in the described way
>  (see \ref sect_gs_flash_fpga_default_image).
> - As the FPGA bitstream and the software will be downloaded to
>   the volatile memory, a power cycle will lead to a loss of the data.
>   This has to be considered if a power cycle of the application processor(s)
>   is needed.

The following steps require that the CMake configuration and generation has
been done (see \ref sect_gs_build_fpga_sw) and the FPGA Design Compilation was
successful.

There are two ways to download the bitstream and software:

### Use Quartus Programmer

* Make sure the board is set up as described in \ref sect_gs_fpgaboard_setup.
* Power up the board and connect it via USB to the computer.
* Open the programmer from within Quartus via the menu `Tools -> Programmer`
* Acknowledge the popup window and change to the programmer window which may
  be in the background
* If `No Hardware` is shown in the top left area, click the button
  `Hardware Setup...` and select `USB-Blaster [USB0]`,
  from the list box and close the window.
* Click the `Start` button. A status window will show up.
  Closing this status window or disconnecting the programming cable will
  immediately stop the processor on the FPGA.

  ![Quartus Programmer, Status window when using a Quartus Web-Edition generated .sof file](quartus_web_programmer.png)
  @image latex quartus_web_programmer.png "QQuartus Programmer, Status window when using a Quartus Web-Edition generated .sof file" width=0.33\textwidth

* from the `Nios II Command Shell` type

        > make download-elf-pcppsi

### Use two Nios II Command Shell instances

* Make sure the board is set up as described in \ref sect_gs_fpgaboard_setup.
* Power up the board and connect it via USB to the computer.
* use one Nios II Command Shell to download the bitstream by typing

    > make download-bits-pcppsi

  This will download the bitstream and block the shell / terminal until
  the program is left by pressing `q`. Exiting the programmer on command line
  or disconnecting the programming cable will immediately stop the processor
  on the FPGA.

* open a second `Nios II Command Shell` and type

        > make download-elf-pcppsi
