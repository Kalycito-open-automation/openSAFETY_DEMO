################################################################################
#
# CMake file of slim interface on demo-cn-gpio for PSI (Target is stm32f103rb)
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

ENABLE_LANGUAGE(ASM-ATT)

################################################################################
# Set C flags for this board configuration
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DUSE_STDPERIPH_DRIVER -DSTM32F10X_MD")

################################################################################
# Add board support package for target stm32f10x
ADD_SUBDIRECTORY(${APP_TARGET_DIR}/stm32f10x ${PROJECT_BINARY_DIR}/bsp)

################################################################################
# Set demo linker script
SET(DEMO_LINKER_SCRIPT ${TARGET_DIR}/stm32f103rb_flash.ld)

################################################################################
# Set architecture specific sources
SET(DEMO_ARCH_SRCS
                  ${TARGET_DIR}/platform.c
                  ${TARGET_DIR}/serial.c
                  ${TARGET_DIR}/syncir.c
                  ${TARGET_DIR}/app-gpio.c
                  ${TARGET_DIR}/startup_stm32f10x_md.s
                  ${TARGET_DIR}/newlib_stubs.c
                  ${TARGET_DIR}/stm32f1xx_it.c
                  ${TARGET_DIR}/system_stm32f10x.c
)

SET(DEMO_ARCH_INCS
                  ${TARGET_DIR}
                  ${HAL_LIB_INCS}
)

################################################################################
# Link bsp objects with executable
SET(DEMO_ARCH_SRCS
                  ${DEMO_ARCH_SRCS}
                  $<TARGET_OBJECTS:bsp-${CFG_ARM_BOARD_TYPE}>
)

#################################################################################
# Set compile flags
STRING (REPLACE "-pedantic" "" CMAKE_C_FLAGS ${CMAKE_C_FLAGS})

################################################################################
# Set architecture specific compile flags
SET(DEMO_ARCH_LINKER_FLAGS "-T${DEMO_LINKER_SCRIPT} -Wl,--start-group -lc -lm -Wl,--end-group -static -Wl,-cref,-u,Reset_Handler -Wl,-Map=${PROJECT_NAME}.map -Wl,--gc-sections -Wl,--defsym=malloc_getpagesize_P=0x1000")
