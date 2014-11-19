/**
********************************************************************************
\file   syncir.c

\brief  Implements the driver for the synchronous interrupt

Defines the platform specific functions for the synchronous interrupt for target
stm32f103rb.

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

#include <stm32f4xx_hal_rcc.h>
#include <stm32f4xx_hal_gpio.h>
#include <stm32f4xx_hal_cortex.h>

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
#define IRx_SYNC_CLK_ENABLE()            __GPIOC_CLK_ENABLE()

#define IRx_SYNC_PIN                     GPIO_PIN_7
#define IRx_SYNC_GPIO_PORT               GPIOC

#define Rx_SYNC_IRQn                     EXTI9_5_IRQn

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

    memset(&GPIO_InitStructure, 0, sizeof(GPIO_InitTypeDef));

    /* Remember ISR handler callback */
    pfnSyncIrq_l = pfnSyncIrq_p;

    /* Enable GPIOC clock */
    IRx_SYNC_CLK_ENABLE();

    /* Configure SYNC interrupt pin */
    GPIO_InitStructure.Pin = IRx_SYNC_PIN;
    GPIO_InitStructure.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStructure.Speed = GPIO_SPEED_MEDIUM;
    GPIO_InitStructure.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(IRx_SYNC_GPIO_PORT, &GPIO_InitStructure);

    /* Enable and set EXTI Line7 Interrupt to the lowest priority */
    HAL_NVIC_SetPriority(Rx_SYNC_IRQn, 2, 0);

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
    syncir_disable();

    HAL_GPIO_DeInit(IRx_SYNC_GPIO_PORT, IRx_SYNC_PIN);

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
    NVIC_EnableIRQ(Rx_SYNC_IRQn);
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
    NVIC_DisableIRQ(Rx_SYNC_IRQn);
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
    /* Toggle interrupt disable/enable */
    if(fEnable_p)
    {
        __enable_irq();
    }
    else
    {
        __disable_irq();
    }
}


/*============================================================================*/
/*            P R I V A T E   F U N C T I O N S                               */
/*============================================================================*/
/* \name Private Functions */
/* \{ */

/*----------------------------------------------------------------------------*/
/**
\brief  External interrupt callback

\param gpioPin_p        The pin the callback is called for

\ingroup module_serial
*/
/*----------------------------------------------------------------------------*/
void HAL_GPIO_EXTI_Callback(uint16_t gpioPin_p)
{
  if(gpioPin_p == IRx_SYNC_PIN)
  {
      if(pfnSyncIrq_l != NULL)
      {
          pfnSyncIrq_l(NULL);
      }
  }
}

/*----------------------------------------------------------------------------*/
/**
\brief  External interrupt handler for the synchronous interrupt

\ingroup module_serial
*/
/*----------------------------------------------------------------------------*/
void EXTI9_5_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(IRx_SYNC_PIN);
}

/* \} */

