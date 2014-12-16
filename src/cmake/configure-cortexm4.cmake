################################################################################
#
# Cortex-M4 configuration options for POWERLINK Slim Interface
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

MESSAGE ( STATUS "Generating build files for platform ARM/Cortex-M4 ..." )

################################################################################
# Handle target specific includes
SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/../cmake/cortex-mx" ${CMAKE_MODULE_PATH})

INCLUDE(AppPostAction)

################################################################################
# User option: Select the type of Cortex-M3 board
IF(NOT CFG_ARM_BOARD_TYPE)
    SET(CFG_ARM_BOARD_TYPE stm32f401re CACHE STRING
        "Select the cortex-m4 target board type"
        FORCE
    )
    SET_PROPERTY(CACHE CFG_ARM_BOARD_TYPE PROPERTY STRINGS "stm32f401re")
ENDIF(NOT CFG_ARM_BOARD_TYPE)

OPTION(CFG_BENCHMARK_ENABLED ON "Enable application benchmark module")

################################################################################
# This target only supports application style projects
SET(CFG_INCLUDE_SUBPROJECTS "application")

################################################################################
# Set path to target specific files
SET( TARGET_DIR ${APP_TARGET_DIR}/${CFG_ARM_BOARD_TYPE} )

################################################################################
# Enable benchmarking
IF(CFG_BENCHMARK_ENABLED)
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DBENCHMARK_ENABLED -DBENCHMARK_MODULES=0xEE800043L")
ENDIF()