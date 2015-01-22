Build environment - Setup {#page_buildenv_stm32}
============

[TOC]

# On Windows   {#sect_buildenv_windows}
This section covers to setup the build environment for using the STM32 Nucleo
board with the PSI under Windows.

> **Note:** This guide explicitly uses only open source software. There are probably
> many other ways to get the PSI demos on an stm32 Nucleo board running.

1. Download **MinGW** from: http://sourceforge.net/projects/mingw/
   - Install the following packages:
      * mingw32-base
      * mingw-developer-toolkit
      * msys-base
      * mingw32-gcc-g++ (To compile st-link)
   - Open the file: **C:\\MinGW\\msys\\1.0\\etc\\fstab** and uncomment: *c:/mingw /mingw*
2. Get the embedded cross compiler (**gcc-arm-none-eabi**)
   - Download the ARM cross compiler from:
     https://launchpad.net/gcc-arm-embedded/+download
   - Install the package to **C:\\MinGW**. (Or set your environment variables accordingly!)
3. Get the **st-link** tools.
   - See https://github.com/texane/stlink/wiki for Windows binaries or see \ref sect_buildenv_windows_stlink
   - Install the package to **C:\\MinGW**. (Or set your environment variables accordingly!)
4. Install the Nucleo board **stlinkv2** drivers from
   http://www.st.com/web/catalog/tools/FM146/CL1984/SC724/SS1677/PF251168?sc=internet/evalboard/product/251168.jsp
   - Install `ST-LINK/V2 USB driver for Windows 7, Vista and XP`
   - Optionally install `STM32 ST-LINK utility`
     (Enables easy firmware upgrade of st-link devices!)

## Compile st-link   {#sect_buildenv_windows_stlink}
This gives details on how to compile the **st-link** tools on windows with
**MinGw**:
1. Get the st-link project from github
    > $ git clone https://github.com/texane/stlink.git
2. Investigate the file INSTALL.mingw and execute each step!
    - If ./configure (stlink) fails with *PKG_CHECK_MODULES USB unexpected token!*
      Add the line: *ACLOCAL_AMFLAGS = -I /mingw/share/aclocal* to the file **Makefile.am**.

# On Linux   {#sect_buildenv_linux}
This section covers to setup your build environment under Linux. Carry out the
following steps in order to do this:

1. Install the following package from your package manager:
   - libusb
   - automake
   - autoconf
   - pkgconfig
   - gcc-arm-none-eabi
   - git
2. Get and compile st-link with:

       > git clone https://github.com/texane/stlink.git stlink-utility
       > cd stlink-utility
       > ./autogen.sh
       > ./configure
       > make
