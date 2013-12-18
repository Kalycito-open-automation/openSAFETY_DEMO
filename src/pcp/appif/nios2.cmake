################################################################################
#
# CMake file of application interface on pcp (nios2 target) for PIFA
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
# Create build directory
SET( ALT_PCP_BUILD_DIR ${ALT_BUILD_DIR}/pcp/appif )
FILE( MAKE_DIRECTORY ${ALT_PCP_BUILD_DIR} )

SET( NIOS2_QSYS_SUB pcp_0 )
SET( NIOS2_QSYS_SUB_CPU cpu_0 )

SET( BSP_TYPE hal )
SET( BSP_DIR ${ALT_PCP_BUILD_DIR}/bsp )

SET( NIOS2_TC_I_MEM ${NIOS2_QSYS_SUB}_tc_i_mem )

# Create altera source list
SET( ALT_APPIF_SRCS ${APPIF_SRCS} ${IPSTACK_SRCS} ${PLKSTACK_SRCS} )

# Create altera include list
SET( ALT_APPIF_INCS  ${APPIF_INCS} ${BSP_DIR} ${BSP_DIR}/HAL/inc ${BSP_DIR}/drivers/inc )

########################################################################
# Board Support Package
########################################################################

SET( NIOS2_BSP_ARGS "--set hal.sys_clk_timer ${NIOS2_QSYS_SUB}_timer_0 --cpu-name ${NIOS2_QSYS_SUB}_${NIOS2_QSYS_SUB_CPU} --set hal.enable_c_plus_plus false --set hal.linker.enable_alt_load_copy_exceptions false --set hal.enable_clean_exit false --set hal.enable_exit false --cmd add_section_mapping .tc_i_mem_pcp ${NIOS2_TC_I_MEM} --set hal.make.bsp_cflags_optimization ${OPT_LEVEL}" )

IF( ${CMAKE_BUILD_TYPE} MATCHES "Release" )
    SET( NIOS2_BSP_ARGS "${NIOS2_BSP_ARGS} --set hal.stdout none --set hal.stderr none" )
ENDIF ( ${CMAKE_BUILD_TYPE} MATCHES "Release" )

EXECUTE_PROCESS( COMMAND bash ${BSP_GEN_DIR}/nios2-bsp ${BSP_TYPE} ${BSP_DIR} ${NIOS2_QUARTUS_DIR} ${NIOS2_BSP_ARGS}
                 WORKING_DIRECTORY ${ALT_PCP_BUILD_DIR}
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

# Get TCI memory size
EXECUTE_PROCESS( COMMAND nios2-bsp-query-settings --settings ${BSP_DIR}/settings.bsp --cmd puts [get_addr_span ${NIOS2_TC_I_MEM}]
                 WORKING_DIRECTORY ${ALT_PCP_BUILD_DIR}
                 RESULT_VARIABLE GET_TCM_RES
                 OUTPUT_VARIABLE GET_TCM_STDOUT
                 ERROR_VARIABLE GET_TCM_STDERR
                 OUTPUT_STRIP_TRAILING_WHITESPACE
)

IF( NOT ${GET_TCM_RES} MATCHES "0" )
    MESSAGE ( FATAL_ERROR "nios2-bsp-query-settings failed with: ${GET_TCM_STDERR}" )
ENDIF ( NOT  ${GET_TCM_RES} MATCHES "0" )

SET( TCI_MEM_SIZE ${GET_TCM_STDOUT} )

MESSAGE( STATUS "Size TCIMEM: ${TCI_MEM_SIZE}" )

# Generate application Makefile
SET( APP_CFLAGS "-D${DBG_MODE} -DDEF_DEBUG_LVL=${DEF_DEBUG_LVL} -DCONFIG_CN" )

EXECUTE_PROCESS( COMMAND nios2-app-generate-makefile --bsp-dir ${BSP_DIR} --set QUARTUS_PROJECT_DIR=${NIOS2_QUARTUS_DIR} --src-files ${ALT_APPIF_SRCS} --set APP_INCLUDE_DIRS=${ALT_APPIF_INCS} --set APP_CFLAGS_DEFINED_SYMBOLS -DALT_TCIMEM_SIZE=${TCI_MEM_SIZE} -DAPPIF_BUILD_PCP --set QSYS_SUB_CPU ${NIOS2_QSYS_SUB_CPU} --set OBJDUMP_INCLUDE_SOURCE 1 --set CREATE_OBJDUMP 0 --set CFLAGS=${APP_CFLAGS} --set APP_CFLAGS_OPTIMIZATION ${OPT_LEVEL} --elf-name ${PROJECT_NAME}.elf
                 WORKING_DIRECTORY ${ALT_PCP_BUILD_DIR}
                 RESULT_VARIABLE GEN_MAKE_RES
                 OUTPUT_VARIABLE GEN_MAKE_STDOUT
                 ERROR_VARIABLE GEN_MAKE_STDERR
)

IF( NOT ${GEN_MAKE_RES} MATCHES "0" )
    MESSAGE ( FATAL_ERROR "nios2-app-generate-makefile failed with: ${GEN_MAKE_STDERR}" )
ENDIF ( NOT  ${GEN_MAKE_RES} MATCHES "0" )

MESSAGE ( STATUS "Generate application Makefile: ${GEN_MAKE_STDOUT}" )

# Fix Makefile issues
EXECUTE_PROCESS( COMMAND bash ${ALT_MISC_DIR}/scripts/fix_makefile.sh
                 WORKING_DIRECTORY ${ALT_PCP_BUILD_DIR}
                 RESULT_VARIABLE FIX_RES
                 OUTPUT_VARIABLE FIX_STDOUT
                 ERROR_VARIABLE FIX_STDERR
)

IF( NOT ${FIX_RES} MATCHES "0" )
    MESSAGE ( FATAL_ERROR "Failed to fix Makefile with: ${FIX_STDERR}" )
ENDIF ( NOT  ${FIX_RES} MATCHES "0" )

# Copy triple buffer configuration file
FILE( MAKE_DIRECTORY ${DEMO_CONFIG_DIR}/config/tbuf/include/ipcore )
FILE( COPY ${BSP_DIR}/tbuf-cfg.h DESTINATION ${DEMO_CONFIG_DIR}/config/tbuf/include/ipcore )

########################################################################
# Eclipse project files
########################################################################
GenEclipseFileList("${APPIF_SRCS}" "" PART_ECLIPSE_FILE_LIST )
SET( ECLIPSE_FILE_LIST "${ECLIPSE_FILE_LIST} ${PART_ECLIPSE_FILE_LIST}" )

GenEclipseFileList("${IPSTACK_SRCS}" "ip" PART_ECLIPSE_FILE_LIST )
SET( ECLIPSE_FILE_LIST "${ECLIPSE_FILE_LIST} ${PART_ECLIPSE_FILE_LIST}" )

GenEclipseFileList("${PLKSTACK_SRCS}" "plk" PART_ECLIPSE_FILE_LIST )
SET( ECLIPSE_FILE_LIST "${ECLIPSE_FILE_LIST} ${PART_ECLIPSE_FILE_LIST}" )

CONFIGURE_FILE( ${ALT_MISC_DIR}/project/appproject.in ${ALT_PCP_BUILD_DIR}/.project @ONLY )
CONFIGURE_FILE( ${ALT_MISC_DIR}/project/appcproject.in ${ALT_PCP_BUILD_DIR}/.cproject @ONLY )
