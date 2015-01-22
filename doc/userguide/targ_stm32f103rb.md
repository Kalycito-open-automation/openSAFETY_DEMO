Target platform stm32f103rb/Cortex-M3 {#page_stm32f103rb}
============

[TOC]


The target board stm32f103rb with an ARM Cortex-M3 is well suited to host the
user application. Therefore the following applications in the folder **app** are
ported to be built for this target hardware:
- `app/demo-cn-gpio:` The software for the POWERLINK CN only demo with GPIO application.
- `app/demo-sn-gpio:` The software for the openSAFETY SN over POWERLINK demo for
   **dual** and **single** channelled applications.

> **Note:** The **stm32f103rb** board can only run application projects. It is
> therefore not possible to host the PCP part of the project on this board. An
> interconnect to a board which is able to run POWERLINK with the PSI driver is
> always necessary when using this setup.

# Build steps - stm32f103rb   {#sect_stm32f103_buildsteps}
The following steps need to be carried out in order to build the POWERLINK interface
for the **stm32f103rb** board:
- \subpage page_stm32f103_setuphw
- \subpage page_stm32f103_buildsw
