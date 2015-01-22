Introduction {#mainpage}
=============

[TOC]

# POWERLINK slim interface (PSI)   {#sect_main_intro}

The POWERLINK slim interface is a lightweight modular interface to the POWERLINK
processor and the POWERLINK network behind this processor. It implements a
Controlled Node (CN) on an FPGA platform which contains the whole functionality
necessary to run a POWERLINK device including both, the data link layer
(POWERLINK cycle handling) as well as the application layer (CANopen part).

This documentation describes the software API of the interface between the
POWERLINK Communication Processor (PCP) and the user application. In this setup
the PCP itself handles the complete processing of the POWERLINK protocol. The interface
to the user application only provides the process and service data. This interface
is considered to be lightweight and modular and is therefore limited in it's
functionality. (See \ref sect_sysdesc_psi_limits)

> This software distribution uses the POWERLINK and openSAFETY protocols as a basis
> for communication. More information about this protocols are provided in
> \ref page_protocols.

In order to provide a better understanding of the interface several example
implementations of the user application are provided. Currently the following
applications are available:
- **cn-gpio**: This example implements a simple four byte digital in/out
  application which reads the input data from node switches and writes the
  output to user leds. (See \ref sect_sysdesc_app_cndemo)
- **sn-gpio**: This more complex demo implements an openSAFETY SN with a simple
  GPIO application. (See \ref sect_sysdesc_app_sndemo)

For processing the POWERLINK protocol on the PCP the open source protocol stack
`openPOWERLINK V2.x` is used. This stack is free of charge and can be downloaded
from https://sourceforge.net/projects/openpowerlink/. For more information about
openPOWERLINK checkout the doxygen documentation of the stack in the folder
`src/stacks/openPOWERLINK/doc`.

If the application processor implements a Safe Node the openSAFETY protocol
stack is executed on this processor. Then the PSI is used to provide the interface
from the openSAFETY stack to the POWERLINK protocol. The openSAFETY stack
can be downloaded from http://sourceforge.net/projects/opensafety.

> In order to get a better overview of the whole system and it's different
> setups please investigate \ref page_sysdesc.

# License   {#sect_main_license}

The POWERLINK slim interface is open-source software (OSS) and is licensed under the
BSD license.
Please refer to the file's header and the file [\"license.md\"](\ref page_licenses)
for the applicable license and the corresponding terms and conditions.

# Documentation   {#sect_main_documentation}

* The documentation of the POWERLINK slim interface is located in the
  subdirectory "doc". It is written in _markdown_ markup format and can be read
  by opening the `.md` with a text editor.
* The software manual can be generated from the markdown
  documentation and the in source-code documentation with the tools
  [Doxygen](http://www.doxygen.org) and [CMake](http://www.cmake.org). Therefore
  Doxygen version greater or equal 1.8 is required. The software manual will be
  created in HTML or pdf (Latex) format under `build/doc/[html/latex]`.

  To generate the documentation enter the following commands according to your
  host operating system.

## Build command on Windows
Open the `Visual Studio Command Prompt` and enter the following commands:

      > cd build
      > cmake -G"NMake Makefiles" ../src
      > nmake doc


## Build command on Linux
Open a terminal there the path to `make`, `doxygen` and `CMake` is set and enter
the following commands:

      > cd build
      > cmake -G"Unix Makefiles" ../src
      > make doc
