Target platform Altera/Nios2 {#page_altnios2}
============

[TOC]

If the demo is built for the target `Altera/Nios2` the evaluation board `TERASIC
Industrial Networking Kit` (terasic-de2-115) is required in order to evaluate the
interface. The following software projects are able to run on this target platform:
- `pcp/psi:` The software for the POWERLINK processors.

> **Note:** As this target is an FPGA each software project requires to compile
> a bitstream in order to run.

# FPGA configurations  {#sect_targalt_configs}
The following **quartus** project is provided for the target board `terasic-de2-115`:

- `cn-pcp-spi:` CN demo with only the POWERLINK processor in the bitstream.
  Interconnect to the external second processor is a SPI serial.

> **Note:** This demo can be found in the folder `src/fpga/boards/altera/terasic-de2-115`

This configuration enables to run a PCP only demo with external application
processor. This external processor can be any of the other provided targets.

# Build steps - Altera/Nios2   {#sect_targalt_buildsteps}
The following steps need to be carried out in order to build the POWERLINK interface
for the Nios2 processor:

- \subpage page_altsetuphw
- \subpage page_altbuildhw
- \subpage page_altbuildsw
