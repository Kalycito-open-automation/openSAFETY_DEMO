How to build the software {#page_stm32f103_buildsw}
============

[TOC]

This section provides information on how to build the software for the **stm32f103rb**
Nucleo board. The following steps need to be carried out in order to do this:

1. Create a build environment with **bash** shell, the **GNU CoreUtils** and the
   **gcc-arm-none-eabi** compiler. (See \subpage page_buildenv_stm32)
2. Inside the shell enter the build directory and execute CMake in the build
   directory with the path to the source directory as a parameter.

       > cd build
       > cmake -G"Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-arm-cortexm3-gnueabi.cmake \
       >       -DCFG_STM32_HAL_LIB_LOC=../../contrib/STM32_Nucleo_FW_V1.2.1 ../

   This will generate Unix style Makefiles for the stm32f103rb platform with the
   default configuration. See the section \ref sect_buildsw_options and
   \ref sect_stm32f103_buildsw_options for all available options which can be
   passed to CMake. Simply add `-D[YOUR_OPTION]` to the build command above.

3. After CMake has finished generating the Makefiles it is possible to compile
   the software for your project. This can be done with the following command.

       > make all

4. After the build is finished it is possible to write the executable to the local
   flash memory. (e.g: DEMO_NAME=demo-sn-gpio-upm)

       > make download-[DEMO_NAME]

> **Note:** Use `make help` to print all available make targets.

When downloading software to the boards, it must to be ensured that
only one Nucleo board is connected to the computer during that time.

**IMPORTANT:** The boards have to be powered up before they are connected via
USB to the computer.

**NOTE**: "WARN src/stlink-common.c: unknown chip id! 0xe0042000"
This indicates that an unknown (or unpowered) micro-controller board is
connected to the host-computer.

**NOTE**: "SRAM size: 0x18000 bytes (96 KiB), Flash: 0 bytes (0 KiB) in pages
of 16384 bytes"
This indicates that the micro-controller board has been connected
to the host-computer first before it got powered up by the external
power-source.

> **Note:** When the application gets stuck on "Restore SOD" the whole
> flash memory of the microcontroller should be erased, and the download of the
> software must be started again.
> Erasing the flash can be done with STM32 ST-LINK Utility:
> `Target -> Erase chip` or with the button `Full chip erase`.

# Target specific configuration option (stm32f103rb)   {#sect_stm32f103_buildsw_options}

- **CFG_STM32_HAL_LIB_LOC**

 Specify the path to the stm32f103 HAL library (**STM32CubeF1 V 1.2.0**).
 Get it from: http://www.st.com/web/catalog/tools/FM147/CL1794/SC961/SS1743/LN1897/PF260820
 > **IMPORTANT**: There is a bug in Version 1.2.0 of the STM32CubeF1 software.
 > Open the file
 > `[STM32CubeDirectory]/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_spi.c`
 > and comment the following lines in function `HAL_SPI_Receive_DMA()`:
 >
 > ~~~~~~~~~~~~~~~~~~~~~{.c}
 >   /*hspi->hdmatx->XferHalfCpltCallback = 0;*/
 >   /*hspi->hdmatx->XferCpltCallback     = 0;*/
 >   /*hspi->hdmatx->XferErrorCallback    = 0;*/
 > ~~~~~~~~~~~~~~~~~~~~~


- **CFG_ARM_BOARD_TYPE**

  Enables to specify the kind of Cortex-M3 board type (e.g: **stm32f103rb**)

- **CFG_PROG_FLASH_ENABLE**

  Enables a custom target to write the firmware to the local flash by using the
  tool **st-flash**.

- **CFG_DUAL_CHANNEL**

  Enables switching between single and dual channelled application.

# Debug the software (stm32f103rb)   {#sect_stm32f103_buildsw_debug}
It is possible to debug the software using arm-none-eabi-gdb, the st-link tools
and the **Eclipse IDE**. With this you should be able to comfortably step through
the code and set breakpoints.

1. Set the CMake generator to: **Eclipse CDT4 - Unix Makefiles**
   - This will generate an additional Eclipse project with the standard Unix Makefiles.

2. Get Eclipse from https://eclipse.org
   - The Eclipse IDE needs to be started from a shell where the path is set
     to all tools needed to compile build and run the demos on an stm32.

3. Import generated CMake Eclipse project into the Eclipse IDE with:
   - File -> Import -> Existing Projects Into Workspace.
   - Set 'Select root directory' to the CMake build folder.
   - Select the found project and hit 'Finish'

4. Open a second shell start the tool **st-util**. This tool is a gdb server
   which enables you to connect with the gdb client (e.g: arm-none-eabi-gdb).
   The tool st-util listens on port 4242 with not set differently.

5. Define a new 'Debug Configuration' in Eclipse:
   - Run -> Debug Configuration -> C/C++ Remote Application
   - In the 'Main' tab select the **elf** file as an application.
   - In the 'Debugger' tab set the following:
     * GDB debugger = arm-none-eabi-gdb
     * Connection / Host = localhost
     * Connection / Port = 4242

6. \b -> Step through the code and find the bug!

> **Note:** If debugging isn't possible in the described way with
> C/C++ Remote Application Debug Configuration go to
> `Help -> Install New Software`.
> In the new window select the entry that matches your version of Eclipse
> e.g. "[VERSION] - http://download.eclipse.org/releases/[VERSION]" from the
> `Work with` drop down menu.
> Enter "hardware" in the filter text box and install
> `C/C++ GDB Hardware Debugging` from the section
> `Mobile and Device Development`.
> Now a new Debug Configuration can be added (GDB Hardware Debugging):
> In the 'Main' tab select your .elf file you want to debug.
> In the 'Debugger' tab make the following settings:
> - GDB Command: arm-none-eabi-gdb
> - check use remote target checkbox
> - Host name or IP address: localhost
> - Port number: 4242


