User Guide {#page_ugindex}
============

[TOC]

The user guide provides details about the build steps of each supported platform
where all platforms use the tool **CMake** as a common configuration
point.

The following target platforms are supported by the PSI where not all platforms
always provide a full set of features.

- \subpage page_x86
- \subpage page_altnios2
- \subpage page_stm32f103rb
- \subpage page_stm32f401re

# CMake    {#sect_buildsw_cmake}

The POWERLINK interface extensively uses the cross-platform, open-source build
system called **CMake**. You can download the tool free of charge from
http://www.cmake.org/ (On Windows) or install it from the local package manager
(On Linux).

## Executing CMake    {#sect_buildsw_execmake}

The configuration of different build options of the POWERLINK interface can be
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
in a separate build directory which keeps your sources clean. The PSI project
already provides the folder `build` in the root directory. If the target
platform consists of several processors with different architectures a second
or third build folder may be created by the user.


## Configuration Options {#sect_buildsw_options}

The following build configuration options are available to pass to **CMake** by
using the `-D` parameter or by enabling them in the graphical user interface:

- **CFG_DEMO_TYPE**

  Change the type of the demo to build. (e.g: `cn-gpio`, `sn-gpio` or `custom`)

- **CMAKE_BUILD_TYPE**

  This parameter specifies the build type of the PSI project. The following options
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

### SN configuration options {#sect_buildsw_options_sn}
All options from this sections are valid when `CFG_DEMO_TYPE=sn-gpio`. This means
that the application build system is set to host a safe node (SN) demo.

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
