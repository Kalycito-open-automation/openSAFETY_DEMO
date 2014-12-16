################################################################################
#
# CMake file of slim interface on demo-cn-gpio for PSI
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

# Generate a target for this library in the CMake Makefile
SET(GEN_LIB_TARGET ON)

PROJECT( demo-${CFG_DEMO_TYPE} C)

########################################################################
# Set all source files
########################################################################
SET ( DEMO_SRCS
    ${PROJECT_SOURCE_DIR}/main.c
    ${APP_COMMON_DIR}/tbufparams.c
)

########################################################################
# Set include paths
########################################################################
SET ( DEMO_INCS
    ${PROJECT_SOURCE_DIR}/include
    ${APP_COMMON_DIR}/include
    ${psi_SOURCE_DIR}/include
    ${psicommonapp_SOURCE_DIR}/include
    ${DEMO_CONFIG_DIR}/tbuf/include
    ${DEMO_CONFIG_DIR}/pcp
    ${TARGET_DIR}/include
)

########################################################################
# Add target specific build information
########################################################################
IF((CMAKE_SYSTEM_NAME STREQUAL "Generic") AND (CMAKE_SYSTEM_PROCESSOR STREQUAL "nios2"))
    INCLUDE(nios2.cmake)
ELSEIF((CMAKE_SYSTEM_NAME STREQUAL "Generic") AND (CMAKE_SYSTEM_PROCESSOR STREQUAL "cortex-m3"))
    IF(CFG_ARM_BOARD_TYPE STREQUAL "stm32f103rb")
        INCLUDE(stm32f103rb.cmake)
    ELSE()
        MESSAGE(FATAL_ERROR "The board type CFG_ARM_BOARD_TYPE=${CFG_ARM_BOARD_TYPE} is not supported by this application!")
    ENDIF()
ELSE()
    MESSAGE(FATAL_ERROR "The selected platform is not supported by this application!")
ENDIF()

########################################################################
# Set include paths
########################################################################
INCLUDE_DIRECTORIES(
                    ${DEMO_INCS}
                    ${DEMO_ARCH_INCS}
)

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
