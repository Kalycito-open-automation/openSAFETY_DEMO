The Fast Track {#page_fast_track}
============

This page provides information about bringing the openSAFTEY_DEMO into operation
in the shortest possible time with minimum  hardware and software.

[TOC]

# Hardware Requirements   {#sect_fastttrack_requirements_sw}

Refer to \ref page_overview_hardware for the hardware requirements.

> **Note:** The **single-channel configuration** of the demo requires only
> **a single Nucleo board**. This is just for testing purposes and may NOT be a
> valid hardware architecture concerning requirements of standards addressing
> (functional) safety.

# Software Requirements {#sect_fasttrack_requirements_sw}

- Windows only: Nucleo USB driver
- Quartus Programmer 13.0sp1
- Windows only: Automation Studio and SafeDESIGNER

See \ref page_install_software for further information on installing the software.

# Hardware Setup and Connection {#sect_fasttrack_hw_setup}

Setup the demo hardware as described in \ref page_setup_hardware.

# Flash the boards {#sect_fasttrack_board_flashing}

## Flashing the FPGA board {#sect_fasttrack_flash_fpga}

- Make sure the Altera USB-Blaster has been set up (see \ref sect_gs_install_software_quartus_driver)
- Connect the FPGA Board to the computer via USB and open the Quartus Programmer
- Click `Add file` and select the file `openSAFETY_DEMO_altera.jic` from the
  demo directory `bin\fpga\boards\terasic-de2-115`.
- Click `Start`

## Flashing the Nucleo board {#sect_fasttrack_flash_nucleo}

- Make sure the Nucleo USB driver has been installed.
- Connect the Nucleo board(s) via USB to the computer, it should be recognised as USB drive.
- Depending on a single- / dual-channel configuration:
  * single-channel:
    copy  the file `demo-sn-gpio-single.bin` from the folder
    `bin\[microcontroller]` on the Nucleo board.
    [microcontroller] can be **stm32f401re** or **stm32f103rb**, depending on
    the used Nucleo board.
  * dual-channel:
    - copy the file `demo-sn-gpio-ups.bin` from the folder `bin\[microcontroller]`
      on the Nucleo board acting as the slave microcontroller.
     [microcontroller] can be **stm32f401re** or **stm32f103rb**,
     depending on the used Nucleo board.

    - copy the file `demo-sn-gpio-upm.bin` from the folder `bin\[microcontroller]`
      on the Nucleo board acting as master microcontroller.

## System Integration {#sect_fasttrack_system_integration}

Refer to \ref page_system_integration and carry out all steps to bring the
system into operation.
