POWERLINK Interface IP-Core    {#page_plkif}
============

[TOC]

The POWERLINK interface IP-Core provides different components for exchanging
isochronous process and/or asynchronous service data. It consists of several
subcomponents where some of them are mutual exclusive. These subcomponents can
be categorized into two types of cores. The components who provide the buffered
memory for the process or service data and the communications cores to access
this data from the user application.

A complete overview of all components is pictured the following figure. The dotted
line separates components which can't be used in the same setup.

![POWERLINK interface IP-Core overview](plk_if_ipcore_overview.png)
@image latex plk_if_ipcore_overview.eps "POWERLINK interface IP-Core overview" width=0.6\textwidth

The configuration of the IP-Core can be changed by using the available IP-Core
graphical user interface. This user interface instantiates the subcomponents and
forwards the user configuration to this components. The POWERLINK interface
IP-Core is therefore only a simple configuration core which eases the configuration
and prevents the user from creating a faulty setup.

The POWERLINK interface IP-Core consists of the following subcomponents:
- \subpage page_tbuf : This core provides buffered memories
- \subpage page_spibridge : This core provides a serial interface to the memory

# IP-Core user interface    {#sect_plkif_gui}
The graphical user interface of the IP-Core can be accessed by double clicking
on the instantiation of the core in qsys. See chapter \ref sect_gs_build_fpga_hw_quartus
for more information on how to open a project with qsys.

In the IP-Core user interface the following options are available
* **Internal Memory**: Set the type of internal memory in the IP-Core.
  - **Triple Buffer**: Instantiate the `Triple Buffer core` who provides the buffered memory for the data exchange.
* **Connection Interface**: Change the connection to the memory from serial to parallel.
  - **spi**: Instantiate the `SPI Bridge` core in the system. (Provides a serial interface to the application)
  - **par**: Instantiate the `PAR core` in the system. (Provides a parallel interface to the application)
  - **avalon**: Do not instantiate a connection core in the POWERLINK interface and just wire the `avalon` bus to the toplevel.

# Interface exemplary configuration    {#sect_plkif_example}
An exemplary configuration of the internals of the POWERLINK interface is given
in the next figure.

![Exemplary IP-Core configuration](plk_if_with_tbuf_spi_bridge.png)
@image latex plk_if_with_tbuf_spi_bridge.eps "Exemplary IP-Core configuration" width=0.8\textwidth

This setup for the core consists of the triple buffer component (\ref page_tbuf)
and the SPI bridge (\ref page_spibridge) component. The triple buffer core
provides buffered access to the exchanged data and the SPI core enables a streamed
transfer to and from the data inside the buffers.