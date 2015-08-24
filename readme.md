Introduction {#mainpage}
=============

[TOC]

# openSAFETY_DEMO   {#sect_main_intro}

The openSAFETY_DEMO is an example implementation of an openSAFETY Safe Node (SN)
with a simple GPIO application using POWERLINK as the underlying
communication network. It is intended for people interested in openSAFETY,
for evaluation purposes of openSAFETY and as
a very first starting point for SN development.

The demo application consists of one part for handling the
POWERLINK communication (POWERLINK Communication Processor, PCP) which is
implemented on a FPGA platform and a second part (application processor) executing the
the user application, such as the openSAFETY stack and the safe application.

The user application part can be used in a single and a dual channelled
configuration. The single channelled configuration uses one application
processor, the dual channelled configuration additionally uses a second redundant
application processor.

The partitioning of the system into application and network part allows a good
separation of safety related and non-safety related code as well as the exchange
of the underlying network interface.

> In order to get a better overview of the whole system and it's setup,
> please investigate \ref page_sysdesc.


# Protocol Stacks

This software distribution uses the POWERLINK and openSAFETY protocols as a basis
for communication. More information about this protocols is provided in
\ref page_protocols.

For processing the POWERLINK protocol on the PCP the open source protocol stack
`openPOWERLINK V2.x` is used. This stack is already integrated in this
distribution. For more information about openPOWERLINK checkout the doxygen
documentation of the stack in the folder
`blackchannel/POWERLINK/stacks/openPOWERLINK/doc`.

The openSAFETY protocol stack is processed on the application processor.
This open source stack can be downloaded from
http://sourceforge.net/projects/opensafety.



# License   {#sect_main_license}

The openSAFETY_DEMO is open-source software (OSS) and is licensed under the
BSD license.
Please refer to the file's header and the file [\"license.md\"](\ref page_licenses)
for the applicable license and the corresponding terms and conditions.

# Documentation   {#sect_main_documentation}

## Scope of the openSAFETY_DEMO Documentation

This documentation provides demo-specific information on the used hard-
and software, how to put the demo into operation and the demo's
customisation possibilities.
With SN development in mind, this documentation also gives generalised
information on the hardware design, software design and the software API of the
interface between the POWERLINK Communication Processor (PCP) and the user
application.


## Location and Generation

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
