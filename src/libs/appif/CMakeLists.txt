 ################################################################################
#
# CMake file of application interface library for PIFA
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

CMAKE_MINIMUM_REQUIRED (VERSION 2.8)

PROJECT (appif C)

# Generate a target for this library in the CMake Makefile
SET(GEN_LIB_TARGET ON)

########################################################################
# Set all source files
########################################################################
SET ( LIB_SRCS
    ${PROJECT_SOURCE_DIR}/ssdo.c
    ${PROJECT_SOURCE_DIR}/cc.c
    ${PROJECT_SOURCE_DIR}/error.c
    ${PROJECT_SOURCE_DIR}/internal.c
    ${PROJECT_SOURCE_DIR}/pdo.c
    ${PROJECT_SOURCE_DIR}/status.c
    ${PROJECT_SOURCE_DIR}/stream.c
    ${PROJECT_SOURCE_DIR}/error.c
)

########################################################################
# Set include paths
########################################################################
SET ( LIB_INCS
    ${PROJECT_SOURCE_DIR}/include
    ${appifcommon${CURR_APPLICATION}_SOURCE_DIR}/include
    ${endian_SOURCE_DIR}/include
    ${DEMO_CONFIG_DIR}/config/tbuf/include
)

IF((CMAKE_SYSTEM_NAME STREQUAL "Generic") AND (CMAKE_SYSTEM_PROCESSOR STREQUAL "Nios2"))
    INCLUDE (nios2.cmake)
ENDIF()

INCLUDE_DIRECTORIES ( ${LIB_INCS} 
                      ${TARGET_DIR}/include
)

########################################################################
# Build library
########################################################################
IF(GEN_LIB_TARGET)
    ADD_LIBRARY ( appif ${LIB_TYPE} ${LIB_SRCS} )

    TARGET_LINK_LIBRARIES ( appif "appifcommon" )
    ADD_DEPENDENCIES ( appif "appifcommon" )

    IF ( ( WIN32 OR CYGWIN ) AND ( ${LIB_TYPE} STREQUAL "SHARED" ) )
        EnsureLibraries(appif "appifcommon" )
    ENDIF ( ( WIN32 OR CYGWIN ) AND ( ${LIB_TYPE} STREQUAL "SHARED" ) )
ENDIF()

################################################################################
# Add clean files
SET_DIRECTORY_PROPERTIES(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${ADD_CLEAN_FILES}")
