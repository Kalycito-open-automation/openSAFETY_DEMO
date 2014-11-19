/**
********************************************************************************
\file   timer.c

\brief  Target specific functions of the system timer

This module implements the hardware near target specific functions of the
system timer for stm32f401 (Cortex-M4).

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

#include <stm32f4xx_hal_rcc.h>
#include <stm32f4xx_hal_dma.h>
#include <stm32f4xx_hal_tim.h>
#include <stm32f4xx_hal_gpio.h>
#include <stm32f4xx_hal_cortex.h>

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
#define ENABLE_TIM_DEBUG           0        /* 1=enable; 0=disable */

#define TIMER_PRESCALE_1US         84
#define TIMER_PRESCALE_10US        840
#define TIMER_PRESCALE_100US       8400
#define TIMER_PRESCALE_1MS         42000

/* Definition for TIMx clock resources */
#define TIMx                       TIM2
#define TIMx_CLK_ENABLE            __TIM2_CLK_ENABLE

#if ENABLE_TIM_DEBUG == 1
/* Definition for TIM test pins */
#define TIMx_IRQn                  TIM2_IRQn
#define TIMx_IRQHandler            TIM2_IRQHandler

#define PINx_TIM_TEST_PIN               GPIO_PIN_9
#define PINx_TIM_TEST_PORT              GPIOA
#define PINx_TIM_TEST_CLK_ENABLE()      __GPIOA_CLK_ENABLE()
#endif

/*----------------------------------------------------------------------------*/
/* local types                                                                */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* local vars                                                                 */
/*----------------------------------------------------------------------------*/
static TIM_HandleTypeDef TimerHandle_l;
static BOOLEAN enpresc1ms_l = FALSE;    /**< Is the prescale for 1ms */

/*----------------------------------------------------------------------------*/
/* local function prototypes                                                  */
/*----------------------------------------------------------------------------*/
static BOOLEAN initTimer(UINT16 prescale_p);

#if ENABLE_TIM_DEBUG == 1
static void initGpio(void);
static void initNvic(void);
#endif

/*============================================================================*/
/*            P U B L I C   F U N C T I O N S                                 */
/*============================================================================*/

/*----------------------------------------------------------------------------*/
/**
\brief    Initialize the timer module

\return TRUE on success; FALSE on error

\ingroup module_timer
*/
/*----------------------------------------------------------------------------*/
BOOLEAN timer_init(void)
{
    BOOLEAN retVal = FALSE;

    enpresc1ms_l = FALSE;

    if(initTimer(TIMER_PRESCALE_100US))
    {
        if(HAL_TIM_Base_Start(&TimerHandle_l) == HAL_OK)
        {
#if ENABLE_TIM_DEBUG == 1
            initGpio();
            initNvic();
#endif

            retVal = TRUE;
        }
    }

    return retVal;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Close the timer module

\ingroup module_timer
*/
/*----------------------------------------------------------------------------*/
void timer_close(void)
{
    /* Close TIMx */
    HAL_TIM_Base_DeInit(&TimerHandle_l);
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
    UINT32 time = 0;

    if(enpresc1ms_l)
        time = (UINT32)(TimerHandle_l.Instance->CNT >> 2);
    else
        time = (UINT32)TimerHandle_l.Instance->CNT;

    return time;
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
    BOOLEAN validBase = FALSE;
    BOOLEAN retVal = FALSE;
    UINT16 prescale = TIMER_PRESCALE_100US;

    switch(base_p)
    {
        case kTimerBase1us:
            prescale = TIMER_PRESCALE_1US;
            validBase = TRUE;
            break;
        case kTimerBase10us:
            prescale = TIMER_PRESCALE_10US;
            validBase = TRUE;
            break;
        case kTimerBase100us:
            prescale = TIMER_PRESCALE_100US;
            validBase = TRUE;
            break;
        case kTimerBase1ms:
            prescale = TIMER_PRESCALE_1MS;
            validBase = TRUE;
            enpresc1ms_l = FALSE;
            break;
        default:
            break;
    }

    if(validBase == TRUE)
    {
        if(HAL_TIM_Base_Stop(&TimerHandle_l) == HAL_OK)
        {
            /* Set new prescale to timer */
            if(initTimer(prescale))
            {
                if(HAL_TIM_Base_Start(&TimerHandle_l) == HAL_OK)
                {
                    retVal = TRUE;
                }
            }
        }
    }

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
static BOOLEAN initTimer(UINT16 prescale_p)
{
    BOOLEAN retVal = FALSE;

    TIMx_CLK_ENABLE();

    TimerHandle_l.Instance = TIMx;
    TimerHandle_l.Init.Period = 0xFFFFFFFF;
    TimerHandle_l.Init.Prescaler = prescale_p;
    TimerHandle_l.Init.ClockDivision = 0;
    TimerHandle_l.Init.CounterMode = TIM_COUNTERMODE_UP;
    if(HAL_TIM_Base_Init(&TimerHandle_l) == HAL_OK)
    {
        retVal = TRUE;
    }

    return retVal;
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

    /* Enable the test port clock */
    PINx_TIM_TEST_CLK_ENABLE();

    /* Init the TIM test pin */
    GPIO_InitStructure.Pin = PINx_TIM_TEST_PIN;
    GPIO_InitStructure.Speed = GPIO_SPEED_MEDIUM;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(PINx_TIM_TEST_PORT, &GPIO_InitStructure);
}

/*----------------------------------------------------------------------------*/
/**
\brief  Initialize the timer overflow interrupt

\ingroup module_timer
*/
/*----------------------------------------------------------------------------*/
static void initNvic(void)
{
    HAL_NVIC_SetPriority(TIMx_IRQn, 4, 0);
    HAL_NVIC_EnableIRQ(TIMx_IRQn);
}

/*----------------------------------------------------------------------------*/
/**
\brief  Timer interrupt handler

\ingroup module_timer
*/
/*----------------------------------------------------------------------------*/
void TIMx_IRQHandler(void)
{
    HAL_GPIO_WritePin(PINx_TIM_TEST_PORT, PINx_TIM_TEST_PIN, GPIO_PIN_SET);

    HAL_TIM_IRQHandler(&TimerHandle_l);

    HAL_GPIO_WritePin(PINx_TIM_TEST_PORT, PINx_TIM_TEST_PIN, GPIO_PIN_RESET);
}
#endif

/* \} */
