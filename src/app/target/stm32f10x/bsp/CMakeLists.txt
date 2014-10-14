################################################################################
#
# Generates the board support package for stm32f10x boards
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

PROJECT(bsp-${CFG_ARM_BOARD_TYPE} C)

ENABLE_LANGUAGE(ASM-ATT)

#################################################################################
# USER OPTIONS
IF(NOT CFG_STM32_HAL_LIB_LOC)
    SET(CFG_STM32_HAL_LIB_LOC
        ${PROJECT_SOURCE_DIR}
        CACHE PATH "Set the location of the STM32 Nucleo HAL library V1.2.0"
       )
ENDIF()

#################################################################################
# Target for creating the board support package
SET(STM32_HAL_LIB_PATH ${CFG_STM32_HAL_LIB_LOC}/Libraries)
SET(STM32_HAL_UTIL_DIR ${CFG_STM32_HAL_LIB_LOC}/Utilities)


SET(STM32_HAL_LIB_DIR ${STM32_HAL_LIB_PATH}/${CFG_ARM_BOARD_TYPE}_StdPeriph_Driver)
SET(STM32_HAL_CMSIS_DIR ${STM32_HAL_LIB_PATH}/CMSIS)
SET(STM32_HAL_CMSIS_TARG_DIR ${STM32_HAL_CMSIS_DIR}/Device/ST/${CFG_ARM_BOARD_TYPE})

SET(HAL_LIB_SRC
    ${STM32_HAL_LIB_DIR}/src/misc.c
    ${STM32_HAL_LIB_DIR}/src/stm32f10x_adc.c
    ${STM32_HAL_LIB_DIR}/src/stm32f10x_bkp.c
    ${STM32_HAL_LIB_DIR}/src/stm32f10x_can.c
    ${STM32_HAL_LIB_DIR}/src/stm32f10x_cec.c
    ${STM32_HAL_LIB_DIR}/src/stm32f10x_crc.c
    ${STM32_HAL_LIB_DIR}/src/stm32f10x_dac.c
    ${STM32_HAL_LIB_DIR}/src/stm32f10x_dbgmcu.c
    ${STM32_HAL_LIB_DIR}/src/stm32f10x_dma.c
    ${STM32_HAL_LIB_DIR}/src/stm32f10x_exti.c
    ${STM32_HAL_LIB_DIR}/src/stm32f10x_flash.c
    ${STM32_HAL_LIB_DIR}/src/stm32f10x_fsmc.c
    ${STM32_HAL_LIB_DIR}/src/stm32f10x_gpio.c
    ${STM32_HAL_LIB_DIR}/src/stm32f10x_i2c.c
    ${STM32_HAL_LIB_DIR}/src/stm32f10x_iwdg.c
    ${STM32_HAL_LIB_DIR}/src/stm32f10x_pwr.c
    ${STM32_HAL_LIB_DIR}/src/stm32f10x_rcc.c
    ${STM32_HAL_LIB_DIR}/src/stm32f10x_rtc.c
    ${STM32_HAL_LIB_DIR}/src/stm32f10x_sdio.c
    ${STM32_HAL_LIB_DIR}/src/stm32f10x_spi.c
    ${STM32_HAL_LIB_DIR}/src/stm32f10x_tim.c
    ${STM32_HAL_LIB_DIR}/src/stm32f10x_usart.c
    ${STM32_HAL_LIB_DIR}/src/stm32f10x_wwdg.c
    ${STM32_HAL_UTIL_DIR}/STM32_Nucleo/stm32f1xx/stm32f1xx_nucleo.c
)

# Modified sources for stm32f10x
SET(HAL_LIB_LOCAL_SRCS
                    ${PROJECT_SOURCE_DIR}/newlib_stubs.c
                    ${PROJECT_SOURCE_DIR}/stm32f1xx_it.c
                    ${PROJECT_SOURCE_DIR}/system_stm32f10x.c
                    ${PROJECT_SOURCE_DIR}/startup_stm32f10x_md.s
)

SET(HAL_LIB_INCS
                ${PROJECT_SOURCE_DIR}
                ${STM32_HAL_LIB_DIR}/inc
                ${STM32_HAL_CMSIS_TARG_DIR}/Include
                ${STM32_HAL_CMSIS_DIR}/Include
                ${STM32_HAL_UTIL_DIR}/STM32_Nucleo/stm32f1xx
                ${STM32_HAL_UTIL_DIR}/STM32_Nucleo/adafruit
                ${STM32_HAL_UTIL_DIR}/Common
                ${STM32_HAL_UTIL_DIR}/Third_Party/FatFs/src
                ${TARGET_DIR}
                CACHE INTERNAL "Very dirty!"
)

INCLUDE_DIRECTORIES(${HAL_LIB_INCS})

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DUSE_STDPERIPH_DRIVER -DSTM32F10X_MD")

# Remove pedantic from CMAKE_C_FLAGS (Causes a lot of warnings!)
STRING (REPLACE "-pedantic" "" CMAKE_C_FLAGS ${CMAKE_C_FLAGS})

#################################################################################
# Create bsp library
ADD_LIBRARY(${PROJECT_NAME} OBJECT ${HAL_LIB_SRC} ${HAL_LIB_LOCAL_SRCS})
