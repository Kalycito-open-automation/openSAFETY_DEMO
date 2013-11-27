/**
********************************************************************************
\file   appif.c

\brief  Application interface managing module

This module acts as an interface between the application interface modules
and the openPOWERLINK stack modules. It initializes all appif modules and calls
the synchronous and asynchronous tasks.

\ingroup module_appif
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

#include <appif/tbuf.h>
#include <appif/status.h>
#include <appif/cc.h>
#include <appif/icc.h>
#include <appif/occ.h>
#include <appif/pdo.h>
#include <appif/rpdo.h>
#include <appif/tpdo.h>
#include <appif/async.h>
#include <appif/fifo.h>
#include <appifcommon/ccobject.h>

#include <config/ccobjectlist.h>
#include <config/tbuflayout.h>


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

typedef struct {
    tAsyncInstance    instAsyncChan_m[kNumAsyncInstCount];    ///< Instance of the asynchronous channel 0
} tAppIfInstance;

//------------------------------------------------------------------------------
// local vars
//------------------------------------------------------------------------------

static tAppIfInstance appifInstance_l;

//------------------------------------------------------------------------------
// local function prototypes
//------------------------------------------------------------------------------

//============================================================================//
//            P U B L I C   F U N C T I O N S                                 //
//============================================================================//

//------------------------------------------------------------------------------
/**
\brief    Application interface initialization function

Initializes the application interface and all its submodules.

\param[in] pfnCritSec_p           Critical section entry point function

\return tAppIfStatus
\retval kAppIfSuccessful          successful initialization
\retval other                     Other initialization error code

\ingroup module_appif
*/
//------------------------------------------------------------------------------
tAppIfStatus appif_init(UINT8 nodeId_p, tAppIfCritSec pfnCritSec_p)
{
    tAppIfStatus         ret = kAppIfSuccessful;
    tStatusInitStruct    statusInitParam;
#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_CC)) != 0)
    tOccInitStruct       occInitParam;
    tIccInitStruct       iccInitParam;
#endif
#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_ASYNC)) != 0)
    UINT8                i;
    tAsyncInitStruct     asyncInitParam;
#endif
#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_PDO)) != 0)
    tRpdoInitStruct      rpdoInitParam;
    tTpdoInitStruct      tpdoInitParam;
#endif


    tTbufDescriptor      tbufDescList[kTbufCount] = TBUF_INIT_VEC;
    UINT8*               prodAckBase = (UINT8 *)(TBUF_BASE_ADDRESS +
                                       tbufDescList[kTbufAckRegisterProd].buffOffset_m);
    UINT8*               consAckBase = (UINT8 *)(TBUF_BASE_ADDRESS +
                                       tbufDescList[kTbufAckRegisterCons].buffOffset_m);

    // Reset appif instance structure
    APPIF_MEMSET(&appifInstance_l, 0 , sizeof(tAppIfInstance));

#if _DEBUG
    if(TRIPLE_BUFFER_COUNT != kTbufCount)
    {
        ret = kAppIfInitError;
        DEBUG_TRACE(DEBUG_LVL_ERROR, "ERROR: Triple buffer count mismatch! -> Correct ipcore settings!\n");
        goto Exit;
    }

    if(UNALIGNED32((UINT8 *) TBUF_BASE_ADDRESS))
    {
        ret = kAppIfInitError;
        DEBUG_TRACE(DEBUG_LVL_ERROR, "ERROR: Triple buffer base address is not aligned!\n");
        goto Exit;
    }

    if( UNALIGNED32(prodAckBase) || UNALIGNED32(consAckBase) )
    {
        ret = kAppIfInitError;
        DEBUG_TRACE(DEBUG_LVL_ERROR, "ERROR: Producer or consumer acknowledge "
                "base address is not aligned!\n");
        goto Exit;
    }
#endif

    ret = tbuf_init();
    if(ret != kAppIfSuccessful)
    {
        DEBUG_TRACE(DEBUG_LVL_ERROR, "ERROR: tbuf_init() failed with 0x%x\n",ret);
        goto Exit;
    }

    ret = fifo_init();
    if(ret != kAppIfSuccessful)
    {
        DEBUG_TRACE(DEBUG_LVL_ERROR, "ERROR: fifo_init() failed with 0x%x\n",ret);
        goto Exit;
    }

#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_ASYNC)) != 0)
    async_init(nodeId_p, SSDO_STUB_OBJECT_INDEX, SSDO_STUB_DATA_OBJECT_INDEX);
#endif

    // Initialize the status module
    statusInitParam.outId_m = kTbufNumStatusOut;
    statusInitParam.pOutTbufBase_m = (tTbufStatusOutStructure *)(TBUF_BASE_ADDRESS +
            tbufDescList[kTbufNumStatusOut].buffOffset_m);
    statusInitParam.pProdAckBase_m = prodAckBase;
    statusInitParam.outTbufSize_m = tbufDescList[kTbufNumStatusOut].buffSize_m;

    statusInitParam.inId_m = kTbufNumStatusIn;
    statusInitParam.pInTbufBase_m = (tTbufStatusInStructure *)(TBUF_BASE_ADDRESS +
            tbufDescList[kTbufNumStatusIn].buffOffset_m);
    statusInitParam.pConsAckBase_m = consAckBase;
    statusInitParam.inTbufSize_m = tbufDescList[kTbufNumStatusIn].buffSize_m;

    ret = status_init(&statusInitParam);
    if(ret != kAppIfSuccessful)
    {
        DEBUG_TRACE(DEBUG_LVL_ERROR, "ERROR: status_init() failed with 0x%x\n",ret);
        goto Exit;
    }

#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_CC)) != 0)
    // Initialize the configuration channel module
    if(ccobject_init(pfnCritSec_p) == FALSE)
    {
        ret = kAppIfConfChanInitError;
        DEBUG_TRACE(DEBUG_LVL_ERROR, "ERROR: ccobject_init() failed with 0x%x\n",ret);
        goto Exit;
    }

    // Initialize the output configuration channel
    occInitParam.id_m = kTbufNumOutputConfChan;
    occInitParam.pTbufBase_m = (tTbufCcStructure *)(TBUF_BASE_ADDRESS +
            tbufDescList[kTbufNumOutputConfChan].buffOffset_m);
    occInitParam.pProdAckBase_m = prodAckBase;
    occInitParam.tbufSize_m = tbufDescList[kTbufNumOutputConfChan].buffSize_m;

    ret = occ_init(&occInitParam);
    if(ret != kAppIfSuccessful)
    {
        DEBUG_TRACE(DEBUG_LVL_ERROR, "ERROR: occ_init() failed with 0x%x\n",ret);
        goto Exit;
    }

    // Initialize the input configuration channel
    iccInitParam.id_m = kTbufNumInputConfChan;
    iccInitParam.pTbufBase_m = (tTbufCcStructure *)(TBUF_BASE_ADDRESS +
            tbufDescList[kTbufNumInputConfChan].buffOffset_m);
    iccInitParam.pConsAckBase_m = consAckBase;
    iccInitParam.tbufSize_m = tbufDescList[kTbufNumInputConfChan].buffSize_m;

    ret = icc_init(&iccInitParam);
    if(ret != kAppIfSuccessful)
    {
        DEBUG_TRACE(DEBUG_LVL_ERROR, "ERROR: icc_init() failed with 0x%x\n",ret);
        goto Exit;
    }
#endif

#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_PDO)) != 0)
    // Initialize the output configuration channel
    rpdoInitParam.id_m = kTbufNumRpdoImage;
    rpdoInitParam.pTbufBase_m = (tTbufRpdoImage *)(TBUF_BASE_ADDRESS +
            tbufDescList[kTbufNumRpdoImage].buffOffset_m);
    rpdoInitParam.pProdAckBase_m = prodAckBase;
    rpdoInitParam.tbufSize_m = tbufDescList[kTbufNumRpdoImage].buffSize_m;

    ret = rpdo_init(&rpdoInitParam);
    if(ret != kAppIfSuccessful)
    {
        DEBUG_TRACE(DEBUG_LVL_ERROR, "ERROR: rpdo_init() failed with 0x%x\n",ret);
        goto Exit;
    }

    // Initialize the input configuration channel
    tpdoInitParam.id_m = kTbufNumTpdoImage;
    tpdoInitParam.pTbufBase_m = (tTbufTpdoImage *)(TBUF_BASE_ADDRESS +
            tbufDescList[kTbufNumTpdoImage].buffOffset_m);
    tpdoInitParam.pConsAckBase_m = consAckBase;
    tpdoInitParam.tbufSize_m = tbufDescList[kTbufNumTpdoImage].buffSize_m;

    ret = tpdo_init(&tpdoInitParam);
    if(ret != kAppIfSuccessful)
    {
        DEBUG_TRACE(DEBUG_LVL_ERROR, "ERROR: tpdo_init() failed with 0x%x\n",ret);
        goto Exit;
    }
#endif

#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_ASYNC)) != 0)
    // Initialize all asynchronous channels
    for(i=0; i < kNumAsyncInstCount; i++)
    {
        asyncInitParam.chanId_m = i;

        asyncInitParam.tbufRxId_m = kTbufNumAsyncReceive0 + i;
        asyncInitParam.pTbufRxBase_m = (tTbufAsyncRxStructure *)(TBUF_BASE_ADDRESS +
                tbufDescList[kTbufNumAsyncReceive0 + i].buffOffset_m);
        asyncInitParam.pProdAckBase_m = prodAckBase;
        asyncInitParam.tbufRxSize_m = tbufDescList[kTbufNumAsyncReceive0 + i].buffSize_m;

        asyncInitParam.tbufTxId_m = kTbufNumAsyncTransmit0 + i;
        asyncInitParam.pTbufTxBase_m = (tTbufAsyncTxStructure *)(TBUF_BASE_ADDRESS +
                tbufDescList[kTbufNumAsyncTransmit0 + i].buffOffset_m);
        asyncInitParam.pConsAckBase_m = consAckBase;
        asyncInitParam.tbufTxSize_m = tbufDescList[kTbufNumAsyncTransmit0 + i].buffSize_m;

        appifInstance_l.instAsyncChan_m[i] = async_create(&asyncInitParam);
        if(appifInstance_l.instAsyncChan_m[i] == NULL)
        {
            ret = kAppIfAsyncInitError;
            DEBUG_TRACE(DEBUG_LVL_ERROR,"ERROR: async_create() failed for instance "
                    "number %d!\n", i );
            goto Exit;
        }
    }
#endif

Exit:
    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Application interface exit function

Close and destroy the application interface and all its submodules.

\ingroup module_appif
*/
//------------------------------------------------------------------------------
void appif_exit(void)
{
#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_ASYNC)) != 0)
    UINT8 i;
#endif

    // Destroy application interface modules
    status_exit();

#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_CC)) != 0)
    ccobject_exit();
    icc_exit();
    occ_exit();
#endif

#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_PDO)) != 0)
    rpdo_exit();
    tpdo_exit();
#endif

#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_ASYNC)) != 0)
    // Destroy all asynchronous channels
    for(i=0; i < kNumAsyncInstCount; i++)
    {
        async_destroy(appifInstance_l.instAsyncChan_m[i]);
    }
#endif
}

//------------------------------------------------------------------------------
/**
\brief    Perform the module configuration steps after the POWERLINK stack init

\return tAppIfStatus
\retval kAppIfSuccessful        On success
\retval Other                   Other initialization error

\ingroup module_appif
*/
//------------------------------------------------------------------------------
tAppIfStatus appif_configureModules(void)
{
    tAppIfStatus ret = kAppIfSuccessful;

#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_PDO)) != 0)
    ret = rpdo_linkRpdos();
    if(ret != kAppIfSuccessful)
    {
        DEBUG_TRACE(DEBUG_LVL_ERROR, "ERROR: Unable to link RPDO to variable! Reason: 0x%x\n", ret);
        goto Exit;
    }

    ret = tpdo_linkTpdos();
    if(ret != kAppIfSuccessful)
    {
        DEBUG_TRACE(DEBUG_LVL_ERROR, "ERROR: Unable to link TPDO to variable! Reason: 0x%x\n", ret);
        goto Exit;
    }
#endif

#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_CC)) != 0)
    ret = appif_initCcObjects();
    if(ret != kAppIfSuccessful)
    {
        DEBUG_TRACE(DEBUG_LVL_ERROR, "ERROR: Unable to init CC object list! Reason: 0x%x\n", ret);
        goto Exit;
    }
#endif

Exit:
    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Process appif background function

Call modules where data needs to be forwarded in the background.

\ingroup module_appif
*/
//------------------------------------------------------------------------------
tAppIfStatus appif_handleAsync(void)
{
    tAppIfStatus ret = kAppIfSuccessful;
#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_ASYNC)) != 0)
    UINT8 i;
#endif

#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_CC)) != 0)
    ret = icc_process();
    if(ret != kAppIfSuccessful)
    {
        DEBUG_TRACE(DEBUG_LVL_ERROR, "ERROR: icc_process() failed with: 0x%x!\n", ret);
        goto Exit;
    }
#endif

#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_ASYNC)) != 0)
    // Process all instantiated asynchronous channels
    for(i=0; i < kNumAsyncInstCount; i++)
    {
        ret = async_process(appifInstance_l.instAsyncChan_m[i]);
        if(ret != kAppIfSuccessful)
        {
            DEBUG_TRACE(DEBUG_LVL_ERROR, "ERROR: async_process() failed for "
                    "instance %d with: 0x%x!\n", i, ret);
            goto Exit;
        }
    }
#endif

Exit:
    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Process appif synchronous functions

Call modules where data needs to be forwarded in the synchronous interrupt.

\ingroup module_appif
*/
//------------------------------------------------------------------------------
tAppIfStatus appif_handleSync(void)
{
    tAppIfStatus ret = kAppIfSuccessful;
#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_ASYNC)) != 0)
    UINT8 i;
#endif

#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_CC)) != 0)
    // Handle configuration channel module
    ret = occ_handleOutgoing();
    if(ret != kAppIfSuccessful)
    {
        goto Exit;
    }

    ret = icc_handleIncoming();
    if(ret != kAppIfSuccessful)
    {
        goto Exit;
    }
#endif

#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_ASYNC)) != 0)
    // Process all instantiated asynchronous channels
    for(i=0; i < kNumAsyncInstCount; i++)
    {
        ret = async_handleIncoming(appifInstance_l.instAsyncChan_m[i]);
        if(ret != kAppIfSuccessful)
        {
            DEBUG_TRACE(DEBUG_LVL_ERROR, "ERROR: async_handleIncoming() failed for "
                    "instance %d with: 0x%x!\n", i, ret);
            goto Exit;
        }
    }
#endif

Exit:
    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Processing of a pdo is finished

\param[in] pdoDir_p       The direction of the PDO

\ingroup module_appif
*/
//------------------------------------------------------------------------------
void appif_pdoProcFinished(tAppIfPdoDir pdoDir_p)
{
#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_PDO)) != 0)
    switch(pdoDir_p)
    {
        case tPdoDirRpdo:
        {
            rpdo_procFinished();
            break;
        }
        case tPdoDirTpdo:
        {
            tpdo_procFinished();
            break;
        }
        default:
        {
            // Should not happen
            break;
        }
    }
#endif
}


//------------------------------------------------------------------------------
/**
\brief    Handle finished SDO transfers

\param[in] pSdoComFinHdl_p             Handle of the finished SDO transfer

\return tAppIfStatus
\retval kAppIfSuccessful                On success


\ingroup module_appif
*/
//------------------------------------------------------------------------------
tAppIfStatus appif_sdoAccFinished(tEplSdoComFinished* pSdoComFinHdl_p )
{
    tAppIfStatus ret = kAppIfSuccessful;

#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_ASYNC)) != 0)
    ret = async_consTxTransferFinished((tAsyncInstance)pSdoComFinHdl_p->m_pUserArg);
#endif

    return ret;
}

//============================================================================//
//            P R I V A T E   F U N C T I O N S                               //
//============================================================================//
/// \name Private Functions
/// \{

/// \}


