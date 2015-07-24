/**
********************************************************************************
\file   target/stm32f103rb/include/apptarget/benchmark.h

\brief  Header file for debugging. Enables setting of benchmark pins

This header is used to set benchmark pins in order to enable timing
measurements on the application processor.

*******************************************************************************/

/*------------------------------------------------------------------------------
Copyright (c) 2013, Bernecker+Rainer Industrie-Elektronik Ges.m.b.H. (B&R)
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holders nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
------------------------------------------------------------------------------*/

#ifndef _INC_apptarget_benchmark_H_
#define _INC_apptarget_benchmark_H_

/*----------------------------------------------------------------------------*/
/* includes                                                                   */
/*----------------------------------------------------------------------------*/
#include <stm32f10x_gpio.h>

/*----------------------------------------------------------------------------*/
/* const defines                                                              */
/*----------------------------------------------------------------------------*/
/* Definition for Benchmark pins */
#define PINx_BENCHMARK_PIN0              GPIO_Pin_10
#define PINx_BENCHMARK_PIN1              GPIO_Pin_11
#define PINx_BENCHMARK_PIN2              GPIO_Pin_12
#define PINx_BENCHMARK_PORT              GPIOC
#define PINx_BENCHMARK_CLK_ENABLE        RCC_APB2Periph_GPIOC

#ifdef BENCHMARK_ENABLED
    #define BENCHMARK_SET(x)    GPIO_WriteBit(PINx_BENCHMARK_PORT, (UINT16)(1<<(10 + (x))), Bit_SET)
    #define BENCHMARK_RESET(x)  GPIO_WriteBit(PINx_BENCHMARK_PORT, (UINT16)(1<<(10 + (x))), Bit_RESET)
    #define BENCHMARK_TOGGLE(x) /* No toggle till now */
#else
    #undef BENCHMARK_MODULES
    #define BENCHMARK_MODULES           0x00000000
#endif

/*----------------------------------------------------------------------------*/
/* typedef                                                                    */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* function prototypes                                                        */
/*----------------------------------------------------------------------------*/


#endif /* _INC_apptarget_benchmark_H_ */
