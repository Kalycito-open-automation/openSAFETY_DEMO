################################################################################
#
# CMake file of slim interface on demo-sn-gpio for PSI
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

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

# Generate a target for this library in the CMake Makefile
SET(GEN_LIB_TARGET ON)

PROJECT(demo-${CFG_DEMO_TYPE} C)

########################################################################
# Set paths
SET(OSFT_BASE_DIR ${CMAKE_SOURCE_DIR}/stacks/openSAFETY)
SET(SN_BASE_DIR ${OSFT_BASE_DIR}/src/eplssrc/SN)
SET(OSCHECKSUM_BASE_DIR ${OSFT_BASE_DIR}/src/tools/oschecksum)

SET(SHNF_DIR ${PROJECT_SOURCE_DIR}/shnf)
SET(SAPL_DIR ${PROJECT_SOURCE_DIR}/sapl)

SET(COMMON_SOURCE_DIR ${PROJECT_SOURCE_DIR}/../common)
SET(SN_TARGET_SOURCE_DIR ${PROJECT_SOURCE_DIR}/target)

########################################################################
# Define the CN unique id
########################################################################
SET(CFG_SAPL_SN_UDID "{0x00U,0x60U,0x65U,0x01U,0x14U,0x9BU}" CACHE STRING "Unique id of this SN")

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DCFG_SAPL_SN_UDID=${CFG_SAPL_SN_UDID}")

########################################################################
# Set all source files
########################################################################
FindSourceFiles("${SN_BASE_DIR}" OSFT_SN_SRCS)

SET(SHNF_SRCS
    ${SHNF_DIR}/shnf.c
    ${SHNF_DIR}/constime.c
    ${SHNF_DIR}/hnfpsi.c
)

SET(SAPL_SRCS
    ${SAPL_DIR}/sapl.c
    ${SAPL_DIR}/parameterset.c
    ${SAPL_DIR}/parametercrc.c
    ${SAPL_DIR}/sodstore.c
    ${SAPL_DIR}/stackerrorhandler.c
)

SET(LIBOSCHECKSUM_SRCS
    ${OSCHECKSUM_BASE_DIR}/crc.c
    ${OSCHECKSUM_BASE_DIR}/crc_protocol.c
)

SET(SN_SRCS
    ${PROJECT_SOURCE_DIR}/main.c
    ${APP_COMMON_DIR}/tbufparams.c
    ${PROJECT_SOURCE_DIR}/errorhandler.c
    ${DEMO_CONFIG_DIR}/sn/sod.c
)

SET(DEMO_SRCS
                ${SN_SRCS}
                ${SHNF_SRCS}
                ${SAPL_SRCS}
                ${OSFT_SN_SRCS}
                ${LIBOSCHECKSUM_SRCS}
)

########################################################################
# Set include paths
########################################################################
SET(DEMO_INCS
    ${PROJECT_SOURCE_DIR}/include
    ${COMMON_SOURCE_DIR}/include
    ${SHNF_DIR}/include
    ${SAPL_DIR}/include
    ${psi_SOURCE_DIR}/include
    ${psicommonapp_SOURCE_DIR}/include
    ${DEMO_CONFIG_DIR}/tbuf/include
    ${DEMO_CONFIG_DIR}/sn
    ${DEMO_CONFIG_DIR}/pcp
    ${SN_BASE_DIR}
    ${OSCHECKSUM_BASE_DIR}/include
    ${TARGET_DIR}/include
)

IF((CMAKE_SYSTEM_NAME STREQUAL "Generic") AND (CMAKE_SYSTEM_PROCESSOR STREQUAL "nios2"))
    INCLUDE(nios2.cmake)
ELSEIF((CMAKE_SYSTEM_NAME STREQUAL "Generic") AND (CMAKE_SYSTEM_PROCESSOR STREQUAL "cortex-m3"))
    IF(CFG_ARM_BOARD_TYPE STREQUAL "stm32f10x")
        INCLUDE(stm32f103.cmake)
    ELSE()
        MESSAGE(FATAL_ERROR "The board type CFG_ARM_BOARD_TYPE=${CFG_ARM_BOARD_TYPE} is not supported by this application!")
    ENDIF()
ELSE()
    MESSAGE(FATAL_ERROR "The selected platform is not supported by this application!")
ENDIF()

INCLUDE_DIRECTORIES(
                    ${DEMO_INCS}
                    ${DEMO_ARCH_INCS}
)

IF("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
    # Remove unknown pragma warning (This disables a warning on CTC_SKIP)
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unknown-pragmas")
ENDIF()

########################################################################
# Build Executable
########################################################################
IF(GEN_LIB_TARGET)
    ADD_EXECUTABLE(${PROJECT_NAME}.elf ${DEMO_SRCS} ${DEMO_ARCH_SRCS})

    SET_PROPERTY(TARGET ${PROJECT_NAME}.elf PROPERTY LINK_FLAGS ${DEMO_ARCH_LINKER_FLAGS})
    TARGET_LINK_LIBRARIES(${PROJECT_NAME}.elf "psi" "psicommonapp")

    # Call target post actions (Target specific!)
    AppPostAction(${PROJECT_NAME}.elf)
ENDIF()

################################################################################
# Add clean files
SET_DIRECTORY_PROPERTIES(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${ADD_CLEAN_FILES}")
