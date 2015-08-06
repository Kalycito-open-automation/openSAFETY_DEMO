User Guide {#page_ugindex}
============

[TOC]

The user guide provides details about the needed hardware, software and toolchains,
as well as the build steps of each supported platform
where all platforms use the tool **CMake** as a common configuration
point.

The following target platforms are supported by the openSAFETY_DEMO where not
all platforms always provide a full set of features.

- \subpage page_x86
- \subpage page_altnios2
- \subpage page_stm32f103rb
- \subpage page_stm32f401re


# Hardware Requirements   {#sect_requirements_hw}

The following hardware is needed for using the openSAFETY_DEMO:

- 1x Terasic DE2-115 FPGA Board http://www.terasic.com.tw/cgi-bin/page/archive.pl?Language=English&CategoryNo=139&No=502
- 2x STMicroelectronics NUCLEO-F401RE http://www.st.com/web/catalog/tools/FM116/SC959/SS1532/LN1847/PF260000

For a complete system integration i.e. putting the demo into operation in an
B&R openSAFETY system, the following hardware is needed:
- B&R X20CP1585 CPU
- B&R X20SL8100 SafeLOGIC + SafeKEY X20MK0211 or SafeKEY X20MK0213

# Software Requirements   {#sect_requirements_sw}

The software requirements are depending on the desired usage of the openSAFETY_DEMO:
1. Usage of the pre-built binaries only:

  For flashing the development hardware with the pre-built binaries only, the following software is needed:
  - ST-Link/V2-1 USB driver for your NUCLEO board
    - NUCLEO-F401RE: http://www.st.com/web/catalog/tools/FM116/SC959/SS1532/LN1847/PF260000

  - Altera Quartus Programmer v13.0sp1 https://www.altera.com/downloads/download-center.html

2. Build the software locally:

  For building the software and the FPGA bitstream locally,
  the following software is additionally needed to (1):
  - MinGW (+ MSYS) http://sourceforge.net/projects/mingw/
  - GCC ARM Embedded 4.8-2014-q2-update https://launchpad.net/gcc-arm-embedded/4.8/4.8-2014-q2-update
  - stlink tools https://github.com/texane/stlink
  - libusb http://libusb.info/
  > For further information on installing the mentioned tools refer to \ref page_buildenv_stm32.

  - HAL Library for the used NUCLEO board:
    - NUCLEO-F401RE: STM32CubeF4 V1.7.0: http://www.st.com/web/en/catalog/tools/PF259243
  - CMake http://www.cmake.org/
  - Altera Quartus v13.0sp1 with device support for Cyclone IV E devices https://www.altera.com/downloads/download-center.html
  > **Note:** The Quartus Programmer is part of the Quartus Toolchain,
  > it doesn't have to be downloaded separately.
  >
  > **Note:** Both versions of Quartus Software (Web-Edition, Subscription-Edition)
  > are able to generate the FPGA bitstream. With the Web-Edition it is not
  > possible to download a design to the non-volatile memory. This means, that
  > a continuous connection between FPGA board and computer / Quartus is
  > mandatory to use the generated bitstream.
  > In the case the connection gets closed, the PCP part will stop working.


3. Modify the openSAFETY_DEMO with user customised OSDD file:

  For customising the demo in respect to the processing of input and output data
  as well as the used GPIO pins, the following software is additionally needed to (2):
  - Python (successfully tested with versions 2.7.9 and 3.4.2) https://www.python.org

    The following Python packages need to be installed:
    * PyXB 1.2.4 https://pypi.python.org/pypi/PyXB/1.2.4
    * cogapp 2.4 https://pypi.python.org/pypi/cogapp/2.4

4. System integration in an B&R openSAFETY system:

  For integrating the demo in an B&R openSAFETY system and putting the whole
  system into operation the following software is needed:
  - Automation Studio 4.2.4 or higher
  - SafeDESIGNER 4.2 or higher


# CMake    {#sect_buildsw_cmake}

The openSAFETY_DEMO extensively uses the cross-platform, open-source build
system called **CMake**. You can download the tool free of charge from
http://www.cmake.org/ (On Windows) or install it from the local package manager
(On Linux).

## Executing CMake    {#sect_buildsw_execmake}

The configuration of different build options of the openSAFETY_DEMO can be
done through the cmake interface. There are three possible ways for configuring
your build options:

* `cmake-gui`

  This is the most comfortable way for configuring the build options. cmake-gui
  is a graphical user interface for CMake.

* `ccmake`

  ccmake is a curses based user interface for cmake.

* `cmake -i`

  If cmake is called with option -i you can interactively select your build
  options.

## Out-of-Source Builds {#sect_buildsw_outofsource}

CMake support out-of-source builds. Therefore, all generated files are located
in a separate build directory which keeps your sources clean. The
openSAFETY_DEMO project already provides the folder `build` in the root
directory. If the target platform consists of several processors with different
architectures a second or third build folder may be created by the user.


## Configuration Options {#sect_buildsw_options}

The following build configuration options are available to pass to **CMake** by
using the `-D` parameter or by enabling them in the graphical user interface:

> **Note:**When using `cmake-gui` check the boxes `Grouped` and `Advanced`
> to make visible all available options grouped by their names.

Variable name                 | Description                               | Default value
----------------------------- | ------------------------------------------|--------------
CFG_DEMO_TYPE       | Change the type of the demo to build | sn-gpio
CMAKE_BUILD_TYPE    | Specifies the build type of the openSAFETY_DEMO project | Release
CFG_APP_DEBUG_LVL   | Change the debug level of your selected application   | 0xEC000000L
CFG_PSI_DEBUG_LVL   | Change the debug level of the PSI library     | 0xEC000000L
CFG_PSICOMMON_DEBUG_LVL | Change the debug level of the PSI common library | 0xEC000000L
PYSODBUILDER_ENABLE | Enable the generation of customised code based on the given OSDD file | False
USE_CUSTOMISED_FILES | Enable or disable the usage of customised code in /user_customisable | True

- **CFG_DEMO_TYPE**

  Change the type of the demo to build. (e.g: `sn-gpio` or `custom`)

- **CMAKE_BUILD_TYPE**

  This parameter specifies the build type of the openSAFETY_DEMO project.
  The following options
  are available:
    * `Release:` The project is built with highest optimization and no debug information.
    * `Debug:` The project is built with no optimization and debug information.
    * `RelWithDebInfo:` The project is built with highest optimization and debug information.
    * `MinSizeRel:` The project is built with size optimization and no debug information.

- **CFG_APP_DEBUG_LVL**

  Change the debug level of your selected application (Depends on the setting of `CFG_DEMO_TYPE`).

- **CFG_PSI_DEBUG_LVL**

  Change the debug level of the PSI library. (See \ref group_libpsi)

- **CFG_PSICOMMON_DEBUG_LVL**

  Change the debug level of the PSI common library. (See \ref group_libpsicommon)

- **PYSODBUILDER_ENABLE**

  Enables or disables the generation of customised code based on the given OSDD file.
  Refer to \ref page_user_customisations for further information.

- **USE_CUSTOMISED_FILES**

  Enables or disables the usage of customised code in /user_customisable.
  This option depends on the selected target system and `PYSODBUILDER_ENABLE`.
  Refer to \ref page_user_customisations for further information.

### SN configuration options {#sect_buildsw_options_sn}
All options from this sections are valid when `CFG_DEMO_TYPE=sn-gpio`. This means
that the application build system is set to host a safe node (SN) demo.

Variable name                 | Description                               | Default value
----------------------------- | ------------------------------------------|--------------
CFG_DUAL_CHANNEL    | Specifies if the demo has one or two SN channels | single-channel
CFG_SAPL_SN_UDID    | Specifies the UDID of your SN | {0x00U,0x60U,0x65U,0x01U,0x14U,0x9BU}
CFG_SAPL_SN_VENDORID| Specifies the vendor ID of your SN    | 0x00005678UL
CFG_SAPL_SN_PRODUCT_CODE | Specifies the product code of your SN | 0x00000001UL
CFG_SAPL_SN_REVISION_NR | Specifies the revision number of your SN  | 0x00010100UL
CFG_SAPL_SN_SERIAL_NR   | Specifies the serial number of your SN    | 0x5A5A5A5AUL
CFG_SAPL_SN_FW_CHKSUM   | Specifies the firmware checksum of your SN    | 0x5A5A5A5AUL

- **CFG_DUAL_CHANNEL**

  This parameter specifies if the demo has one or two SN channels. A second
  channel means that the application is hosted on two redundant processors.

- **CFG_SAPL_SN_UDID**

  This parameter specifies the UDID of your SN. This ID needs to be set as a unique
  number for each SN in the network.

- **CFG_SAPL_SN_VENDORID**

  This parameter specifies the vendor ID of your SN. (Object 0x1018/0x1 in the SOD)

- **CFG_SAPL_SN_PRODUCT_CODE**

  This parameter specifies the product code of your SN. (Object 0x1018/0x2 in the SOD)

- **CFG_SAPL_SN_REVISION_NR**

  This parameter specifies the revision number of your SN. (Object 0x1018/0x3 in the SOD)

- **CFG_SAPL_SN_SERIAL_NR**

  This parameter specifies the serial number of your SN. (Object 0x1018/0x4 in the SOD)

- **CFG_SAPL_SN_FW_CHKSUM**

  This parameter specifies the firmware checksum of your SN. (Object 0x1018/0x5 in the SOD)


# Flashing the Boards {#sect_flashing}

Please refert to \subpage page_flashing for information on how to download the
software to the boards.

# Customise the openSAFETY_DEMO {#sect_user_customisations}
Please refer to \subpage page_user_customisations for information on
customisation possibilities without having to alter the original source code.

# System Integration {#sect_system_integration}
Please refer to \subpage page_system_integration for information on how to
integrate the openSAFETY_DEMO into a B&R openSAFETY system and put the whole
system into operation.