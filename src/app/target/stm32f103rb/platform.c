/**
********************************************************************************
\file   target/stm32f103rb/platform.c

\defgroup module_targ_stm32f103_platform Platform module
\{

\brief  Application interface target handling

Defines the platform specific functions of the slim interface for target
stm32f103rb (Cortex-M3).

\ingroup group_app_targ_stm32f103
*******************************************************************************/

/*------------------------------------------------------------------------------
* License Agreement
*
* Copyright 2014 BERNECKER + RAINER, AUSTRIA, 5142 EGGELSBERG, B&R STRASSE 1
* All rights reserved.
*
* Redistribution and use in source and binary forms,
* with or without modification,
* are permitted provided that the following conditions are met:
*
*   * Redistributions of source code must retain the above copyright notice,
*     this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above copyright notice,
*     this list of conditions and the following disclaimer
*     in the documentation and/or other materials provided with the
*     distribution.
*   * Neither the name of the B&R nor the names of its contributors
*     may be used to endorse or promote products derived from this software
*     without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
* THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
* A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
* THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* includes                                                                   */
/*----------------------------------------------------------------------------*/
#include <common/platform.h>

#include <apptarget/benchmark.h>

#include <misc.h>
#include <stm32f1xx_it.h>

#include <stdio.h>

/*============================================================================*/
/*            G L O B A L   D E F I N I T I O N S                             */
/*============================================================================*/

/*----------------------------------------------------------------------------*/
/* const defines                                                              */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* module global vars                                                         */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* global function prototypes                                                 */
/*----------------------------------------------------------------------------*/
static void usart2init(void);
static void initBenchmark(void);

/*============================================================================*/
/*            P R I V A T E   D E F I N I T I O N S                           */
/*============================================================================*/

/*----------------------------------------------------------------------------*/
/* const defines                                                              */
/*----------------------------------------------------------------------------*/
#define USARTx                           USART2

#define USARTx_RCC_PERIPH                RCC_APB1Periph_USART2
#define USARTx_RX_RCC_PERIPH             RCC_APB2Periph_GPIOA
#define USARTx_TX_RCC_PERIPH             RCC_APB2Periph_GPIOA

/* Definition for USARTx Pins */
#define USARTx_TX_PIN                    GPIO_Pin_2
#define USARTx_TX_GPIO_PORT              GPIOA
#define USARTx_RX_PIN                    GPIO_Pin_3
#define USARTx_RX_GPIO_PORT              GPIOA

/*----------------------------------------------------------------------------*/
/* local types                                                                */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* local vars                                                                 */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* local function prototypes                                                  */
/*----------------------------------------------------------------------------*/

/*============================================================================*/
/*            P U B L I C   F U N C T I O N S                                 */
/*============================================================================*/

/*----------------------------------------------------------------------------*/
/**
\brief  Initialize the peripherals of the target

This function init's the peripherals of the AP like cache and the interrupt
controller.

\return TRUE on success; FALSE on error
*/
/*----------------------------------------------------------------------------*/
BOOL platform_init(void)
{
    RCC_ClocksTypeDef RCC_Clocks;

    /* SysTick end of count event each 1ms */
    RCC_GetClocksFreq(&RCC_Clocks);
    SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000);

    /* by default stdin/stdout are on usart2 */
    usart2init();

    /* turn off buffers, so IO occurs immediately */
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    /* Initialize the benchmark pins */
    initBenchmark();

    return TRUE;
}

/*----------------------------------------------------------------------------*/
/**
\brief  Close all peripherals of the target
*/
/*----------------------------------------------------------------------------*/
void platform_exit(void)
{
    /* Close the USART serial */
    GPIO_DeInit(USARTx_TX_GPIO_PORT);
    GPIO_DeInit(USARTx_RX_GPIO_PORT);

    USART_DeInit(USARTx);

    /* Close the benchmark pins */
    GPIO_DeInit(PINx_BENCHMARK_PORT);
}

/*----------------------------------------------------------------------------*/
/**
\brief  Sleep for an amount of milliseconds

\param[in] msec_p       The milliseconds to sleep
*/
/*----------------------------------------------------------------------------*/
void platform_msleep(UINT32 msec_p)
{
    UINT32 currTime = tickCnt_l;
    UINT32 endTime = currTime + msec_p;

    while(currTime < endTime)
    {
        currTime = tickCnt_l;
    }
}

/*============================================================================*/
/*            P R I V A T E   F U N C T I O N S                               */
/*============================================================================*/
/** \name Private Functions */
/** \{ */

/*----------------------------------------------------------------------------*/
/**
\brief  Initialize the USART to enable prints to the terminal

The USART is used to forward characters from printf via the stdlib to the
host PC terminal.
*/
/*----------------------------------------------------------------------------*/
static void usart2init(void)
{
    USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Init peripheral clocks */
    RCC_APB2PeriphClockCmd(USARTx_RX_RCC_PERIPH | USARTx_TX_RCC_PERIPH | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(USARTx_RCC_PERIPH, ENABLE);

    /* Configure USART Tx as push-pull */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = USARTx_TX_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(USARTx_TX_GPIO_PORT, &GPIO_InitStructure);

    /* Configure USART Rx as input floating */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = USARTx_RX_PIN;
    GPIO_Init(USARTx_RX_GPIO_PORT, &GPIO_InitStructure);

    /* Configure USART */
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USARTx, &USART_InitStructure);

    /* Enable USART */
    USART_Cmd(USARTx, ENABLE);
}

/*----------------------------------------------------------------------------*/
/**
\brief  Initialize the benchmark pins used by benchmark.h
*/
/*----------------------------------------------------------------------------*/
static void initBenchmark(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    memset(&GPIO_InitStructure, 0, sizeof(GPIO_InitTypeDef));

    /* Enable BENCHMARK GPIO clock */
    RCC_APB2PeriphClockCmd(PINx_BENCHMARK_CLK_ENABLE, ENABLE);

    /* Use PA8 for benchmark pin 0 */
    GPIO_InitStructure.GPIO_Pin = PINx_BENCHMARK_PIN0 | PINx_BENCHMARK_PIN1 | PINx_BENCHMARK_PIN2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(PINx_BENCHMARK_PORT, &GPIO_InitStructure);
}

/**
 * \}
 * \}
 */
