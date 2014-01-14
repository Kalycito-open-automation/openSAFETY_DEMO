/**
********************************************************************************
\file   platform.c

\brief  Application interface target handling

Defines the platform specific functions of the application interface example
implementation.

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

//------------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------------
#include <apptarget/platform.h>

#include <alt_types.h>
#include <sys/alt_cache.h>
#include <sys/alt_irq.h>
#include <altera_avalon_pio_regs.h>

// Include custom SPI driver
#include <avalon_spi/avalon_spi.h>

//============================================================================//
//            G L O B A L   D E F I N I T I O N S                             //
//============================================================================//

//------------------------------------------------------------------------------
// const defines
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// module global vars
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// global function prototypes
//------------------------------------------------------------------------------


//============================================================================//
//            P R I V A T E   D E F I N I T I O N S                           //
//============================================================================//

//------------------------------------------------------------------------------
// const defines
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// local types
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// local vars
//------------------------------------------------------------------------------

/**
 * \brief Platform module instance type
 */
typedef struct {
    alt_irq_context irqContext_m;
} tPlatformInstance;


//------------------------------------------------------------------------------
// local function prototypes
//------------------------------------------------------------------------------

static tPlatformInstance platformInstance_l;

//============================================================================//
//            P U B L I C   F U N C T I O N S                                 //
//============================================================================//

//------------------------------------------------------------------------------
/**
\brief  Initialize the peripherals of the target

This function init's the peripherals of the AP like cache and the interrupt
controller.

\ingroup module_platform
*/
//------------------------------------------------------------------------------
void platform_init(void)
{
    alt_icache_flush_all();
    alt_dcache_flush_all();
}

//------------------------------------------------------------------------------
/**
\brief  Close all peripherals of the target

\ingroup module_platform
*/
//------------------------------------------------------------------------------
void platform_exit(void)
{

}

//------------------------------------------------------------------------------
/**
\brief Enter/leave the critical section

This function enables/disables the interrupts of the AP processor

\param[in]  fEnable_p       TRUE = enable interrupts; FALSE = disable interrupts

\ingroup module_platform
*/
//------------------------------------------------------------------------------
void platform_enterCriticalSection(UINT8 fEnable_p)
{

    if(fEnable_p)
    {
        alt_irq_enable_all(platformInstance_l.irqContext_m);
    }
    else
    {
        platformInstance_l.irqContext_m = alt_irq_disable_all();
    }
}

//------------------------------------------------------------------------------
/**
\brief  initialize synchronous interrupt

SysComp_initSyncInterrupt() initializes the synchronous interrupt. The timing parameters
will be initialized, the interrupt handler will be connected and the interrupt
will be enabled.

\param[in] pfnSyncIrq_p       The callback of the sync interrupt

\return BOOL
\retval TRUE        Synchronous interrupt initialization successful
\retval FALSE       Error while initializing the synchronous interrupt

\ingroup module_platform
*/
//------------------------------------------------------------------------------
BOOL platform_initSyncInterrupt(tPlatformSyncIrq pfnSyncIrq_p)
{
    BOOL fReturn = FALSE, regRet = FALSE;

    /* register interrupt handler */
#ifdef ALT_ENHANCED_INTERRUPT_API_PRESENT
    if (alt_ic_isr_register(APP_INTERRUPT_CONTROLLER_ID, SYNC_IRQ_NUM, pfnSyncIrq_p, NULL, 0) == 0)
    {
        regRet = TRUE;
    }
#else
    if (alt_irq_register(SYNC_IRQ_NUM, NULL, pfnSyncIrq_p) == 0)
    {
        regRet = TRUE;
    }
#endif

    if(regRet != FALSE)
    {
        /* enable interrupt from PCP to AP */
        alt_ic_irq_enable(APP_INTERRUPT_CONTROLLER_ID, SYNC_IRQ_NUM);      // enable specific IRQ Number
        IOWR_ALTERA_AVALON_PIO_IRQ_MASK(SYNC_IRQ_BASE, 0x01);

        fReturn = TRUE;
    }

    return fReturn;
}

//------------------------------------------------------------------------------
/**
\brief  Acknowledge synchronous interrupt

\ingroup module_platform
*/
//------------------------------------------------------------------------------
void platform_ackSyncIrq(void)
{
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(SYNC_IRQ_BASE, 0x01);
}

//------------------------------------------------------------------------------
/**
\brief  Enable synchronous interrupt

SysComp_enableSyncInterrupt() enables the synchronous interrupt.

\ingroup module_platform
*/
//------------------------------------------------------------------------------
void platform_enableSyncInterrupt(void)
{
    alt_ic_irq_enable(0, SYNC_IRQ_NUM);  // enable specific IRQ Number
}

//------------------------------------------------------------------------------
/**
\brief  Disable synchronous interrupt

SysComp_disableSyncInterrupt() disable the synchronous interrupt.

\ingroup module_platform
*/
//------------------------------------------------------------------------------
void platform_disableSyncInterrupt(void)
{
    alt_ic_irq_disable(0, SYNC_IRQ_NUM);  // disable specific IRQ Number
}


//------------------------------------------------------------------------------
/**
\brief  Execute SPI command

SysComp_SPICommand() sends an SPI command to the SPI master by using the
avalon_spi driver

\param[in] pHandlParam_p       The parameters of the SPI command handler

\return BOOL
\retval TRUE        On success
\retval FALSE       SPI send or receive failed

\ingroup module_platform
*/
//------------------------------------------------------------------------------
BOOL platform_spiCommand(tHandlerParam* pHandlParam_p)
{
    BOOL fReturn = FALSE;
    int spiRet;
    tDescriptor   initDesc = { NULL, 4, NULL, 4 };

    // Perform four byte initial SPI access
    spiRet = avalon_spi_tf(
            SPI_MASTER_BASE_ADDRESS,
            0,
            &initDesc,
            AVALON_SPI_HOLD_SS
            );
    if(spiRet == 0)
    {
        // Transfer the actual input and output image
        spiRet = avalon_spi_tf(
                SPI_MASTER_BASE_ADDRESS,
                0,
                (tDescriptor *)&pHandlParam_p->consDesc_m,
                0
                );
        if(spiRet == 0)
        {
            fReturn = TRUE;
        }
    }

    return fReturn;
}

//============================================================================//
//            P R I V A T E   F U N C T I O N S                               //
//============================================================================//
/// \name Private Functions
/// \{


/// \}

