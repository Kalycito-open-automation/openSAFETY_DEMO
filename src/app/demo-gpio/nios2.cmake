################################################################################
#
# CMake file of application interface on demo-gpio (nios2 target) for PIFA
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

##########################################################################
# Set build directory for the Altera Makefile
SET(ALT_BUILD_DIR ${PROJECT_BINARY_DIR}/${ALT_BUILD_DIR_NAME})

##########################################################################
# Set build parameters
SET(NIOS2_QSYS_SUB app_0)
SET(NIOS2_QSYS_SUB_CPU cpu_1)

SET( BSP_TYPE hal )

########################################################################
# Adapt source file lists and includes
########################################################################
SET( ALT_DEMO_SRCS ${DEMO_SRCS}
                   ${ALT_TARGET_DIR}/platform.c
                   ${ALT_TARGET_DIR}/app-gpio.c
                   ${ALT_DRIVERS_DIR}/avalon_spi/avalon_spi.c
)

SET( ALT_DEMO_INCS ${DEMO_INCS}
                   ${ALT_APP_BSP_DIR}
                   ${ALT_APP_BSP_DIR}/HAL/inc
                   ${ALT_APP_BSP_DIR}/drivers/inc
                   ${ALT_DRIVERS_DIR}/avalon_spi/include
                   ${ALT_TARGET_DIR}/include
)

########################################################################
# Board Support Package
########################################################################

SET( NIOS2_BSP_ARGS
                    "--cpu-name ${NIOS2_QSYS_SUB}_${NIOS2_QSYS_SUB_CPU}"
                    "--set hal.enable_c_plus_plus false"
                    "--set hal.linker.enable_alt_load_copy_exceptions false"
                    "--set hal.enable_clean_exit false"
                    "--set hal.enable_exit false"
                    "--set hal.make.bsp_cflags_optimization ${OPT_LEVEL}"
   )

IF( ${CMAKE_BUILD_TYPE} MATCHES "Release" )
    SET( NIOS2_BSP_ARGS "${NIOS2_BSP_ARGS} --set hal.stdout none --set hal.stderr none" )
ENDIF ( ${CMAKE_BUILD_TYPE} MATCHES "Release" )

EXECUTE_PROCESS( COMMAND bash ${ALT_BSP_GEN_DIR}/nios2-bsp ${BSP_TYPE} ${ALT_APP_BSP_DIR} ${NIOS2_QUARTUS_DIR} ${NIOS2_BSP_ARGS}
                 WORKING_DIRECTORY ${ALT_BUILD_DIR}
                 RESULT_VARIABLE GEN_BSP_RES
                 OUTPUT_VARIABLE GEN_BSP_STDOUT
                 ERROR_VARIABLE GEN_BSP_STDERR
)

IF( NOT ${GEN_BSP_RES} MATCHES "0" )
    MESSAGE ( FATAL_ERROR "nios2-bsp generation error: ${GEN_BSP_STDERR}" )
ENDIF ( NOT ${GEN_BSP_RES} MATCHES "0" )

MESSAGE ( STATUS  "Generate board support package: ${GEN_BSP_STDOUT}" )

########################################################################
# Application Makefile
########################################################################

SET( APP_CFLAGS "${CFLAGS} -D${DBG_MODE} -DDEF_DEBUG_LVL=${DEF_DEBUG_LVL}" )

SET( ALT_LIB_GEN_ARGS
                      "--bsp-dir ${ALT_APP_BSP_DIR}"
                      "--set QUARTUS_PROJECT_DIR=${NIOS2_QUARTUS_DIR}"
                      "--src-files ${ALT_DEMO_SRCS}"
                      "--set APP_INCLUDE_DIRS=${ALT_DEMO_INCS}"
                      "--set APP_CFLAGS_DEFINED_SYMBOLS"
                      "--set QSYS_SUB_CPU=${NIOS2_QSYS_SUB_CPU}"
                      "--set OBJDUMP_INCLUDE_SOURCE=1"
                      "--set CREATE_OBJDUMP=0"
                      "--set CFLAGS=${APP_CFLAGS}"
                      "--set APP_CFLAGS_OPTIMIZATION=${OPT_LEVEL}"
                      "--elf-name ${PROJECT_NAME}.elf"
                      "--use-lib-dir ${endian_BINARY_DIR}/${ALT_BUILD_DIR_NAME}"
                      "--use-lib-dir ${appif_BINARY_DIR}/${ALT_BUILD_DIR_NAME}"
                      "--use-lib-dir ${appifcommon_BINARY_DIR}/${ALT_BUILD_DIR_NAME}"
   )

EXECUTE_PROCESS( COMMAND ${ALT_APP_GEN_MAKEFILE} ${ALT_LIB_GEN_ARGS}
                 WORKING_DIRECTORY ${ALT_BUILD_DIR}
                 RESULT_VARIABLE GEN_MAKE_RES
                 OUTPUT_VARIABLE GEN_MAKE_STDOUT
                 ERROR_VARIABLE GEN_MAKE_STDERR
)

IF( NOT ${GEN_MAKE_RES} MATCHES "0" )
    MESSAGE ( FATAL_ERROR "${ALT_APP_GEN_MAKEFILE} failed with: ${GEN_MAKE_STDERR}" )
ENDIF ( NOT  ${GEN_MAKE_RES} MATCHES "0" )

MESSAGE ( STATUS "Generate application Makefile: ${GEN_MAKE_STDOUT}" )

# Fix Makefile issues
EXECUTE_PROCESS( COMMAND bash ${ALT_MISC_DIR}/scripts/fix_makefile.sh
                 WORKING_DIRECTORY ${ALT_BUILD_DIR}
                 RESULT_VARIABLE FIX_RES
                 OUTPUT_VARIABLE FIX_STDOUT
                 ERROR_VARIABLE FIX_STDERR
)

IF( NOT ${FIX_RES} MATCHES "0" )
    MESSAGE ( FATAL_ERROR "Failed to fix Makefile with: ${FIX_STDERR}" )
ENDIF ( NOT  ${FIX_RES} MATCHES "0" )

########################################################################
# Connect the CMake Makefile with the Altera Makefile
########################################################################
ConnectCMakeAlteraExeTargets(${PROJECT_NAME} ${ALT_BUILD_DIR})

########################################################################
# Eclipse project files
########################################################################
GenEclipseFileList("${ALT_DEMO_SRCS}" "" ECLIPSE_FILE_LIST )

CONFIGURE_FILE(${ALT_MISC_DIR}/project/appproject.in ${ALT_BUILD_DIR}/.project @ONLY)
CONFIGURE_FILE(${ALT_MISC_DIR}/project/appcproject.in ${ALT_BUILD_DIR}/.cproject @ONLY)
