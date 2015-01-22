How to setup the board {#page_stm32f401_setuphw}
============

[TOC]


This section provides information on how to setup the **stm32f401re** Nucleo board.
This setup changes if the desired demo application needs to run as a **single**
or **dual** channelled solution.

# Board setup (stm32f401re) - Single channelled    {#sect_stm32f401_setuphw_single}
In order to setup the board for the **demo-cn-gpio** or the **demo-sn-gpio** in
single channelled configuration, carry out the following steps:
1. Connect the jumper **JP5** to pin **1-2**. (Power supply over E5V)
2. Connect the stm32 **E5V** power supply pins with the terasic-de2 **JP5**
  (5V, GND) pins.
3. Connect the **PCP** <-> **uP** interconnect the following way:
    Name   | terasic-de2 | stm32f401
    :------|:------------|:---------
    sync   | AC15        | PC7
    SS_n   | Y17         | PB6
    MOSI   | Y16         | PA7
    MISO   | AE16        | PA6
    SCK    | AE15        | PA5

4. For programming and debugging connect the USB connector with your PC.

![Setup stm32f401 (Single channelled)](stm32_nucleo_single.png)
@image latex stm32_nucleo_single.eps "Setup stm32f401 (Single channelled)" width=0.6\textwidth

> **Note:** Take care to always connect the USB connector after the board is
> powered up. Otherwise it is not possible to program the board and the LED **LD1**
> glows orange.

# Board setup (stm32f401re) - Dual channelled    {#sect_stm32f401_setuphw_dual}
For a setup with two redundant **demo-sn-gpio** processors a special wiring is
needed. The following figure provides an overview of the wiring of the two boards.

> **Note:** You can use the bottom end of the E5V pins of uP-Master to forward
> the connection to uP-Slave. In addition most of the pins are wired to the
> **Morpho** and **Arduino** connector.

![Setup - stm32f401 (Dual channelled)](stm32_nucleo_dual.png)
@image latex stm32_nucleo_dual.eps "Setup - stm32f401 (Dual channelled)" width=1\textwidth

Carry out the following steps in order to wire a dual channelled demo:
1. Setup one board exactly as described in \ref sect_stm32f401_setuphw_single
2. Also connect the jumper **JP5** to pin **1-2** of **uP-Slave**.
3. Connect the uP-Slave **E5V** power supply pins with the ones from **uP-Master**.
   (You can use the bottom side end of the uP-Master pins!)
4. Wire the SPI bus **uP-Master** <-> **uP-Slave** in the following way:
    Name    | uP-Master | uP-Slave | Name
    :-------|:----------|:---------|:---------
    SCK     | PA5       | PA5      | SCK
    MISO    | PA6       | PA7      | MOSI
    SS_n    | PB6       | PA4      | SS_n
    sync    | PC7       | PC7      | sync

5. Wire the UART from **uP-Master** <-> **uP-Slave** bus the following way:
    Name    | uP-Master | uP-Slave | Name
    :-------|:----------|:---------|:---------
    UART_RX | PA9       | PA10     | UART_TX
    UART_TX | PA10      | PA9      | UART_RX
