# STM32 Nucleo - Getting Started (Windows)

1. Compile st-link
   * Get the st-link project from github
     $ git clone https://github.com/texane/stlink.git
   * Investigate the file INSTALL.mingw and execute each step!
       - If ./configure (stlink) fails with "PKG_CHECK_MODULES USB unexpected token"!
         Add "ACLOCAL_AMFLAGS = -I /mingw/share/aclocal" to Makefile.am.
2. Get the embedded cross compiler (gcc-arm-none-eabi)
   - Download the ARM cross compiler from:
     https://launchpad.net/gcc-arm-embedded/+download
   - Install the package to C:\MinGW
3. Install the board st-link drivers from
   http://www.st.com/web/catalog/tools/FM146/CL1984/SC724/SS1677/PF251168?sc=internet/evalboard/product/251168.jsp
   - Install 'ST-LINK/V2 USB driver for Windows 7, Vista and XP'
   - Optional Install 'STM32 ST-LINK utility'
     (Enables easy firmware upgrade of st-link (Recommended!!))
4. Connect to the nucleo board:
   - You should be able to start the st-link gdbserver with:
     $ st-util
   - Use arm-none-eabi-gdb to connect to the server
     $ arm-none-eabi-gdb fancyblink.elf
     ...
     (gdb) tar extended-remote :4242
     ...
     (gdb) load
     Loading section .text, size 0x458 lma 0x8000000
     Loading section .data, size 0x8 lma 0x8000458
     Start address 0x80001c1, load size 1120
     Transfer rate: 1 KB/sec, 560 bytes/write.
     (gdb)
     ...
     (gdb) continue
5. Eclipse integration (TBD!!)
