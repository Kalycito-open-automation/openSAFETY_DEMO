Directory structure {#page_directories}
===================

[TOC]

This page describes the directory structure of the POWERLINK interface source directory.
It gives details about the content of each folder and also adds other useful information.

# Main directories {#sect_directories_main}
The POWERLINK slim interface sources are located in the folder **src** in the
release package. It consists of all the source code needed in order to run
a provided demo application. The following folders are given in the source directory.

Directory                     | Description
----------------------------- | -----------------------------------------------
app                           | Demo applications for evaluating the POWERLINK interface (See \ref sect_directories_examples)
cmake                         | Files for the CMake build tool
contrib                       | Additional files needed by a platform or a tool
fpga                          | FPGA specific stuff like IP cores and VHDL code
libs                          | POWERLINK interface libraries (See also \ref sect_directories_libraries)
misc                          | Miscellaneous tools, scripts and utilities
pcp                           | POWERLINK interface PCP drivers (Driver of the POWERLINK stack)
stacks                        | The protocol stacks used in this project (See \ref sect_directories_stack)
unittests                     | CUnit Unit tests and framework

## Examples {#sect_directories_examples}
A set of demo applications is included in the POWERLINK interface which demonstrate
the usage of the user library.

Directory                     | Description
----------------------------- | -----------------------------------------------
demo-cn-gpio                  | A simple demo for reading and writing GPIO pins (See \ref sect_sysdesc_app_cndemo)
demo-sn-gpio                  | A lightweight openSAFETY SN demo (See \ref sect_sysdesc_app_sndemo)


### Configuration files {#sect_directories_configuration}

Each demo consists of a directory called **config**. It contains the CANopen
object dictionaries and the triple buffer configuration headers.

Directory                     | Description
----------------------------- | -----------------------------------------------
pcp                           | POWERLINK processor configuration files. (Object dictionary and xdd)
tbuf                          | Configuration headers for the triple buffers


## Libraries {#sect_directories_libraries}
The __libs__ directory contains libraries used by the POWERLINK interface examples.

Directory                     | Description
----------------------------- | -----------------------------------------------
psi                           | Slim interface user library
psicommon                     | Slim interface common library (Used by POWERLINK and the user application)


## Stack sources {#sect_directories_stack}

This directory contains the source code of the stacks which are included in the
POWERLINK interface.

Directory                     | Description
----------------------------- | -----------------------------------------------
openPOWERLINK                 | The openPOWERLINK stack sources needed by the POWERLINK processor
openSAFETY                    | The openSAFETY stack sources needed by the safety demo
ip                            | A lightweight ip stack needed by the POWERLINK processor
