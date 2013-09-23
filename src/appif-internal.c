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

//------------------------------------------------------------------------------
// local types
//------------------------------------------------------------------------------

/**
\brief Internal user instance type
*/
typedef struct
{
    tTbufAckRegister*    pConsAckRegister_m;      ///< Pointer to the consumer ACK register
    tTbufAckRegister*    pProdAckRegister_m;      ///< Pointer to the producer ACK register
} tInternalInstance;

//------------------------------------------------------------------------------
// local vars
//------------------------------------------------------------------------------

static tInternalInstance          intInstance_l;

//------------------------------------------------------------------------------
// local function prototypes
//------------------------------------------------------------------------------

static tAppIfStatus appif_initIntModules(tAppIfInitParam* pInitParam_p);
static tAppIfStatus appif_initAckRegisters(tTbufNumLayout idConsAck_p,
                                         tTbufNumLayout idProdAck_p);

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

    APPIF_MEMSET(&intInstance_l, 0 , sizeof(tInternalInstance));

#if _DEBUG
    if(pInitParam_p != NULL)
    {
        if(pInitParam_p->idConsAck_m >= kTbufCount ||
           pInitParam_p->idProdAck_m >= kTbufCount  )
        {
            ret = kAppIfInitError;
        }
    }
    else
    {
        ret = kAppIfInitError;
    }
#endif

    if(ret == kAppIfSuccessful)
    {
        // Initialize internal library modules
        ret = appif_initIntModules(pInitParam_p);
        if(ret == kAppIfSuccessful)
        {
            // Initialize acknowledge registers
            ret = appif_initAckRegisters(pInitParam_p->idConsAck_m,
                                         pInitParam_p->idProdAck_m);
        }
    }

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
#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_ASYNC)) != 0)
    UINT8 i;

    // Process all asynchronous channels
    for(i=0; i < kNumAsyncInstCount; i++)
    {
        ret = async_process(ppInstance_p[i]);
        if(ret != kAppIfSuccessful)
        {
            break;
        }
    }
#endif

#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_CC)) != 0)
    if(ret == kAppIfSuccessful)
    {
        // Process configuration channel objects
        ret = cc_process();
    }
#endif

    return ret;
}

//============================================================================//
//            P R I V A T E   F U N C T I O N S                               //
//============================================================================//
/// \name Private Functions
/// \{


//------------------------------------------------------------------------------
/**
\brief    Initialize all internal modules

\param[in]  pInitParam_p         Internal module initialization parameters

\return  tAppIfStatus
\retval  kAppIfSuccessful          On success
\retval  kAppIfStreamInitError     Error while initializing the stream module

\ingroup module_internal
*/
//------------------------------------------------------------------------------
static tAppIfStatus appif_initIntModules(tAppIfInitParam* pInitParam_p)
{
    tAppIfStatus ret = kAppIfSuccessful;
    tStreamInitParam streamInitParam;

#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_ASYNC)) != 0)
    // Initialize the asynchronous module
    async_init();
#endif

    // Initialize stream module
    streamInitParam.pBuffDescList_m = pInitParam_p->pBuffDescList_m;
    streamInitParam.pfnStreamHandler_m = pInitParam_p->pfnStreamHandler_m;
    streamInitParam.idConsAck_m = pInitParam_p->idConsAck_m;
    streamInitParam.idFirstProdBuffer_m = pInitParam_p->idFirstProdBuffer_m;

    ret = stream_init(&streamInitParam);

    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Initialize the acknowledge registers

\param[in]  idConsAck_p         Id of the consuming acknowledge register
\param[in]  idProdAck_p         Id of the producing acknowledge register

\return tAppIfStatus
\retval kAppIfSuccessful            On success
\retval kAppIfStreamInvalidBuffer   Unable to find the buffer in the list
\retval kAppIfBufferSizeMismatch    Size of the buffer is invalid

\ingroup module_internal
*/
//------------------------------------------------------------------------------
static tAppIfStatus appif_initAckRegisters(tTbufNumLayout idConsAck_p,
                                         tTbufNumLayout idProdAck_p)
{
    tAppIfStatus ret = kAppIfSuccessful;
    tBuffDescriptor* pDescConsAck;
    tBuffDescriptor* pDescProdAck;

    ret = stream_getBufferParam(idConsAck_p, &pDescConsAck);
    if(ret == kAppIfSuccessful)
    {
        if(pDescConsAck->buffSize_m == sizeof(tTbufAckRegister))
        {
            // Set consumer ACK register address
            intInstance_l.pConsAckRegister_m = (tTbufAckRegister*)pDescConsAck->pBuffBase_m;

            // Set register to always ack all buffers (Needed for streamed access)
            *intInstance_l.pConsAckRegister_m = 0xFFFFFFFF;

            ret = stream_getBufferParam(idProdAck_p, &pDescProdAck);
            if(ret == kAppIfSuccessful)
            {
                if(pDescProdAck->buffSize_m == sizeof(tTbufAckRegister))
                {
                    // Set producer ACK register address
                    intInstance_l.pProdAckRegister_m = (tTbufAckRegister*)pDescProdAck->pBuffBase_m;

                    // Set register to always ack all buffers (Needed for streamed access)
                    *intInstance_l.pProdAckRegister_m = 0xFFFFFFFF;
                }
                else
                {
                    ret = kAppIfBufferSizeMismatch;
                }
            }
        }
        else
        {
            ret = kAppIfBufferSizeMismatch;
        }
    }

    return ret;
}

/// \}


