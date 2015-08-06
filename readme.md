Introduction {#mainpage}
=============

[TOC]

# openSAFETY_DEMO   {#sect_main_intro}

The openSAFETY_DEMO is an example implementation of an openSAFETY Safe Node (SN)
using POWERLINK as the underlying communication network. It is intended for
people interested in openSAFETY, for evaluation purposes of openSAFETY and as
a very first starting point for SN development.

The demo application consists of one part executing the openSAFETY stack and
the user application (application processor) and a second part for handling the
POWERLINK communication (POWERLINK Communication Processor, PCP) which is
implemented on a FPGA platform.

The software for the application part implements a single and a dual channelled
solution. The single channelled application uses one application processor,
the dual channelled application uses two application processors which receive
the same data, exchange data among them and verify each others produced output.

The partitioning of the system into application and network part allows a good
separation of safety related and non-safety related code as well as the exchange
of the underlying network interface.

This demo uses a lightweight modular interface to connect the application
processor to the PCP and the POWERLINK network behind this processor.

This documentation describes the software API of the interface between the
POWERLINK Communication Processor (PCP) and the user application. In this setup
the PCP itself handles the complete processing of the POWERLINK protocol.
The interface to the user application only provides the process and service
data. As this interface is considered to be lightweight and modular,
it is therefore limited in it's functionality. (See \ref sect_sysdesc_oS_demo_limits)

> This software distribution uses the POWERLINK and openSAFETY protocols as a basis
> for communication. More information about this protocols are provided in
> \ref page_protocols.

In order to provide a better understanding of the interface an example
implementation of the user application is provided. Currently the following
application is available:
- **sn-gpio**: This demo implements an openSAFETY SN with a simple
  GPIO application. (See \ref sect_sysdesc_app_sndemo)

For processing the POWERLINK protocol on the PCP the open source protocol stack
`openPOWERLINK V2.x` is used. This stack is already integrated in this
distribution. For more information about openPOWERLINK checkout the doxygen
documentation of the stack in the folder
`blackchannel/POWERLINK/stacks/openPOWERLINK/doc`.

The application processor implements a Safe Node which executes the openSAFETY
protocol stack. It can be downloaded from http://sourceforge.net/projects/opensafety.

> In order to get a better overview of the whole system and it's different
> setups please investigate \ref page_sysdesc.

# License   {#sect_main_license}

The openSAFETY_DEMO is open-source software (OSS) and is licensed under the
BSD license.
Please refer to the file's header and the file [\"license.md\"](\ref page_licenses)
for the applicable license and the corresponding terms and conditions.

# Documentation   {#sect_main_documentation}

* The documentation of the openSAFETY_DEMO is located in the
  subdirectory "doc". It is written in _markdown_ markup format and can be read
  by opening the `.md` with a text editor.
* The software manual can be generated from the markdown
  documentation and the in source-code documentation with the tools
  [Doxygen](http://www.doxygen.org) and [CMake](http://www.cmake.org). Therefore
  Doxygen version greater or equal 1.8 is required. The software manual will be
  created in HTML or pdf (Latex) format under `build/doc/[html/latex]`.
  [Graphviz] (http://www.graphviz.org/) is used for generating graph
  visualisations and diagrams.

  To generate the documentation enter the following commands according to your
  host operating system.

## Build command on Windows
Open the `Visual Studio Command Prompt` and enter the following commands:

      > cd build
      > cmake -G"NMake Makefiles" ../
      > nmake doc


## Build command on Linux
Open a terminal there the path to `make`, `doxygen` and `CMake` is set and enter
the following commands:

      > cd build
      > cmake -G"Unix Makefiles" ../
      > make doc
