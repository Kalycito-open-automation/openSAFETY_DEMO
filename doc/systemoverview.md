System Overview {#page_sysdesc}
============

[TOC]

# openSAFETY_DEMO {#sect_sysdesc_oS_demo}

This chapter provides an overview of the openSAFETY_DEMO system components
and how they interact with each other. The basic concept of the system is the
clear separation of the network protocol and the user application by using a
standalone processor for POWERLINK called the
`POWERLINK Communication Processor (PCP)` and a second standalone processor
for the application. The PCP handles the complete processing
of the network protocol and provides a memory interface to forward the process and
service data to the user application. System setup details are pictured in the next
figure.

![Component overview of the POWERLINK interface](system_overview.png)
@image latex system_overview.eps "Component overview of the POWERLINK interface"

## POWERLINK processor (PCP)  {#sect_sysdesc_oS_demo_pcp}
In order to handle the real time requirements of POWERLINK and to provide the
lowest possible cycle times, the PCP is implemented on an FPGA where the most time
critical parts are realized as soft IP-Cores. It consists of the openMAC IP-Core
for access to the POWERLINK network and the POWERLINK interface IP-Core for an
interface to the user application. More about these IP-Cores is given in the
section \ref page_idxipcore!

The software parts of the POWERLINK interface, like the protocol stack and the
interface drivers, are executed on a soft processor inside the FPGA and are
therefore out of the scope of an application engineer. Only basic configuration
steps need to be carried out inside the code of the PCP. (See \ref sect_sw_pcp)

## POWERLINK interface library   {#sect_sysdesc_oS_demo_libpsi}
The PSI library is executed on the application processor and provides a simple
interface for the user application. More details about the POWERLINK interface library can
be found in the \ref group_libpsi and \ref group_libpsicommon.

## Interconnect PCP <-> Application   {#sect_sysdesc_oS_demo_interconn}
The application processor uses a serial peripheral interface (**SPI**) as an interconnect
to access the PCP. In addition to this serial device the synchronous interrupt line
is connected to the application processor. This line is used to periodically trigger
the SPI master on the application processor which then starts a new cyclically data
transfer.

## Limits of the demo application {#sect_sysdesc_oS_demo_limits}
In order to reduce the code size at the application processor also the feature set
of the PSI is limited. The following list gives details about the limits of the
interface:

- The application is not able to write configuration parameters to the PCP during
  initialization.
- The number of RPDOs is limited to one.
- Sending and receiving of SDOs at the application is not possible.
  * Access to the data of unique objects can be achieved by using the
  \ref module_psi_cc.
- There is no Virtual Ethernet interface on the application.
  (It is therefore not possible to send or receive any generic ehternet frame
   at the application)

# User applications  {#sect_sysdesc_applications}
The application processor hosts the user application and can be any piece of
code which is executed on the target hardware and needs to transfer data by
using the POWERLINK protocol. In order to ease the access to the interface
the PSI user library is provided. This library needs to run on the application
processor and is linked to the user application. For demonstration purposes the
software package consists of the following example application which are ported
to the available hardware platforms.

## Safe Node - Demo  {#sect_sysdesc_app_sndemo}
The safe node demo (**demo-sn-gpio**) implements an openSAFETY SN on the
application processor which uses the PSI interface to access the POWERLINK
processor. In order to enable the special requirements of openSAFETY over
POWERLINK the PSI interface needs to be configured with the following
modules:
- \ref module_psi_status

    This interface internal module is used to forward the time information of
    POWERLINK to the application. In addition it internally handles the asynchronous
    transfer of the SSDO and logbook channel.

- \ref module_psi_pdo

    The PDO module is configured to forward the SPDO input and output images
    periodically. This demo is configured to receive one RSPDO image with the
    size \ref RX_SPDO0_SIZE and one TSPDO with the size \ref TX_SPDO_SIZE.
    The TSPDO channel is mapped to the POWERLINK object **0x4000** subindex **0x1**
    which is assigned by the macro \ref TPDO_LINKING_LIST_INIT_VECTOR!
    The RSPDO channel is mapped to the POWERLINK object **0x4001** subindex **0x1**
    which is assigned by the macro \ref RPDO_LINKING_LIST_INIT_VECTOR!

- \ref module_psi_ssdo

    The SSDO module provides the synchronous channel of the SN demo where for the
    demo application only one asynchronous channel is configured. The macros
    \ref SSDO_STUB_DATA_DOM_SIZE and \ref SSDO_STUB_OBJECT_INDEX define the size
    and object of the ssdo receive channel. The macros \ref TSSDO_TRANSMIT_DATA_SIZE
    and \ref SSDO_STUB_DATA_OBJECT_INDEX define the size and object of the transmit
    channel.

- \ref module_psi_log

    The logbook module implements an asynchronous channel of the SN which is
    dedicated to send asynchronous messages in a format which can be read by
    the B&R Automation Studio logbook.

The SN demo enables to be run as a **single** or **dual** channel configuration.
This means that the system setup pictured above is extended by a second redundant
application processor. This second processor shares the SPI serial device with the
other processor.

> For implementation details of this demo see \ref group_app_sn
