Build environment - Setup {#page_buildenv_stm32}
============

[TOC]

# On Windows   {#sect_buildenv_windows}
This section covers to setup the build environment for using the STM32 Nucleo
board with the openSAFETY_DEMO under Windows.

> **Note:** This guide explicitly uses only open source software. There are probably
> many other ways to get the demo software running on an stm32 Nucleo board.

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

> **Note:** The st-link tools rely on libusb to access the ST-LINK/V2.
> Older versions of libusb have trouble accessing ST-LINK/V2 debug adaptors
> when they are connected to an USB 3.0 port (even with an USB 2.0 hub in
> between). In this case the error message "Couldn't find any ST-Link/V2
> devices" will be shown.
> To solve this issue, a newer version of libusb has to be used.
> Version 1.0.19 has been tested to be successful.
>
> See files INSTALL in libusb sources and INSTALL.mingw in the st-link
> sources for building libusb.

> **libusb build issues for Windows:** On some systems, while trying to
> build libusb, ./configure may cause wrong results while checking for
> "struct timespec". To solve this issue, please remove the line
> `#define HAVE_STRUCT_TIMESPEC 1` in the file config.h.
>
> To fix errors like *"In function 'get_windows_version':
> os/windows_usb.c:850:4: error: implicit declaration of function
> 'VerSetConditionMask' [-Werror=implicit-function-declaration] major_equal
> = VerSetConditionMask(0, VER_MAJORVERSION, VER_EQUAL);"*:
> add the defines with the appropriate value to the file **config.h** and
> build libusb again.
> `#define WINVER 0x0601`
> `#define _WIN32_WINNT 0x0601`
>
> - Windows 7: 0x0601
> - Windows 8: 0x0602
> - Windows 8.1: 0x0603
> - for other versions see https://msdn.microsoft.com/en-us/library/aa383745.aspx

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
