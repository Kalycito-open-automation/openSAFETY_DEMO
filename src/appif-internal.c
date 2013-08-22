/**
********************************************************************************
\file   appif-internal.c

\brief  Application interface internal configuration module

This module handlers library internals. It initializes internal modules and
processes the synchronous and asynchronous task.

\ingroup module_internal
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

#include "appif.h"

#include "appif-streamint.h"
#include "appif-asyncint.h"
#include "appif-ccint.h"

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

#define CONS_ACK_REGISTER_ID        0       ///< ID of the consumer ACK registers
#define ACK_REGISTER_COUNT          2       ///< Number of acknowledge registers

//------------------------------------------------------------------------------
// local types
//------------------------------------------------------------------------------
/**
\brief Rpdo user instance type

The rpdo instance holds the pdo information of this module
*/
typedef struct
{
    UINT32    consAckRegister;      ///< Data of the consumer ACK register
    UINT32    prodAckRegister;      ///< Data of the producer ACK register
} tInternalInstance;

//------------------------------------------------------------------------------
// local vars
//------------------------------------------------------------------------------

static tInternalInstance          intInstance_l;

//------------------------------------------------------------------------------
// local function prototypes
//------------------------------------------------------------------------------


//============================================================================//
//            P U B L I C   F U N C T I O N S                                 //
//============================================================================//

//------------------------------------------------------------------------------
/**
\brief    Application interface initialization function

\param[in]  pInitParam_p       Initialization parameters

\return tAppIfStatus
\retval kAppIfSuccessful         On success
\retval kAppIfStreamInitError    Initialization of the stream module failed

\ingroup module_internal
*/
//------------------------------------------------------------------------------
tAppIfStatus appif_init(tAppIfInitParam* pInitParam_p)
{
    tAppIfStatus ret = kAppIfSuccessful;
    tBuffParam   ackBuffer;
    tStreamInitParam streamInitParam;

    APPIF_MEMSET(&intInstance_l, 0 , sizeof(tInternalInstance));

#if _DEBUG
    if(pInitParam_p->countConsBuff_m + pInitParam_p->countProdBuff_m + ACK_REGISTER_COUNT !=
            kTbufCount)
    {
        ret = kAppIfInitError;
        goto Exit;
    }
#endif

    // Initialize stream module
    streamInitParam.pBuffDescList_m = pInitParam_p->pBuffDescList_m;
    streamInitParam.pfnStreamHandler_m = pInitParam_p->pfnStreamHandler_m;
    streamInitParam.countConsBuff_m = pInitParam_p->countConsBuff_m;
    streamInitParam.countProdBuff_m = pInitParam_p->countProdBuff_m;
    streamInitParam.pfnEnterCritSec_m = pInitParam_p->pfnEnterCritSec_m;

    ret = stream_init(&streamInitParam);
    if(ret != kAppIfSuccessful)
    {
        goto Exit;
    }

    // Initialize the asynchronous module
    ret = async_init();
    if(ret != kAppIfSuccessful)
    {
        goto Exit;
    }

    // Set consumer/producer ACK register to always ACK all!
    intInstance_l.consAckRegister = 0xFFFFFFFF;
    intInstance_l.prodAckRegister = 0xFFFFFFFF;

    // Register consumer acknowledge triple buffer
    ackBuffer.buffId_m = CONS_ACK_REGISTER_ID;
    ackBuffer.pBuffBase_m = (UINT8 *)&intInstance_l.consAckRegister;
    ackBuffer.buffSize_m = sizeof(intInstance_l.consAckRegister);

    ret = stream_registerBuffer(&ackBuffer);
    if(ret != kAppIfSuccessful)
    {
        goto Exit;
    }

    // Register producer acknowledge triple buffer
    ackBuffer.buffId_m = pInitParam_p->countConsBuff_m +
            pInitParam_p->countProdBuff_m + ACK_REGISTER_COUNT - 1;
    ackBuffer.pBuffBase_m = (UINT8 *)&intInstance_l.prodAckRegister;
    ackBuffer.buffSize_m = sizeof(intInstance_l.prodAckRegister);

    ret = stream_registerBuffer(&ackBuffer);
    if(ret != kAppIfSuccessful)
    {
        goto Exit;
    }

Exit:
    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Close the application interface

\ingroup module_internal
*/
//------------------------------------------------------------------------------
void appif_exit(void)
{
    // Destroy initialized modules
    stream_exit();
}

//------------------------------------------------------------------------------
/**
\brief    Finish application interface module initialization

\return  kAppIfSuccessful

\ingroup module_internal
*/
//------------------------------------------------------------------------------
tAppIfStatus appif_finishModuleInit(void)
{
    tAppIfStatus ret = kAppIfSuccessful;

    ret = stream_finishModuleInit();

    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Process application interface synchronous task

\return  tAppIfStatus
\retval  kAppIfSuccessful           On success
\retval  kAppIfStreamTransferError  Unable to transfer data to/from PCP

\ingroup module_internal
*/
//------------------------------------------------------------------------------
tAppIfStatus appif_processSync(void)
{
    tAppIfStatus ret = kAppIfSuccessful;

    ret = stream_processSync();

    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Process application interface asynchronous task

\param[in]  ppInstance_p         Pointer to the array of instances

\return  tAppIfStatus
\retval  kAppIfSuccessful           On success
\retval  Other                      User specified error code

\ingroup module_internal
*/
//------------------------------------------------------------------------------
tAppIfStatus appif_processAsync(tAsyncInstance* ppInstance_p)
{
    tAppIfStatus ret = kAppIfSuccessful;
    UINT8 i;

    // Process all asynchronous channels
    for(i=0; i < kNumAsyncInstCount; i++)
    {
        ret = async_process(ppInstance_p[i]);
        if(ret != kAppIfSuccessful)
        {
            goto Exit;
        }
    }

    // Process configuration channel objects
    ret = cc_process();
    if(ret != kAppIfSuccessful)
    {
        goto Exit;
    }

Exit:
    return ret;
}

//============================================================================//
//            P R I V A T E   F U N C T I O N S                               //
//============================================================================//
/// \name Private Functions
/// \{


/// \}


