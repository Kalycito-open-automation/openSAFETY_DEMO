/**
********************************************************************************
\file   apptarget/platform.h

\brief  Application interface system components implementation

Defines the platform specific functions of the application interface example
implementation.

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

#ifndef _INC_platform_H_
#define _INC_platform_H_

//------------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------------
#include <libappif/appif.h>

#include <system.h>
#include <string.h>
#include <stdio.h>




//------------------------------------------------------------------------------
// const defines
//------------------------------------------------------------------------------

// SYNC IRQ dependencies
#if APP_0_SYNC_IRQ_BASE
  #define SYNC_IRQ_NUM                    0     ///< Id of the synchronous interrupt (Workaround: Parameter is not forwarded to system.h when CPU is in a subsystem)
  #define SYNC_IRQ_BASE                   APP_0_SYNC_IRQ_BASE
  #define APP_INTERRUPT_CONTROLLER_ID     0     ///< Id of the Nios ISR controller (Workaround: Parameter is not forwarded to system.h when CPU is in a subsystem)
#endif

// SPI master
#ifdef SPI_MASTER_BASE
  #define SPI_MASTER_BASE_ADDRESS SPI_MASTER_BASE
#endif


//------------------------------------------------------------------------------
// typedef
//------------------------------------------------------------------------------

/**
 * \brief Synchronous interrupt callback function
 */
#ifndef ALT_ENHANCED_INTERRUPT_API_PRESENT
    typedef void (*tPlatformSyncIrq)(void *, void *);
#else
    typedef void (*tPlatformSyncIrq)(void *);
#endif

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
void platform_init(void);
void platform_exit(void);

void platform_enterCriticalSection(UINT8 fEnable_p);
BOOL platform_initSyncInterrupt(tPlatformSyncIrq pfnSyncIrq_p);
void platform_ackSyncIrq(void);
void platform_enableSyncInterrupt(void);
void platform_disableSyncInterrupt(void);

BOOL platform_spiCommand(tHandlerParam* pHandlParam_p);

#endif /* _INC_platform_H_ */

