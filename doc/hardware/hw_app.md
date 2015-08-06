Hardware design - Application {#page_hw_app}
============

[TOC]

# Overview - Application    {#sect_hw_app_overview}

Basically the board setup changes heavily depending on the configuration of the
user application.
As the PSI library is a generic software it should be possible to run on huge
variety of microcontrollers.

# Layout - Single Channelled   {#sect_hw_app_single}

In case of a single channelled SN demo the following uC features are needed or
recommended:

SN feature            | uC - Feature
----------------------|------------------------------
Synchronous IR        | External IR controller
PCP serial            | SPI master + DMA controller
Non-volatile memory   | Parallel flash (128kb)
Volatile memory       | SRAM (20kb)
Consecutive time      | 16bit timer
SOD storage           | Needs to fit into the parallel flash (optional)
Debugging interface   | JTAG pin header + programmer (e.g: stlink)
Debug prints          | Serial to the host PC
Application           | Hardware to support the user application (e.g: GPIO pins)

# Layout - Dual Channelled   {#sect_hw_app_dual}

In case of a dual channelled SN demo a special setup is required. Basically it is
recommended to use two of the microcontrollers used for the single channelled demo
with an additional serial device between the uP-Master and uP-Slave. In addition,
the serial to the PCP needs to be an SPI in slave mode.

SN feature            | uC - Feature
----------------------|------------------------------
uP serial             | UART serial + DMA controller
PCP serial            | SPI slave + DMA controller
