################################################################################
#
# Nios2 configuration options for POWERLINK Interface For Applications
#
# Copyright (c) 2013, Bernecker+Rainer Industrie-Elektronik Ges.m.b.H. (B&R)
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the copyright holders nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
################################################################################

INCLUDE(ConnectCMakeAlteraTargets)
INCLUDE(GenEclipseFileList)

MESSAGE ( STATUS "Generating build files for platform Altera/Nios2 ..." )

###############################################################################
# User settings
OPTION( CFG_SINGLE_BITSTREAM "Both processors are in a single bitstream" ON )

###############################################################################
# Hardware settings
IF( CFG_DEMO_INTERCONNECT MATCHES "spi" AND CFG_SINGLE_BITSTREAM )
    set( QSYS_SYSTEM_NAME cn_SPI_ap_pcp)
    SET( NIOS2_QUARTUS_DIR ${CMAKE_SOURCE_DIR}/fpga/boards/altera/terasic_de2-115/${QSYS_SYSTEM_NAME} )
    SET( QSYS_SYSTEM_FILE ${NIOS2_QUARTUS_DIR}/${QSYS_SYSTEM_NAME}.qsys )
ELSE( CFG_DEMO_INTERCONNECT MATCHES "spi" AND CFG_SINGLE_BITSTREAM )
    MESSAGE( FATAL_ERROR "Only demos with 'spi' interconnect and CFG_SINGLE_BITSTREAM is supported!" )
ENDIF( CFG_DEMO_INTERCONNECT MATCHES "spi" AND CFG_SINGLE_BITSTREAM )

###############################################################################
# Target dependent paths
SET( ALT_MISC_DIR ${MISC_DIR}/altera_nios2 )
SET( ALT_TARGET_DIR ${CMAKE_SOURCE_DIR}/app/target/altera-nios2 )
SET( ALT_DRIVERS_DIR ${FPGA_DIR}/drivers/altera )

SET(ALT_BUILD_DIR_NAME "altera")

# Set path to application board support package -> Needed by the libraries
SET(ALT_APP_BUILD_DIR ${CMAKE_BINARY_DIR}/app/demo-${CFG_DEMO_TYPE}/${ALT_BUILD_DIR_NAME})
SET(ALT_APP_BSP_DIR ${ALT_APP_BUILD_DIR}/bsp)

###############################################################################
# Find Altera Nios2 tools
FIND_PATH( ALT_BSP_GEN_DIR nios2-bsp
           DOC "Path of the Altera Nios2 board support package generator"
         )
MARK_AS_ADVANCED(ALT_BSP_GEN_DIR)

FIND_PROGRAM(ALT_BSP_QUEUE nios2-bsp-query-settings
             DOC "Queue information from a BSP package"
            )
MARK_AS_ADVANCED(ALT_BSP_QUEUE)

FIND_PROGRAM(ALT_APP_GEN_MAKEFILE nios2-app-generate-makefile
             DOC "Generate an application Makefile for the Nios2 processor"
            )
MARK_AS_ADVANCED(ALT_APP_GEN_MAKEFILE)

FIND_PROGRAM(ALT_LIB_GEN_MAKEFILE nios2-lib-generate-makefile
             DOC "Generate a library Makefile for the Nios2 processor"
            )
MARK_AS_ADVANCED(ALT_LIB_GEN_MAKEFILE)

FIND_PROGRAM(ALT_QSYS_SCRIPT qsys-script
             DOC "Queue information from a qsys project"
            )
MARK_AS_ADVANCED(ALT_QSYS_SCRIPT)

###############################################################################
# Set CFLAGS depending on build type
IF( ${CMAKE_BUILD_TYPE} MATCHES "Debug" )
    SET ( DBG_MODE _DEBUG)
    SET ( DEF_DEBUG_LVL 0xEC000000L)
    SET ( OPT_LEVEL -O0)
ELSEIF ( ${CMAKE_BUILD_TYPE} MATCHES "Release" )
    SET ( DBG_MODE NDEBUG)
    SET ( DEF_DEBUG_LVL 0x00000000L)
    SET ( OPT_LEVEL -O2)
ELSEIF ( ${CMAKE_BUILD_TYPE} MATCHES "MinSizeRel" )
    SET ( DBG_MODE NDEBUG)
    SET ( DEF_DEBUG_LVL 0x00000000L)
    SET ( OPT_LEVEL -Os)
ELSE( ${CMAKE_BUILD_TYPE} MATCHES "Debug" )
    SET ( DBG_MODE NDEBUG)
    SET ( DEF_DEBUG_LVL 0x00000000L)
    SET ( OPT_LEVEL -O2)
ENDIF( ${CMAKE_BUILD_TYPE} MATCHES "Debug" )

###############################################################################
# Check plkif ipcore parameters
IF( ${ALT_QSYS_SCRIPT} STREQUAL "ALT_QSYS_SCRIPT-NOTFOUND" )
    MESSAGE( FATAL_ERROR "unexpected: The variable ALT_QSYS_SCRIPT is set to ${ALT_QSYS_SCRIPT}! Start CMake from the nios2 shell to solve this issue!" )
ENDIF ()

SET( QSYS_SCRIPT_COMMAND "set selDemo ${CFG_DEMO_TYPE}\; set qsysSystemName ${QSYS_SYSTEM_NAME}" )

EXECUTE_PROCESS( COMMAND ${ALT_QSYS_SCRIPT} --cmd=${QSYS_SCRIPT_COMMAND} --script=${ALT_MISC_DIR}/scripts/plkif-config.tcl --system-file=${QSYS_SYSTEM_FILE}
                 WORKING_DIRECTORY ${NIOS2_QUARTUS_DIR}
                 RESULT_VARIABLE QSYS_RES
                 OUTPUT_VARIABLE QSYS_STDOUT
                 ERROR_VARIABLE QSYS_STDERR
)

IF( ${QSYS_RES} MATCHES "0" )
    MESSAGE ( STATUS "Change configuration of ${QSYS_SYSTEM_FILE} succeeded!\n${QSYS_STDERR}\n" )
ELSEIF( ${QSYS_RES} MATCHES "2" )
    MESSAGE ( STATUS "\n${QSYS_STDERR}\n" )
    MESSAGE ( SEND_ERROR "POWERLINK interface ipcore buffer layout is not set to demo '${CFG_DEMO_TYPE}'! Change the ipcore configuration and rebuild the FPGA design!" )
ELSE ( ${QSYS_RES} MATCHES "0" )
    MESSAGE ( FATAL_ERROR "qsys-script failed with:\n${QSYS_STDERR}\n" )
ENDIF ( ${QSYS_RES} MATCHES "0" )
