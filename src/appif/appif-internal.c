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

#include <appif/appif.h>

#include <appif/appif-streamint.h>
#include <appif/appif-asyncint.h>
#include <appif/appif-ccint.h>
#include <appif/appif-errorint.h>
#include <appif/appif-timeout.h>

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

#define ACK_REG_INITIAL_VALUE    0xFFFFFFFF     ///< Initial value of the ACK register

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

static BOOL appif_initIntModules(tAppIfInitParam* pInitParam_p);
static tTbufAckRegister* appif_initAckRegister(tTbufNumLayout idAckReg_p);

//============================================================================//
//            P U B L I C   F U N C T I O N S                                 //
//============================================================================//

//------------------------------------------------------------------------------
/**
\brief    Application interface initialization function

\param[in]  pInitParam_p       Initialization parameters

\return BOOL
\retval TRUE        Successfully initialized the internal module
\retval FALSE       Error during initialization

\ingroup module_internal
*/
//------------------------------------------------------------------------------
BOOL appif_init(tAppIfInitParam* pInitParam_p)
{
    BOOL fReturn = FALSE;

    APPIF_MEMSET(&intInstance_l, 0 , sizeof(tInternalInstance));

    if(pInitParam_p == NULL)
    {
        // Invalid input parameters provided
        error_setError(kAppIfModuleInternal, kAppIfInitError);
    }
    else
    {
        if(pInitParam_p->idConsAck_m >= kTbufCount ||
           pInitParam_p->idProdAck_m >= kTbufCount  )
        {
            // Invalid input parameters provided
            error_setError(kAppIfModuleInternal, kAppIfInitError);
        }
        else
        {
            // Initialize internal library modules
            if(appif_initIntModules(pInitParam_p))
            {
                // Initialize consumer acknowledge register
                intInstance_l.pConsAckRegister_m = appif_initAckRegister(pInitParam_p->idConsAck_m);
                if(intInstance_l.pConsAckRegister_m != NULL)
                {
                    // Initialize producer acknowledge register
                    intInstance_l.pProdAckRegister_m = appif_initAckRegister(pInitParam_p->idProdAck_m);
                    if(intInstance_l.pProdAckRegister_m != NULL)
                    {
                        fReturn = TRUE;
                    }
                    else
                    {
                        // Producer acknowledge register initialization failed
                        error_setError(kAppIfModuleInternal, kAppIfInitError);
                    }
                }
                else
                {
                    // Consumer acknowledge register initialization failed
                    error_setError(kAppIfModuleInternal, kAppIfInitError);
                }
            }
            else
            {
                // Initializing internal modules failed
                error_setError(kAppIfModuleInternal, kAppIfInitError);
            }
        }
    }

    return fReturn;
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

    // Destroy error module
    error_exit();
}

//------------------------------------------------------------------------------
/**
\brief    Process application interface synchronous task

\return  BOOL
\retval  TRUE      Sync processing successful
\retval  FALSE     Error while processing sync

\ingroup module_internal
*/
//------------------------------------------------------------------------------
BOOL appif_processSync(void)
{
    BOOL fReturn = FALSE;

    if(stream_processSync() != FALSE)
    {
        fReturn = TRUE;
    }
    else
    {
        error_setError(kAppIfModuleInternal, kAppIfProcessSyncFailed);
    }

    return fReturn;
}

//------------------------------------------------------------------------------
/**
\brief    Process application interface asynchronous task

\param[in]  ppInstance_p         Pointer to the array of instances

\return  BOOL
\retval  TRUE         Async processing successful
\retval  FALSE        Error while processing async

\ingroup module_internal
*/
//------------------------------------------------------------------------------
BOOL appif_processAsync(tAsyncInstance* ppInstance_p)
{
    BOOL fReturn = FALSE, fError = FALSE;
#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_ASYNC)) != 0)
    UINT8 i;

    // Process all asynchronous channels
    for(i=0; i < kNumAsyncInstCount; i++)
    {
        if(async_process(ppInstance_p[i]) == FALSE)
        {
            fError = TRUE;
            break;
        }
    }
#endif

    if(!fError)
    {
#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_CC)) != 0)
        // Process configuration channel objects
        if(cc_process() != FALSE)
        {
            fReturn = TRUE;
        }
#else
        fReturn = TRUE;
#endif
    }

    return fReturn;
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

\return  BOOL
\retval  TRUE      Successfully initialized modules
\retval  FALSE     Error while initializing internal modules

\ingroup module_internal
*/
//------------------------------------------------------------------------------
static BOOL appif_initIntModules(tAppIfInitParam* pInitParam_p)
{
    BOOL fReturn = FALSE;
    tStreamInitParam streamInitParam;

#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_ASYNC)) != 0)
    // Initialize the asynchronous module
    async_init();
#endif

    // Initialize the timeout module
    timeout_init();

    // Initialize the error module
    error_init(pInitParam_p->pfnErrorHandler_m);

    // Initialize stream module
    streamInitParam.pBuffDescList_m = pInitParam_p->pBuffDescList_m;
    streamInitParam.pfnStreamHandler_m = pInitParam_p->pfnStreamHandler_m;
    streamInitParam.idConsAck_m = pInitParam_p->idConsAck_m;
    streamInitParam.idFirstProdBuffer_m = pInitParam_p->idFirstProdBuffer_m;

    if(stream_init(&streamInitParam) != FALSE)
    {
        fReturn = TRUE;
    }

    return fReturn;
}

//------------------------------------------------------------------------------
/**
\brief    Initialize the acknowledge registers

\param[in]  idAckReg_p         Id of the acknowledge register

\return tTbufAckRegister
\retval Address      Address of the initialized ACK register
\retval NULL         Error while initializing the acknowledge register

\ingroup module_internal
*/
//------------------------------------------------------------------------------
static tTbufAckRegister* appif_initAckRegister(tTbufNumLayout idAckReg_p)
{
    tBuffDescriptor* pDescAckReg;
    tTbufAckRegister* pAckRegBase = NULL;

    pDescAckReg = stream_getBufferParam(idAckReg_p);
    if(pDescAckReg != NULL)
    {
        if(pDescAckReg->buffSize_m == sizeof(tTbufAckRegister))
        {
            // Set ACK register address
            pAckRegBase = (tTbufAckRegister*)pDescAckReg->pBuffBase_m;

            // Set register to always ack all buffers (Needed for streamed access)
            *pAckRegBase = ACK_REG_INITIAL_VALUE;
        }
    }

    return pAckRegBase;
}

/// \}


