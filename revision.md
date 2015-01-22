Revision History {#page_revision}
==================

[TOC]

Revision history of the POWERLINK slim interface (PSI).

# V1.0.0    {#sect_rev_v100}

Initial release of the POWERLINK slim interface.

* `Features:`
  - openPOWERLINK stack PSI driver.
  - PCP port to target board terasic-de2-115 with demos:
     * cn-dual-spi-gpio
     * cn-pcp-spi
  - PSI user libraries:
     * libpsi
     * libpsicommon
  - PSI user library unit tests.
  - Application processor demos:
     * demo-cn-gpio
     * demo-sn-gpio
        - Single channelled
        - Dual channelled
  - Demos ported to platforms:
     * terasic-de2-115
     * stm32f103rb
     * stm32f401re
  - Full documentation in doxygen.
  - Common build system for all platforms (CMake)
* `Fixes:`
    * none
