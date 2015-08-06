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
       >       -DCFG_NIOS2_QUARTUS_DIR=../blackchannel/POWERLINK/fpga/boards/altera/[board_name]/[demo-name] ../

   This will generate Unix style Makefiles for the Altera platform with the default
   configuration. See the section \ref sect_buildsw_options and
   \ref sect_alt_buildsw_options of all available options which can be
   passed to CMake. Simply add `-D[YOUR_OPTION]` to the build command above.

3. After CMake has finished generating the Makefiles it is possible to compile
   the software for your project. This can be done with the following command.

       > make all

   It is also possible to use the generated build system in combination with the
   tool `Nios II 13.0sp1 Software Build Tools for Eclipse`. There execute the
   following steps:
   - Select the `build` folder as the workspace
   - Click on `File -> Import` and select `General -> Existing Projects Into
     Workspace`.
   - Select the folder `build` as your root folder.
   - Import all found projects into your workspace.
   - Build the projects **pcppsi** and **demo-sn-gpio** by right clicking
     on the project and selecting `Build`.

> **Note:** It is also possible to use the `Make Target` view to build the project
> by enabling it with `Window -> Show View -> Other -> Make Target`. This view
> shows all predefined make targets for each project which provide all necessary
> build steps.

# Download to Target {#sect_alt_buildsw_download}

The FPGA configuration and application software can be downloaded to the hardware
by using predefined make targets. The following targets are available:

Target                    | Description
--------------------------|------------------------------
download-bits-pcppsi | Download the FPGA configuration to the target.
download-elf-pcppsi  | Download the software binary to the target.
program-nvm-pcppsi   | Generate a flash image with the FPGA configuration and the software binary and write it to the local flash.
erase-nvm-pcppsi     | Erase the local flash.
flash_default_fpga_image | Write the pre-built image contained in the **bin** directory to the local flash.


> **Note:** Use `make help` to print all available make targets.

> **Note:** Make targets download-bits-pcppsi and download-elf-pcppsi
> download configuration and software to volatile memory, which are therefore
> lost after a power cycle of the FPGA board. To avoid a possible mixup for
> configurations and software in volatile and non-volatile memory, flash memory
> may be erased before downloading to volatile memory.

## Downloading with Altera Quartus Web-Edition {#sect_alt_buildsw_download_web}

Since the Web-Edition of Quartus Software generates a time-limited
bitstream, it is not possible to use the target `program-nvm-pcppsi` to
download the bitstream and the software to the non-volatile memory.
> **Note:** Downloading the pre-built bitstream and software is not affected
> by the Web-Edition of Quartus and can be done in the described way
> (See \ref page_flashing).

Executing the target will download the bitstream and block the terminal
until the programmer is left by pressing `q`.

To download the software, either use the Quartus Programmer for downloading
the bitstream and keeping the connection to the FPGA board alive and execute

        > make download-elf-pcppsi

from the Nios II Command Shell, or use one Nios II Command Shell to download the bitstream via

        > make download-bits-pcppsi

and open a second Nios II Command Shell for executing

        > make download-elf-pcppsi


> **Note:** As the FPGA bitstream and the software will be downloaded to
> the volatile memory, a power cycle will lead to a loss of the data.
> This has to be considered if a power cycle of the application processor(s)
> is needed.

# Target specific configuration option {#sect_alt_buildsw_options}

- **CFG_NIOS2_QUARTUS_DIR**

 Specify the path to the reference hardware project (e.g: blackchannel/POWERLINK/fpga/boards/altera/[board_name]/[demo-name]).

- **ALT_PCP_INSTANCE_ID**

  The instance number of the POWERLINK processor. (Needs to be set for flash programming!)
