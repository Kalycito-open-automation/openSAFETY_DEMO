/**
********************************************************************************
\file   demo-sn-gpio/target/stm32f103rb/timer.c

\defgroup module_sn_stm32f103_timer Timer module
\{

\brief  Target specific functions of the system timer

This module implements the hardware near target specific functions of the
system timer for stm32f103rb (Cortex-M3).

\ingroup group_app_sn_targ_stm32f103
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

#define TIMER_PRESCALE_1US         (UINT16)63       /**< Prescaler for 1us resolution */

/* Defines for the TIMx peripheral */
#define TIMx                TIM2

#define TIMx_RCC_PERIPH     RCC_APB1Periph_TIM2

/*----------------------------------------------------------------------------*/
/* local types                                                                */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* local vars                                                                 */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* local function prototypes                                                  */
/*----------------------------------------------------------------------------*/
static void initTimer(void);

/*============================================================================*/
/*            P U B L I C   F U N C T I O N S                                 */
/*============================================================================*/

/*----------------------------------------------------------------------------*/
/**
\brief    Initialize the timer module

\return 0 on success; 1 on error
*/
/*----------------------------------------------------------------------------*/
BOOLEAN timer_init(void)
{
    initTimer();

    /* Enable timer interface */
    TIM_Cmd(TIMx, ENABLE);

    return TRUE;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Close the timer module
*/
/*----------------------------------------------------------------------------*/
void timer_close(void)
{
    /* Close TIMx interface */
    TIM_DeInit(TIMx);
}

/*----------------------------------------------------------------------------*/
/**
\brief    Get current system tick

This function returns the current system tick determined by the system timer.

\return Returns the system tick in milliseconds
*/
/*----------------------------------------------------------------------------*/
UINT16 timer_getTickCount(void)
{
    UINT16 time = 0;

    time = (UINT16)TIM_GetCounter(TIMx);

    return time;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Set the current system tick to a desired value

\param[in] newVal_p     The new value for the timer
*/
/*----------------------------------------------------------------------------*/
void timer_setTickCount(UINT16 newVal_p)
{
    TIM_SetCounter(TIMx, newVal_p);
}


/*============================================================================*/
/*            P R I V A T E   F U N C T I O N S                               */
/*============================================================================*/
/** \name Private Functions */
/** \{ */

/*----------------------------------------------------------------------------*/
/**
\brief  Initialize the timer core
*/
/*----------------------------------------------------------------------------*/
static void initTimer(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    memset(&TIM_TimeBaseStructure, 0, sizeof(TIM_TimeBaseInitTypeDef));

    /* Enable timer2 clock */
    RCC_APB1PeriphClockCmd(TIMx_RCC_PERIPH, ENABLE);

    TIM_TimeBaseStructure.TIM_Prescaler = TIMER_PRESCALE_1US;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIMx, &TIM_TimeBaseStructure);
}

/**
 * \}
 * \}
 */
