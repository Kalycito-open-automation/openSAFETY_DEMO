/**
********************************************************************************
\file   timer.c

\brief  Target specific functions of the system timer

This module implements the hardware near target specific functions of the
system timer for stm32f103rb (Cortex-M3).

\ingroup module_timer
*******************************************************************************/

/*------------------------------------------------------------------------------
* License Agreement
*
* Copyright 2013 BERNECKER + RAINER, AUSTRIA, 5142 EGGELSBERG, B&R STRASSE 1
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
#include <sn/timer.h>

#include <stm32f10x_tim.h>

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

/*============================================================================*/
/*            P R I V A T E   D E F I N I T I O N S                           */
/*============================================================================*/

/*----------------------------------------------------------------------------*/
/* const defines                                                              */
/*----------------------------------------------------------------------------*/
#define ENABLE_TIM_DEBUG           0        /* 0 = disable; 1 = enable */

#define TIMER_PRESCALE_1US         64
#define TIMER_PRESCALE_10US        640
#define TIMER_PRESCALE_100US       6400
#define TIMER_PRESCALE_1MS         32000

/* Defines for the TIMx peripheral */
#define TIMx                TIM2

#define TIMx_RCC_PERIPH     RCC_APB1Periph_TIM2

#define TIMx_IRQn           TIM2_IRQn
#define TIMx_IRQHandler     TIM2_IRQHandler

#if ENABLE_TIM_DEBUG == 1
#define TIMx_DEBUG_PIN            GPIO_Pin_5
#define TIMx_DEBUG_GPIO_PORT      GPIOA
#define TIMx_DEBUG_RCC_PERIPH     RCC_APB2Periph_GPIOA
#endif

/*----------------------------------------------------------------------------*/
/* local types                                                                */
/*----------------------------------------------------------------------------*/
static UINT16 timerValHigh_l = 0;       /**< High UINT16 of the timer value */
static BOOLEAN enpresc1ms_l = FALSE;    /**< Is the prescale for 1ms */

/*----------------------------------------------------------------------------*/
/* local vars                                                                 */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* local function prototypes                                                  */
/*----------------------------------------------------------------------------*/
static void initTimer(UINT16 prescale);
static void initNvic(void);

#if ENABLE_TIM_DEBUG == 1
static void initGpio(void);
#endif

/*============================================================================*/
/*            P U B L I C   F U N C T I O N S                                 */
/*============================================================================*/

/*----------------------------------------------------------------------------*/
/**
\brief    Initialize the timer module

\return 0 on success; 1 on error

\ingroup module_timer
*/
/*----------------------------------------------------------------------------*/
BOOLEAN timer_init(void)
{
    timerValHigh_l = 0;
    enpresc1ms_l = FALSE;

    initTimer(TIMER_PRESCALE_100US);
    initNvic();

    /* Enable timer interface */
    TIM_Cmd(TIMx, ENABLE);

#if ENABLE_TIM_DEBUG == 1
    initGpio();
#endif

    return TRUE;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Close the timer module

\ingroup module_timer
*/
/*----------------------------------------------------------------------------*/
void timer_close(void)
{
    /* Close TIMx interface */
    TIM_DeInit(TIMx);

#if ENABLE_TIM_DEBUG == 1
    GPIO_DeInit(TIMx_DEBUG_GPIO_PORT);
#endif
}

/*----------------------------------------------------------------------------*/
/**
\brief    Get current system tick

This function returns the current system tick determined by the system timer.

\return Returns the system tick in milliseconds

\ingroup module_timer
*/
/*----------------------------------------------------------------------------*/
UINT32 timer_getTickCount(void)
{
    UINT64 time = 0;

    if(enpresc1ms_l == TRUE)
        time = ((timerValHigh_l << 16) | (UINT16)TIM_GetCounter(TIMx))>>2;
    else
        time = (timerValHigh_l << 16) | (UINT16)TIM_GetCounter(TIMx);

    return (UINT32)time;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Update the base of the timer

\param base_p The new base of the timer

\return TRUE on success; FALSE on error

\ingroup module_timer
*/
/*----------------------------------------------------------------------------*/
BOOLEAN timer_setBase(tTimerBase base_p)
{
    BOOLEAN retVal = FALSE;
    UINT16 prescale = TIMER_PRESCALE_100US;

    switch(base_p)
    {
        case kTimerBase1us:
            prescale = TIMER_PRESCALE_1US;
            retVal = TRUE;
            break;
        case kTimerBase10us:
            prescale = TIMER_PRESCALE_10US;
            retVal = TRUE;
            break;
        case kTimerBase100us:
            prescale = TIMER_PRESCALE_100US;
            retVal = TRUE;
            break;
        case kTimerBase1ms:
            prescale = TIMER_PRESCALE_1MS;
            enpresc1ms_l = TRUE;
            retVal = TRUE;
            break;
        default:
            break;
    }

    TIM_Cmd(TIMx, DISABLE);

    /* Set new prescale to timer */
    initTimer(prescale);

    TIM_Cmd(TIMx, ENABLE);

    return retVal;
}


/*============================================================================*/
/*            P R I V A T E   F U N C T I O N S                               */
/*============================================================================*/
/* \name Private Functions */
/* \{ */

/*----------------------------------------------------------------------------*/
/**
\brief  Initialize the timer core

\param prescale     The prescale value of the current time base

\ingroup module_timer
*/
/*----------------------------------------------------------------------------*/
static void initTimer(UINT16 prescale)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    memset(&TIM_TimeBaseStructure, 0, sizeof(TIM_TimeBaseInitTypeDef));

    /* Enable timer2 clock */
    RCC_APB1PeriphClockCmd(TIMx_RCC_PERIPH, ENABLE);

    TIM_TimeBaseStructure.TIM_Prescaler = prescale;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIMx, &TIM_TimeBaseStructure);

    /* Enable update event interrupt */
    TIM_ITConfig(TIMx, TIM_IT_Update, ENABLE);
}

/*----------------------------------------------------------------------------*/
/**
\brief  Initialize the nested interrupt controller

\ingroup module_timer
*/
/*----------------------------------------------------------------------------*/
static void initNvic(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    memset(&NVIC_InitStructure, 0, sizeof(NVIC_InitTypeDef));

    /* Enable TIMx interrupt in NVIC */
    NVIC_InitStructure.NVIC_IRQChannel = TIMx_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

#if ENABLE_TIM_DEBUG == 1
/*----------------------------------------------------------------------------*/
/**
\brief  Initialize gpio PORTA pin9 for debugging

\ingroup module_timer
*/
/*----------------------------------------------------------------------------*/
static void initGpio(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    memset(&GPIO_InitStructure, 0, sizeof(GPIO_InitTypeDef));

    /* Enable the GPIOA clock */
    RCC_APB2PeriphClockCmd(TIMx_DEBUG_RCC_PERIPH, ENABLE);

    /* Initialize the timer debug pin */
    GPIO_InitStructure.GPIO_Pin = TIMx_DEBUG_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(TIMx_DEBUG_GPIO_PORT, &GPIO_InitStructure);
}
#endif

/*----------------------------------------------------------------------------*/
/**
\brief  Handle TIMx overflow interrupt

\ingroup module_timer
*/
/*----------------------------------------------------------------------------*/
void TIMx_IRQHandler(void)
{
    if(TIM_GetITStatus(TIMx, TIM_IT_Update) == SET)
    {
#if ENABLE_TIM_DEBUG == 1
        TIMx_DEBUG_GPIO_PORT->BSRR = TIMx_DEBUG_PIN;
#endif

        /* TIMx overflow -> increment high word! */
        timerValHigh_l++;

        TIM_ClearITPendingBit(TIMx, TIM_IT_Update);

#if ENABLE_TIM_DEBUG == 1
        TIMx_DEBUG_GPIO_PORT->BRR = TIMx_DEBUG_PIN;
#endif
    }
}

/* \} */
