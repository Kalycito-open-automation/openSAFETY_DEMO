Target platform stm32f401re/Cortex-M4 {#page_gs_stm32f401re}
============

[TOC]


The target board stm32f401re with an ARM Cortex-M4 is well suited to host the
user application. Therefore the following application in the folder **app** is
ported to be built for this target hardware:
- `app/demo-sn-gpio:` The software for the openSAFETY SN over POWERLINK demo for
   **dual** and **single** channelled applications.

> **Note:** The **stm32f401re** board can only run application projects. It is
> therefore not possible to host the PCP part of the project on this board. An
> interconnect to a board which is able to run POWERLINK with the PSI driver is
> always necessary when using this setup.

# Build steps - stm32f401re   {#sect_gs_stm32f401_buildsteps}
The following steps need to be carried out in order to build the POWERLINK interface
for the **stm32f401re** board:
- \ref sect_gs_nucleoboard_setup and \ref sect_gs_board_connection
- \ref page_build_nucleo_sw
- \ref page_download_nucleo
