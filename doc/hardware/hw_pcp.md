Hardware Design - PCP {#page_hw_pcp}
============

[TOC]

# Overview - PCP    {#sect_hw_pcp_overview}
In general, each POWERLINK Slave consists of the following hardware components:
* FPGA
* Clock source (Quartz crystal oscillator)
* Non-volatile memory (PROM)
* Volatile memory (RAM)
* Ethernet physicals (PHYs)
* Ethernet jacks (RJ45) + transformer
* Diagnostic LEDs
* Node ID switches
* Debugging interface (JTAG)

# Recommended components    {#sect_hw_pcp_components}
The components listed in the following table are recommended for a POWERLINK CN
with PSI interface. Each of the parts are described in more detail throughout
this chapter of the documentation.

Part                  | Component family             | Type
----------------------|------------------------------|----------------------------------------------
FPGA                  | Altera Cyclone IV            | EP4CE6 or larger
Clock source          | Quartz crystal oscillator    | 50 MHz (Tolerance 50ppm)
Non-volatile memory   | Serial flash                 | SPI interface, = 16 MBit
Volatile memory       | SRAM                         | 32 bit, 10 ns, = 512 kByte
Ethernet PHYs         | IEEE 802.3u (Fast Ethernet)  | RMII (PHYs in slave mode)
Ethernet jacks        | RJ45 (8P8C)                  | 2x RJ45 jacks with integrated transformer
Diagnostic LEDs       | Single or two-coloured LEDs  | 1x green/red for S/E , 2x green for L/A 
Node ID switches      | Hardware Node ID switches    | 2x hexadecimal rotary switches
Debugging interface   | JTAG pin heads               | -

## FPGA    {#sect_hw_pcp_fpga}
The POWERLINK Slave is designed to work with any FPGA of the Altera **Cyclone IV**
family. Depending on the used configuration of the POWERLINK interface and the
requirements of the device, the actual part type can change. Note that a larger
FPGA (**EP4CE10**) needs to be selected if additional IP-Cores shall be placed
in the FPGA (e.g. second soft-core CPU for the internal application processor
configuration). Furthermore, it is recommended to use a larger FPGA during
development to fit debugging IP-Cores. The final design can then be size- and
therefore cost-optimized. (e.g: **EP4CE6**)

## Clock source    {#sect_hw_pcp_clock}
The POWERLINK Slave hardware should be equipped with a 50 MHz (Tolerance  50 ppm)
clock source which sources the FPGA and the Ethernet PHYs. The clock lines to the
components shall introduce the same delay for all clock connections.

## Non-volatile memory    {#sect_hw_pcp_nonvolmemory}
The FPGA configuration bit stream and the software for the soft-core CPU are
stored within one non-volatile storage device. It is recommended to use serial
flash memory components from which the FPGA is able to automatically read its
configuration during boot up.
The size of the flash memory mainly depends on the used FPGA type (larger FPGAs
have got a larger configuration bit stream) and on the code size of the
openPOWERLINK stack. Additionally, device-specific configuration data or the
code of the application processor (e.g. when using the internal application
processor configuration) might be stored in the same memory.

Memory segment              | Size (kBytes)
----------------------------|------------------
FPGA bit stream (EP4CE10)   | ~ 367 kBytes
POWERLINK stack code        | ~ 250 kBytes
Total (one firmware image): | ~ 617 kBytes


The total memory consumption for two images is around 1.2 MBytes, thus a flash
memory device with 2 MBytes (16 MBit) is required. A recommended part with this
size is the **M25P16-VMN6P** (compatible to EPCS16).

## Volatile memory    {#sect_hw_pcp_volmemory}
The soft-core processor executes the POWERLINK stack out of external RAM. In
order to provide the highest possible performance and determinism, a memory chip
fulfilling the requirements listed in the following table.

Characteristic          | Recommendation
------------------------|----------------------------------------------
Type of memory          | SRAM
Minimum size            | 512 kByte
Data width              | 32 bit (16 bit possible)
Performance             | 10 ns
Recommended parts       | CY7C1041DV33-10BVXIT \n IS61WV25616BLL-10BLI


> In order to fulfil hard real-time requirements, the PCP must be equipped with
> its ownRAM and cannot share it with the Application Processor. Therefore, for
> the internal application processor configuration, the FPGA needs to be
> connected to two separate RAM chips.

## Ethernet interface    {#sect_hw_pcp_ethernet}
The hardware platform of the POWERLINK Slave includes an Ethernet interface. It
is highly recommended to place two Ethernet ports on a POWERLINK device which
allows more flexible cabling options (e.g. line topology) at the end user's site.

> Due to heavy-duty demands in industry, Industrial Ethernet cables might be
> stiffer and their plugs larger than typical office / IT Ethernet plugs. Therefore,
> it is advised placing two separate RJ45 jacks rather than using a combined
> two-socket component.

An Ethernet device is electrically isolated from the network by a transformer.
Some RJ45 jacks already come with an integrated transformer which saves space on
the PCB. There are also RJ45 jacks with integrated LEDs available. Typically, those
LEDs are used for the link and activity signals.

Each transformer is wired to an Ethernet physical transceiver (PHY). The PHYs are
connected to the FPGA using either Media Independent Interface (MII) or Reduced
Media Independent Interface (RMII). For a POWERLINK Slave, the use of an RMII-capable
PHY in slave mode is preferred.

> Be careful with the PHY address strapping! Each PHY needs a unique address. The
> values 00h and 1F h shall be avoided, since these values are reserved for
> indicating a special mode on some PHYs.

Characteristic                   | Recommendation
---------------------------------|----------------------------------------
Ethernet standard                | IEEE 802.3u (Fast Ethernet)
Data rate                        | 100 MBit/s
No of Ethernet ports             | 2 (2x single RJ45 jack)
Physical transceiver interface   | RMII (PHY in slave mode)
Recommended parts                | National DP83640 \n National DP83848T

## Diagnostic LEDs    {#sect_hw_pcp_leds}
For each Ethernet port LEDs indicating the link status and the Ethernet activity
are required. Often, both signals are combined in a single LED. The link and
activity signals are typically produced by the PHY and can either be directly
wired to the according LED or can be routed through the FPGA. The latter option
allows overriding the PHY signals by the POWERLINK Slave software and might
be useful for some applications (e.g. running LED tests for quality assurance
during series production).

Additionally, a POWERLINK Slave device needs to be equipped with a green STATUS
LED and a red ERROR LED. Instead of two separate LEDs, also a combined two-colour
LED (red/green) can be used. In this case, the red colour (ERROR) shall be the
dominant signal and shall override the state of the green colour (STATUS).

> There are also RJ45 jacks with integrated LEDs available. Typically, those
> LEDs are used for the link and activity signal.

Characteristic      | Recommendation
--------------------|----------------------------------------------------------------------------
2x Link (L) LED     | 1 green colour LED with a combined link/activity signal for each connection
2x Activity (A) LED | -
1x Status (S) LED   | Green colour of a dual-colour LED
1x Error (E) LED    | Red colour of a dual-colour LED

## Node ID switches    {#sect_hw_pcp_switches}
Each POWERLINK device is identified by its 8 bit Node ID on the network. For a
POWERLINK Slave the valid Node IDs are ranging from 1 to 239. In order to
configure the POWERLINK Node ID, a hardware Node ID switch is highly recommended.
Due to the small footprint on the PCB the most commonly used parts are two
hexadecimal rotary switches. Each of the switches represents a nibble of the Node
ID.

> The switches may be directly connected to the FPGA. If they are not, a valid
> Node ID must be passed from the application processor during initialization of
> the POWERLINK Slave.

## Debugging interface    {#sect_hw_pcp_debugging}
For downloading and debugging the FPGA design during development, a JTAG interface
is required. On the final product, the pins of the interface can be left unconnected
or even omitted, unless the JTAG interface is used to program the initial firmware
to the flash during the production process.
