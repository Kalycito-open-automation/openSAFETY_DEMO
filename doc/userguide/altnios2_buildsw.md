How to build the software {#page_altbuildsw}
============

[TOC]

# Building {#sect_alt_buildsw_build}
After all steps described in chapter \ref page_altbuildhw are carried out and
the file `[toplevel].sopcinfo` exists it is possible to generate the software
for the POWERLINK processor and the user application example. In order to do
this the following build steps need to be executed:

1. Open the `Nios II 13.0sp1 Command Shell` on Windows or a shell where the path
   to Quartus II is set.
2. Inside the shell enter the build directory and execute CMake in the build
   directory with the path to the source directory as a parameter.

       > cd build
       > cmake -G"Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-altera-nios2-gnu.cmake \
       >       -DCFG_NIOS2_QUARTUS_DIR=../src/fpga/boards/altera/[board_name]/[demo-name] ../src

   This will generate Unix style Makefiles for the Altera platform with the default
   configuration. See the section \ref sect_buildsw_options and
   \ref sect_alt_buildsw_options of all available options which can be
   passed to CMake. Simply add `-D[YOUR_OPTION]` to the build command above.

3. After CMake has finished generating the Makefiles it is possible to compile
   the software for your project. This can be done with the following command.

       > make all

   It is also possible to use the generated build system in combination with the
   tool`Nios II 13.0sp1 Software Build Tools for Eclipse`! There execute the
   following steps:
   - Select the `build` folder as the workspace
   - Click on `File -> Import` and select `General -> Existing Projects Into
     Workspace`.
   - Select the folder `build` as your root folder.
   - Import all found projects into your workspace.
   - Build the projects **pcppsi** and **demo-[cn,sn]-gpio** by right clicking
     on the project and selecting `Build`.

> **Note:** It is also possible to use the `Make Target` view to build the project
> by enabling it with `Window -> Show View -> Other -> Make Target`! This view
> shows all predefined make targets for each project which provide all necessary
> build steps.

# Download to Target {#sect_alt_buildsw_download}

The FPGA configuration and application software can be downloaded to the hardware
by using predefined make targets. The following targets are available:

Target                    | Description
--------------------------|------------------------------
download-bits-[CURR_DEMO] | Download the FPGA configuration to the target.
download-elf-[CURR_DEMO]  | Download the software binary to the target.
program-nvm-[CURR_DEMO]   | Generate a flash image with the FPGA configuration and the software binary and write it to the local flash.
erase-nvm-[CURR_DEMO]     | Erase the local flash.

> **Note:** The variable **CURR_DEMO** can be `pcppsi`, `demo-cn-gpio` or `demo-sn-gpio-single`!
> Use `make help` to print all available make targets.

## Application processor - Flash programming   {#sect_targalt_appprogflash}
It is possible to program the application processor software to the parallel flash
of the **terasic-de2-115** by using the **program-nvm** make target. Before this
target is used it is important that the application processor reset vector is
set to the parallel flash in qsys. This can be done with the following steps:
1. Open **fpga/ipcore/altera/qsys/cn_app.qsys** with qsys.
2. Double click on the cpu_1 to open the Nios2 configuration window.
3. Change the reset vector memory to 'Absolute'.
4. Set the 'Reset Vector Offset' to 0x18000000.
5. Save and close in qsys.
6. Redo all steps in \ref page_altbuildhw and \ref page_altbuildsw.
   * Take care to rebuild the main qsys project in `fpga/boards/[platform]/[board_name]/[example_name]`
     not the submodule.

# Target specific configuration option {#sect_alt_buildsw_options}

- **CFG_NIOS2_QUARTUS_DIR**

 Specify the path to the reference hardware project (e.g: ../src/fpga/boards/altera/[board_name]/[demo-name]).

- **ALT_PCP_INSTANCE_ID**

  The instance number of the POWERLINK processor. (Needs to be set for flash programming!)

- **ALT_APP_INSTANCE_ID**

  The instance number of the application processor. (Needs to be set for flash programming!)