Downloading the Nucleo software {#page_download_nucleo}
============

This page provides information about downloading the Nucleo software.
There are several ways to download the software to the Nucleo board.

\warning Wrong wiring of the boards or previously downloaded software
which drives the I/O pins in a different manner can destroy the hardware.
In case of doubt, do the initial flash without the wiring, just with
the power supply and the USB programming cable connected.

[TOC]

> **Notes for downloading the software:**
>
> * The boards have to be powered up **before** they are connected
>  via USB to the computer.
>
> * When downloading software to the boards, it must to be ensured that
>   only one Nucleo board is connected to the computer during that time.
>
> * "WARN src/stlink-common.c: unknown chip id! 0xe0042000"
>   This indicates that an unknown (or unpowered) micro-controller board is
>   connected to the host-computer.
>
> * "SRAM size: 0x18000 bytes (96 KiB), Flash: 0 bytes (0 KiB) in pages
>   of 16384 bytes" This indicates that the micro-controller board has been
>   connected to the host-computer first before it got powered up
>   by the externalpower-source.
>
> * When the application gets stuck on "Restore SOD" the whole
>   flash memory of the microcontroller should be erased, and the download of
>   the software must be started again.
>   Erasing the flash can be done with STM32 ST-LINK Utility:
>   `Target -> Erase chip` or with the button `Full chip erase`.


# Downloading the pre-built Nucleo Software {#sect_download_nucleo_pre-built}
## Download via copy / paste {#sect_download_nucleo_cp}

When connecting the Nucleo board to the computer via USB, the Nucleo board
will be recognized as a USB drive. Therefore, binary files can be downloaded
to the board by just copying / drag and dropping the binary file onto the
related USB drive:

- Ensure the Nucleo board is powered up
- Connect the board via USB with the computer
- The board will be recognized as USB drive
- Select the appropriate binary file from the demo's **bin/[NUCLEO]** folder,
  where `NUCLEO` corresponds to the used board and can either be `stm32f103rb`
  or `stm32f401re`. In a single channelled configuration, select the file
  `demo-sn-gpio-single.bin`. In a dual channelled configuration select the file
  `demo-sn-gpio-upm.bin`, when downloading to the board acting as uP-Master,
  and the file `demo-sn-gpio-ups.bin` when downloading to the Nucleo board
  acting as uP-Slave.
- Copy / paste or drag and drop the file to the Nucleo USB drive.
- The flashing of the board is complete, after the copying process was successful.

## Download pre-built Nucleo Software via make target {#sect_download_nucleo_pre-built_make}

- Ensure the Nucleo board is powered up
- Connect the board via USB with the computer.
  When downloading software this way, just one Nucleo may be connected to the
  computer at a time.
- Change into the build directory or create a new one and change there.
- Generate a build configuration with CMake

        > cmake -G"Unix Makefiles"  -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-arm-[CORTEXMX]-gnueabi.cmake ../bin

  `CORTEXMX` can be `cortexm3` (for Nucleo-F103RB) or `cortexm4`
  (for Nucleo-F401RE),
  depending on the used Nucleo board.

- Depending on the build configuration some of the following targets are
  available for downloading the software:

  Target Name                    | Description
  -------------------------------|----------------------------------
  flash-default-demo-sn-gpio-single.bin | download the pre-built image for single channelled configuration from **bin** directory
  flash-default-demo-sn-gpio-upm.bin    | download the pre-built image for the uP-Master of a dual channelled configuration from **bin** directory
  flash-default-demo-sn-gpio-ups.bin    | download the pre-built image for the uP-Slave of a dual channelled configuration from **bin** directory

- To list available targets type

        > make help

- To download the software to the connected Nucleo board type

        > make TARGETNAME

# Download locally built Nucleo Software via make target {#sect_download_nucleo_make}

- Ensure the Nucleo board is powered up
- Connect the board via USB with the computer.
  When downloading software this way, just one Nucleo may be connected to the
  computer at a time.
- Generate a build configuration with CMake with the settings needed for the used
  Nucleo boards as described in \ref page_build_nucleo_sw.
- Depending on the build configuration some of the following targets are
  available for downloading the software:

  Target Name                    | Description
  -------------------------------|----------------------------------
  flash-default-demo-sn-gpio-single.bin | download the pre-built image for single channelled configuration from **bin** directory
  flash-default-demo-sn-gpio-upm.bin    | download the pre-built image for the uP-Master of a dual channelled configuration from **bin** directory
  flash-default-demo-sn-gpio-ups.bin    | download the pre-built image for the uP-Slave of a dual channelled configuration from **bin** directory
  download-demo-sn-gpio-single    | (build and) download software for single channelled configuration
  download-demo-sn-gpio-upm       | (build and) download the built software for the uP-Master of a dual channelled configuration
  download-demo-sn-gpio-ups       | (build and) download the built software for the uP-Slave of a dual channelled configuration

- To list available targets type

        > make help

- To download the software to the connected Nucleo board type

        > make TARGETNAME
