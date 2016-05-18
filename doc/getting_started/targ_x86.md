Target platform x86/amd64 (Windows/Linux) {#page_gs_x86}
============

[TOC]

If the target platform is the local machine with a Windows or Linux operating
system all demo projects are deactivated and it is only possible to build and
execute the unit tests and to generate the documentation.

All unit tests for the PSI project use the testing framework CUnit and are able
to be run with **CTest**. (CTest is the CMake testing framework). All tests are
located in the folder **blackchannel/POWERLINK/unittest**.

# Configuration options - x86   {#sect_gs_targx86_options}
In order to configure the build on your local workstation the following CMake
configuration options are available:

- **UNITTEST_ENABLE**

  Enable/Disable the whole unit testing framework.

- **UNITTEST_XML_REPORT**

  Enable/Disable the generation of a test report in XML format.

# CMake Generators {#sect_gs_targx86_generators}
The following CMake generators are tested for the PSI project on Windows:

- **Visual Studio 10:** Generates a Visual Studio solution and uses the MSVC compiler
- **NMake Makefile:** Generates a Makefile which can be used in the Visual Studio command shell (Uses the MSVC compiler)
- **Msys Makefile:** Can be used in the MSYS Shell (Provided with MinGw) under Windows and is used with the GNU Compiler Collection (GCC)
- **WinGw Makefile:** Can be used in a Windows standard command shell where the MinGW GCC is configured.

On Linux the PSI project can be built by using the `Unix Makefile` generator with
the locally installed GCC compiler.
