/**
********************************************************************************
\file   syncir.c

\brief  Implements the driver for the synchronous interrupt

Defines the platform specific functions for the synchronous interrupt for target
stm32f103rb (Cortex-M3).

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
#include <common/syncir.h>

#include <stm32f10x_exti.h>
#include <stm32f10x_gpio.h>
#include <misc.h>

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

/*============================================================================*/
/*            P R I V A T E   D E F I N I T I O N S                           */
/*============================================================================*/

/*----------------------------------------------------------------------------*/
/* const defines                                                              */
/*----------------------------------------------------------------------------*/
#define IRx_SYNC_PIN                     GPIO_Pin_7
#define IRx_SYNC_GPIO_PORT               GPIOC
#define IRx_RCC_APB2Periph               RCC_APB2Periph_GPIOC

#define EXTI_LINEx                       EXTI_Line7
#define EXTIx_IRQn                       EXTI9_5_IRQn

/*----------------------------------------------------------------------------*/
/* local types                                                                */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* local vars                                                                 */
/*----------------------------------------------------------------------------*/
static tPlatformSyncIrq pfnSyncIrq_l = NULL;


/*----------------------------------------------------------------------------*/
/* local function prototypes                                                  */
/*----------------------------------------------------------------------------*/

/*============================================================================*/
/*            P U B L I C   F U N C T I O N S                                 */
/*============================================================================*/

/*----------------------------------------------------------------------------*/
/**
\brief  initialize synchronous interrupt

syncir_init() initializes the synchronous interrupt. The timing parameters
will be initialized, the interrupt handler will be connected to the ISR.

\param[in] pfnSyncIrq_p       The callback of the sync interrupt

\return BOOL
\retval TRUE        Synchronous interrupt initialization successful
\retval FALSE       Error while initializing the synchronous interrupt

\ingroup module_syncir
*/
/*----------------------------------------------------------------------------*/
BOOL syncir_init(tPlatformSyncIrq pfnSyncIrq_p)
{
    BOOL fReturn = TRUE;

    GPIO_InitTypeDef   GPIO_InitStructure;
    EXTI_InitTypeDef   EXTI_InitStructure;
    NVIC_InitTypeDef   NVIC_InitStructure;

    /* Remember ISR handler callback */
    pfnSyncIrq_l = pfnSyncIrq_p;

    /* Enable GPIOC clock */
    RCC_APB2PeriphClockCmd(IRx_RCC_APB2Periph, ENABLE);

    /* Configure SYNC IR pin as input floating */
    GPIO_InitStructure.GPIO_Pin = IRx_SYNC_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(IRx_SYNC_GPIO_PORT, &GPIO_InitStructure);

    /* Connect EXTI Line7 to PC7 pin */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource7);

    /* Configure EXTI Line7 */
    EXTI_InitStructure.EXTI_Line = EXTI_LINEx;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* Enable and set EXTI Line7 Interrupt to the highest priority */
    NVIC_InitStructure.NVIC_IRQChannel = EXTIx_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;   /* Use lowest sub priority! */
    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&NVIC_InitStructure);

    syncir_disable();

    return fReturn;
}

/*----------------------------------------------------------------------------*/
/**
\brief  Shutdown the synchronous interrupt

\ingroup module_syncir
*/
/*----------------------------------------------------------------------------*/
void syncir_exit(void)
{
    EXTI_DeInit();
    GPIO_DeInit(IRx_SYNC_GPIO_PORT);

    pfnSyncIrq_l = NULL;
}

/*----------------------------------------------------------------------------*/
/**
\brief  Acknowledge synchronous interrupt

\ingroup module_syncir
*/
/*----------------------------------------------------------------------------*/
void syncir_acknowledge(void)
{
    /* Acknowledge is done in ISR */
}

/*----------------------------------------------------------------------------*/
/**
\brief  Enable synchronous interrupt

syncir_enable() enables the synchronous interrupt.

\ingroup module_syncir
*/
/*----------------------------------------------------------------------------*/
void syncir_enable(void)
{
    NVIC_EnableIRQ(EXTIx_IRQn);
}

/*----------------------------------------------------------------------------*/
/**
\brief  Disable synchronous interrupt

syncir_disable() disable the synchronous interrupt.

\ingroup module_syncir
*/
/*----------------------------------------------------------------------------*/
void syncir_disable(void)
{
    NVIC_DisableIRQ(EXTIx_IRQn);
}

/*----------------------------------------------------------------------------*/
/**
\brief Enter/leave the critical section

This function enables/disables the interrupts of the AP processor

\param[in]  fEnable_p       TRUE = enable interrupts; FALSE = disable interrupts

\ingroup module_syncir
*/
/*----------------------------------------------------------------------------*/
void syncir_enterCriticalSection(UINT8 fEnable_p)
{
    if(fEnable_p)
        syncir_enable();
    else
        syncir_disable();
}


/*============================================================================*/
/*            P R I V A T E   F U N C T I O N S                               */
/*============================================================================*/
/* \name Private Functions */
/* \{ */

void EXTI9_5_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_LINEx) != RESET)
    {
        if(pfnSyncIrq_l != NULL)
        {
            pfnSyncIrq_l(NULL);
        }

        /* Clear the EXTI line pending bit */
        EXTI_ClearITPendingBit(EXTI_LINEx);
    }
}

/* \} */

