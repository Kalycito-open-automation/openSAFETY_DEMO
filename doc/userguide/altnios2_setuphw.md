How to setup the board {#page_altsetuphw}
============

[TOC]

This section provides detailed information on how to setup the hardware of the
demo boards. 

# TERASIC Industrial Ethernet Kit (terasic-de2-115) {#sect_targxalt_terasicink}

The **terasic-de2-115** needs a couple of pre-settings which are shown in the provided
figure.

1. Set the jumpers **JP1**, **JP2**, **JP3**, **JP6** and **JP7**.
2. Set switch **SW19** to **Run**.
3. Connect the USB-Blaster to your PC via the USB-cable.
4. Connect one Ethernet port of the INK-board to a POWERLINK MN with the delivered cable.
5. Set the POWERLINK Node ID to **1**: Push **SW10** to upper position.
6. Switch on the board with the Power button (**SW18**).

![Terasic INK - Peripherals](terasic-de2-115.png)
@image latex terasic-de2-115.eps "Terasic INK - Peripherals"

The INK PHYs are not configured for auto-crossing the Ethernet connections.
Thus, a link will only be established if one of the following conditions is met.

1. Patch-cable connected to MDI-X (Media Dependent Interface Crossed), e.g. industrial hub.
2. Crossover cable connected to MDI (Media Dependent Interface) e.g. POWERLINK MN X20CP1485-1 IF 3.
3. Either cable type connected to auto-crossover interface.

> **Note:** In field applications CAT 5e cables are required.

# PCP <-> Application - Interconnect  {#sect_altsetuphw_interconnect}

In order to interconnect the POWERLINK processor with the application the GPIO
bank JP5 on the **terasic-de2-115** is used. The following figure shows the pinout
of this IO bank in detail. In order to power-up external application boards the
**3.3V/5V** and **GND** pins may be used.

![Terasic INK - JP5 Pinout](terasic-de2-115_jp5.png)
@image latex terasic-de2-115_jp5.eps "Terasic INK - JP5 Pinout" width=0.75\textwidth

> **Note:** If the POWERLINK processor and the application are located in the same
> bitstream jumpers may be used to connect the SPI master input with the slave
> output.
