Target platform Altera/Nios2 {#page_altnios2}
============

[TOC]

If the PSI is built for the target `Altera/Nios2` the evaluation board `TERASIC
Industrial Ethernet Kit` (terasic-de2-115) is required in order to evaluate the
interface. The following software projects are able to run on this target platform:
- `pcp/psi:` The software for the POWERLINK processors.
- `app/demo-cn-gpio:` The software for the POWERLINK CN only demo with GPIO application.
- `app/demo-sn-gpio:` The software for the openSAFETY SN over POWERLINK demo.
   * For this target only a single channelled SN demo is implemented.

> **Note:** As this target is an FPGA each software project requires to compile
> a bitstream in order to run.

# FPGA configurations  {#sect_targalt_configs}
The following **quartus** projects are provided for the target board `terasic-de2-115`:

- `cn-dual-spi-gpio:` CN demo with PCP and application processor in one bitstream.
   Interconnect between the PCP and the application is a SPI serial. The application
   is a simple GPIO demo.
- `cn-pcp-spi:` CN demo with only the POWERLINK processor in the bitstream.
  Interconnect to the external second processor is a SPI serial.

> **Note:** These demos can be found in the folder `src/fpga/boards/altera/terasic-de2-115`
  
These two configurations enable to run a standalone CN/SN on one board and also
a PCP only demo with external application processor. This external processor can
be any of the other provided targets.

# Build steps - Altera/Nios2   {#sect_targalt_buildsteps}
The following steps need to be carried out in order to build the POWERLINK interface
for the Nios2 processor:

- \subpage page_altsetuphw
- \subpage page_altbuildhw
- \subpage page_altbuildsw
