 ################################################################################
#
# CMake file of application interface library (nios2 target) for PIFA
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
FILE( MAKE_DIRECTORY ${ALT_LIBAPPIF_BUILD_DIR} )

########################################################################
# Adapt source file lists and includes
########################################################################
SET( ALT_LIB_SRCS ${LIB_SRCS} )

SET( ALT_LIB_INCS ${LIB_INCS}
                  ${ALT_TARGET_DIR}/include
)

########################################################################
# Library Makefile
########################################################################

SET( LIB_CFLAGS "-D${DBG_MODE} -DDEF_DEBUG_LVL=${DEF_DEBUG_LVL}" )

EXECUTE_PROCESS( COMMAND nios2-lib-generate-makefile --bsp-dir ${ALT_APP_BSP_DIR} --lib-dir ${ALT_LIBAPPIF_BUILD_DIR} --lib-name ${PROJECT_NAME} --set LIB_CFLAGS_DEFINED_SYMBOLS=${LIB_CFLAGS} --set LIB_CFLAGS_OPTIMIZATION=${OPT_LEVEL} --set LIB_INCLUDE_DIRS=${ALT_LIB_INCS} --src-files ${ALT_LIB_SRCS}
                 WORKING_DIRECTORY ${ALT_LIBAPPIF_BUILD_DIR}
                 RESULT_VARIABLE GEN_LIB_RES
                 OUTPUT_VARIABLE GEN_LIB_STDOUT
                 ERROR_VARIABLE GEN_LIB_STDERR
)

IF( NOT ${GEN_LIB_RES} MATCHES "0" )
    MESSAGE ( FATAL_ERROR "nios2-lib-generate-makefile failed with: ${GEN_LIB_STDERR}" )
ENDIF ( NOT  ${GEN_LIB_RES} MATCHES "0" )

MESSAGE ( STATUS "Generate ${PROJECT_NAME} Makefile: ${GEN_LIB_STDOUT}" )

########################################################################
# Eclipse project files
########################################################################
GenEclipseFileList("${ALT_LIB_SRCS}" "" ECLIPSE_FILE_LIST)

CONFIGURE_FILE( ${ALT_MISC_DIR}/project/libproject.in ${ALT_LIBAPPIF_BUILD_DIR}/.project @ONLY )
CONFIGURE_FILE( ${ALT_MISC_DIR}/project/libcproject.in ${ALT_LIBAPPIF_BUILD_DIR}/.cproject @ONLY )
