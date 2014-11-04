/**
********************************************************************************
\file   hnfappif.c

\brief  Implements the hardware near firmware for the appif interface

Provides the interface from the SHNF to the application interface library.

\ingroup module_hnf
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

#include <shnf/hnf.h>

#include <libappif/appif.h>

#include <apptarget/platform.h>     /* Platform specific functions */
#include <apptarget/benchmark.h>    /* Debug header for performance measurements */

/*============================================================================*/
/*            G L O B A L   D E F I N I T I O N S                             */
/*============================================================================*/

/*----------------------------------------------------------------------------*/
/* const defines                                                              */
/*----------------------------------------------------------------------------*/
#define TBUF_IMAGE_SIZE     ( TBUF_OFFSET_PROACK + TBUF_SIZE_PROACK )   /**< Size of the triple buffer image */

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

/*----------------------------------------------------------------------------*/
/* local types                                                                */
/*----------------------------------------------------------------------------*/

/**
 * \brief Describes a buffer
 */
typedef struct
{
    UINT8 * pBuffer_m;      /**< Pointer to the buffer */
    UINT16 buffSize_m;      /**< Size of the buffer */
} tBuffer;

typedef struct {
    UINT8 tbufMemLayout_m[TBUF_IMAGE_SIZE];                 /**< Local copy of the triple buffer memory */
    UINT8 fCcWriteObjTestEnable_m;                          /**< Enable periodic writing of a cc object */
    tSsdoInstance apSsdoInstance_m[kNumSsdoInstCount];      /**< SSDO channel instance handler array */
    tSsdoRxHandler apfnSsdoRxHandler[kNumSsdoInstCount];    /**< Array of SSDO channel receive callbacks */
    tLogInstance apLogInstance_m[kNumLogInstCount];         /**< Logbook instance handler array */
    tAsyncRxHandler pfnSsdoSnmtRcvHandler_m;                /**< SSDO/SNMT receive handler */
    UINT8 ssdoSnmtRcvBuffer_m[SSDO_STUB_DATA_DOM_SIZE];     /**< Temporary SSDO/SNMT reveive buffer */
    tSyncRxHandler pfnSpdoRxHandler_m;                      /**< SPDO receive handler */
    tSyncTxCreate pfnSpdoTxCreate_m;                        /**< Triggers the building of a transmit spdo frame */
    tBuffer spdo0TxBuffer_m;                                /**< Describes the current transmit buffer of the spdo0 channel */
} tHnfAppifInstance;

/*----------------------------------------------------------------------------*/
/* local vars                                                                 */
/*----------------------------------------------------------------------------*/

static tHnfAppifInstance hnfAppifInstance_l SAFE_INIT_SEKTOR;            /**< Instance of the main module */

/*----------------------------------------------------------------------------*/
/* local function prototypes                                                  */
/*----------------------------------------------------------------------------*/
static BOOL initAppif(void);
static BOOL initModules(void);
static void exitModules(void);
static void genDescList(tBuffDescriptor* pBuffDescList_p, UINT8 buffCount_p);
static BOOL appCbSync(tAppIfTimeStamp* pTimeStamp_p );
static BOOL processSync(UINT32 rpdoRelTimeLow_p,
        tRpdoMappedObj* pRpdoImage_p,
        tTpdoMappedObj* pTpdoImage_p );

#if defined(__NIOS2__) && !defined(ALT_ENHANCED_INTERRUPT_API_PRESENT)
    static void syncHandler(void* pArg_p, void* dwInt_p);
#else
    static void syncHandler(void* pArg_p);
#endif

static void errorHandler(tAppifErrorInfo* pErrorInfo_p);

static BOOL processRxAsyncChannel0(UINT8 * pPayload_p, UINT16 size_p);

static BOOL reformatErrorMessage(tErrorDesc * pErrDesc_p, tLogFormat * pLogData);
static tLogLevel reformatErrorInfo(tErrLevel errLevel_p);

/*============================================================================*/
/*            P U B L I C   F U N C T I O N S                                 */
/*============================================================================*/

/*----------------------------------------------------------------------------*/
/**
\brief    Initialize the SHNF application interface driver

The hardware near firmware driver for the application interface. This driver
provides an interface to POWERLINK by using the application interface library.

\param pHnfInit_p    Module initialization parameters

\retval TRUE    Init of the application interface successfully
\retval FALSE   Error on initialization

\ingroup module_hnf
*/
/*----------------------------------------------------------------------------*/
BOOLEAN hnf_init(tHnfInit * pHnfInit_p)
{
    BOOL fReturn = FALSE;

    APPIF_MEMSET(&hnfAppifInstance_l, 0, sizeof(hnfAppifInstance_l));

    if(pHnfInit_p != NULL)
    {
        if(pHnfInit_p->asyncRcvChan0Handler_m != NULL  &&
           pHnfInit_p->syncRcvHandler_m != NULL      &&
           pHnfInit_p->syncTxBuild_m != NULL          )
        {
            /* Set callback functions */
            hnfAppifInstance_l.pfnSsdoSnmtRcvHandler_m = pHnfInit_p->asyncRcvChan0Handler_m;
            hnfAppifInstance_l.pfnSpdoRxHandler_m = pHnfInit_p->syncRcvHandler_m;
            hnfAppifInstance_l.pfnSpdoTxCreate_m = pHnfInit_p->syncTxBuild_m;

            /* Init array of SSDO channel receive handler */
            hnfAppifInstance_l.apfnSsdoRxHandler[kNumSsdoChan0] = processRxAsyncChannel0;

            /* Initialize target specific functions */
            platform_init();

            /* Initialize the application interface internals */
            if(initAppif())
            {
                fReturn = TRUE;
            }
        }
        else
        {
            errh_postFatalError(kErrSourceHnf, kErrorInvalidParameter, 0);
        }
    }
    else
    {
        errh_postFatalError(kErrSourceHnf, kErrorInvalidParameter, (UINT32)pHnfInit_p);
    }

    return fReturn;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Close the HNF application interface driver

\ingroup module_hnf
*/
/*----------------------------------------------------------------------------*/
void hnf_exit(void)
{
    DEBUG_TRACE(DEBUG_LVL_ALWAYS,"\n\nShutdown application interface -> ");

    exitModules();

    appif_exit();

    MEMSET(&hnfAppifInstance_l, 0, sizeof(tHnfAppifInstance));

    DEBUG_TRACE(DEBUG_LVL_ALWAYS,"SUCCESS!\n");
}

/*----------------------------------------------------------------------------*/
/**
\brief    Process the asynchronous task

\retval TRUE    Processing of the asynchronous task failed
\retval FALSE   Error during processing

\ingroup module_hnf
*/
/*----------------------------------------------------------------------------*/
BOOLEAN hnf_processAsync(void)
{
    BOOLEAN fReturn = FALSE;
    BOOLEAN fError = FALSE;
    UINT8 i;

    if(appif_processAsync())
    {
#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_SSDO)) != 0)
        for(i=0; i < kNumSsdoInstCount; i++)
        {
            if(ssdo_process(hnfAppifInstance_l.apSsdoInstance_m[i]) == FALSE)
            {
                errh_postFatalError(kErrSourceHnf, kErrorAsyncProcessFailed, 0);
                fError = TRUE;
                break;
            }
        }
#endif

#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_LOGBOOK)) != 0)
        if(fError == FALSE)
        {
            for(i=0; i < kNumLogInstCount; i++)
            {
                if(log_process(hnfAppifInstance_l.apLogInstance_m[i]) == FALSE)
                {
                    errh_postFatalError(kErrSourceHnf, kErrorAsyncProcessFailed, 0);
                    fError = TRUE;
                    break;
                }
            }
        }
#endif
    }
    else
    {
        errh_postFatalError(kErrSourceHnf, kErrorAsyncProcessFailed, 0);
    }

    if(fError == FALSE)
        fReturn = TRUE;

    return fReturn;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Transmit a frame over the asynchronous channel 0

This function forwards an asynchronous frame over the asynchronous
channel 0 to the network.

\param pPayload_p     Pointer to the payload to transmit
\param paylLen_p      Length of the payload

\retval TRUE    Transmit of the asynchronous frame successfully
\retval FALSE   Transmit failed or channel is busy

\ingroup module_hnf
*/
/*----------------------------------------------------------------------------*/
BOOLEAN hnf_postAsyncTxChannel0(const UINT8 * pPayload_p, UINT16 paylLen_p)
{
    BOOL fReturn = FALSE;
    tSsdoTxStatus transState;

    transState = ssdo_postPayload(hnfAppifInstance_l.apSsdoInstance_m[kNumSsdoChan0],
                                  (UINT8 *)pPayload_p, paylLen_p);
    if(transState == kSsdoTxStatusSuccessful)
    {
        fReturn = TRUE;
    } /* no else: Error is posted to error handler in shnf module */

    return fReturn;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Get the current transmit buffer from channel0

\param[out] ppTxBuffer_p     Pointer to the current transmit buffer
\param[out] pBuffLen_p       Pointer to the length of the current transmit buffer

\retval TRUE    Transmit buffer valid
\retval FALSE   Error on getting the transmit buffer

\ingroup module_hnf
*/
/*----------------------------------------------------------------------------*/
BOOLEAN hnf_getAsyncTxBufferChannel0(UINT8 ** ppTxBuffer_p, UINT16 * pBuffLen_p)
{
    BOOLEAN fReturn = FALSE;

    if(ssdo_getCurrentTxBuffer(hnfAppifInstance_l.apSsdoInstance_m[kNumSsdoChan0],
                               ppTxBuffer_p, pBuffLen_p))
    {
        fReturn = TRUE;
    } /* no else: Error is posted to error handler in shnf module */

    return fReturn;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Call this function to free the receive message of async channel0

\ingroup module_hnf
*/
/*----------------------------------------------------------------------------*/
void hnf_finishedAsyncRxChannel0(void)
{
    ssdo_receiveMsgFinished(hnfAppifInstance_l.apSsdoInstance_m[kNumSsdoChan0]);
}


/*----------------------------------------------------------------------------*/
/**
\brief    Forward a logbook entry over logbook channel 0

\param pErrDesc_p       Pointer to the error descriptor

\retval TRUE    Post of the entry successful
\retval FALSE   Error on posting

\ingroup module_hnf
*/
/*----------------------------------------------------------------------------*/
BOOLEAN hnf_postLogChannel0(tErrorDesc * pErrDesc_p)
{
    BOOL fReturn = FALSE;
    tLogTxStatus transState;
    tLogFormat * pLogData = NULL;

    /* Acquire buffer for new log message */
    if(log_getCurrentLogBuffer(hnfAppifInstance_l.apLogInstance_m[kNumLogChan0],
                               &pLogData))
    {
        /* Adapt error message format to special interface type */
        if(reformatErrorMessage(pErrDesc_p, pLogData))
        {
            /* Post the logging data to the logger module */
            transState = log_postLogEntry(hnfAppifInstance_l.apLogInstance_m[kNumLogChan0],
                                          pLogData);
            if(transState == kLogTxStatusSuccessful)
            {
                fReturn = TRUE;
            }   /* no else: Error will be indicated by the caller of this function*/
        }
    }   /* no else: Error will be indicated by the caller of this function */

    return fReturn;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Transmit a isochronous frame over the synchronous channel

This function forwards an asynchronous frame over the asynchronous
channel 0 to the network.

\param pPayload_p     Pointer to the payload to transmit
\param paylLen_p      Length of the payload

\retval TRUE    Transmit of the synchronous frame successful
\retval FALSE   Error on transmitting

\ingroup module_hnf
*/
/*----------------------------------------------------------------------------*/
BOOLEAN hnf_postSyncTx(const UINT8 * pPayload_p, UINT16 paylLen_p)
{
    UNUSED_PARAMETER(pPayload_p);
    UNUSED_PARAMETER(paylLen_p);

    return TRUE;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Get the current synchronous transmit buffer

\param[out] ppTxBuffer_p     Pointer to the current transmit buffer
\param[out] pBuffLen_p       Pointer to the length of the current transmit buffer

\retval TRUE    Transmit buffer valid
\retval FALSE   Error on getting the tx buffer

\ingroup module_hnf
*/
/*----------------------------------------------------------------------------*/
BOOLEAN hnf_getSyncTxBuffer(UINT8 ** ppTxBuffer_p, UINT16 * pBuffLen_p)
{
    BOOL fReturn = FALSE;

    if(hnfAppifInstance_l.spdo0TxBuffer_m.pBuffer_m != NULL &&
       hnfAppifInstance_l.spdo0TxBuffer_m.buffSize_m > 0     )
    {
        *ppTxBuffer_p = hnfAppifInstance_l.spdo0TxBuffer_m.pBuffer_m;
        *pBuffLen_p = hnfAppifInstance_l.spdo0TxBuffer_m.buffSize_m;
        fReturn = TRUE;
    } /* no else: Error is posted to error handler in shnf module */

    return fReturn;
}

/*============================================================================*/
/*            P R I V A T E   F U N C T I O N S                               */
/*============================================================================*/
/* \name Private Functions */
/* \{ */

/*----------------------------------------------------------------------------*/
/**
\brief    Initialize the application interface

\retval TRUE    Transmit of the asynchronous frame successfully
\retval FALSE   Transmit failed

\ingroup module_hnf
*/
/*----------------------------------------------------------------------------*/
static BOOL initAppif(void)
{
    BOOL fReturn = FALSE;
    tAppIfInitParam     initParam;
    tBuffDescriptor     buffDescList[kTbufCount];

    APPIF_MEMSET(&buffDescList, 0, sizeof(buffDescList));

    /* Generate buffer descriptor list */
    genDescList(&buffDescList[0], kTbufCount);

    /* Enable test of configuration channel */
    hnfAppifInstance_l.fCcWriteObjTestEnable_m = TRUE;

    /* initialize and start the application interface internals */
    DEBUG_TRACE(DEBUG_LVL_ALWAYS,"\nInitialize appif internals -> ");

    initParam.pBuffDescList_m = &buffDescList[0];
    initParam.pfnStreamHandler_m = platform_spiCommand;
    initParam.pfnErrorHandler_m = errorHandler;
    initParam.idConsAck_m = kTbufAckRegisterCons;
    initParam.idProdAck_m = kTbufAckRegisterProd;
    initParam.idFirstProdBuffer_m = TBUF_NUM_CON + 1;   /* Add one buffer for the consumer ACK register */

    if(appif_init(&initParam))
    {
        DEBUG_TRACE(DEBUG_LVL_ALWAYS, "SUCCESS!\n");

        /* initialize and start the application interface modules */
        DEBUG_TRACE(DEBUG_LVL_ALWAYS,"\nInitialize appif modules -> ");
        if(initModules())
        {
            DEBUG_TRACE(DEBUG_LVL_ALWAYS, "SUCCESS!\n");

            /* Set the base address of the PDO image */
            hnfAppifInstance_l.spdo0TxBuffer_m.pBuffer_m = (UINT8 *)pdo_getTpdoImage();
            hnfAppifInstance_l.spdo0TxBuffer_m.buffSize_m = TX_SPDO_SIZE;

            /* initialize PCP interrupt handler*/
            DEBUG_TRACE(DEBUG_LVL_ALWAYS,"\nInitialize sync interrupt -> ");
            if(platform_initSyncInterrupt(syncHandler))
            {
                DEBUG_TRACE(DEBUG_LVL_ALWAYS, "SUCCESS!\n");
                fReturn = TRUE;
            }
            else
            {   /* platform_initSyncInterrupt() failed! */
                DEBUG_TRACE(DEBUG_LVL_ALWAYS, "ERROR!\n");
            }
        }
        else
        {   /* Error on libpsi module initialization! Error reported via errorHandler() */
            DEBUG_TRACE(DEBUG_LVL_ALWAYS, "ERROR!\n");
        }
    }
    else
    {   /* Error on libpsi initialization! Error reported via errorHandler() */
        DEBUG_TRACE(DEBUG_LVL_ALWAYS, "ERROR!\n");
    }

    return fReturn;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Initialize application interface modules

\retval TRUE    Library module initialization successful
\retval FALSE   Error during library module initialization

\ingroup module_hnf
*/
/*----------------------------------------------------------------------------*/
static BOOL initModules(void)
{
    BOOL fReturn = FALSE;
    UINT8 i;
    tStatusInitParam   statusInitParam;
#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_PDO)) != 0)
    tPdoInitParam      pdoInitParam;
#endif
#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_SSDO)) != 0)
    tSsdoInitParam     ssdoInitParam;
#endif
#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_LOGBOOK)) != 0)
    tLogInitParam      logInitParam;
#endif
#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_CC)) != 0)
    tCcInitParam       ccInitParam;
#endif

    /* Initialize the status module */
    statusInitParam.pfnAppCbSync_m = appCbSync;
    statusInitParam.buffOutId_m = kTbufNumStatusOut;
    statusInitParam.buffInId_m = kTbufNumStatusIn;

    if(status_init(&statusInitParam) == FALSE)
    {
        DEBUG_TRACE(DEBUG_LVL_ERROR,"ERROR: status_init() failed!\n");
        return fReturn;
    }

#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_PDO)) != 0)
    /* Initialize the PDO module */
    pdoInitParam.buffIdRpdo_m = kTbufNumRpdoImage;
    pdoInitParam.buffIdTpdo_m = kTbufNumTpdoImage;

    if(pdo_init(processSync, &pdoInitParam) == FALSE)
    {
        DEBUG_TRACE(DEBUG_LVL_ERROR,"ERROR: pdo_init() failed!\n");
        return fReturn;
    }
#endif

#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_SSDO)) != 0)
    /* Initialize all needed SSDO channels */
    for(i=0; i<kNumSsdoInstCount; i++)
    {
        ssdoInitParam.buffIdRx_m = kTbufNumSsdoReceive0 + i;
        ssdoInitParam.buffIdTx_m = kTbufNumSsdoTransmit0 + i;

        ssdoInitParam.pfnRxHandler_m = hnfAppifInstance_l.apfnSsdoRxHandler[i];

        hnfAppifInstance_l.apSsdoInstance_m[i] = ssdo_create(kNumSsdoChan0 + i, &ssdoInitParam);
        if(hnfAppifInstance_l.apSsdoInstance_m[i] == NULL)
        {
            DEBUG_TRACE(DEBUG_LVL_ERROR,"ERROR: ssdo_create() failed for instance %d!\n", i);
            return fReturn;
        }
    }
#endif

#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_LOGBOOK)) != 0)
    /* Initialize all needed logbook channels */
    for(i=0; i<kNumLogInstCount; i++)
    {
        logInitParam.buffIdTx_m = kTbufNumLogbook0 + i;

        hnfAppifInstance_l.apLogInstance_m[i] = log_create(kNumLogChan0 + i, &logInitParam);
        if(hnfAppifInstance_l.apLogInstance_m[i] == NULL)
        {
            DEBUG_TRACE(DEBUG_LVL_ERROR,"ERROR: log_create() failed for instance %d!\n", i);
            return fReturn;
        }
    }
#endif

#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_CC)) != 0)
    /* Initialize configuration channel (CC) */
    ccInitParam.iccId_m = kTbufNumInputConfChan;
    ccInitParam.occId_m = kTbufNumOutputConfChan;
    ccInitParam.pfnCritSec_p = platform_enterCriticalSection;

    if(cc_init(&ccInitParam) == FALSE)
    {
        DEBUG_TRACE(DEBUG_LVL_ERROR,"ERROR: cc_init() failed!\n");
        return fReturn;
    }
#endif

    /* All modules successfully initialized! -> Set return to success! */
    fReturn = TRUE;

    return fReturn;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Destroy application interface modules

\ingroup module_hnf
*/
/*----------------------------------------------------------------------------*/
static void exitModules(void)
{
#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_SSDO)) != 0)
    UINT8 i;
#endif
#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_LOGBOOK)) != 0)
    UINT8 j;
#endif

    /* Destroy all library modules */
    status_exit();

#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_PDO)) != 0)
    pdo_exit();
#endif

#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_CC)) != 0)
    cc_exit();
#endif

#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_SSDO)) != 0)
    for(i=0; i<kNumSsdoInstCount; i++)
        ssdo_destroy(hnfAppifInstance_l.apSsdoInstance_m[i]);
#endif

#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_LOGBOOK)) != 0)
    for(j=0; j<kNumLogInstCount; j++)
        log_destroy(hnfAppifInstance_l.apLogInstance_m[j]);
#endif
}

/*----------------------------------------------------------------------------*/
/**
\brief    Generate a list of buffer descriptors for the useage inside the library

\param[out] pBuffDescList_p     Buffer descriptor list in library format
\param[in]  buffCount_p         Count of buffer descriptors

\ingroup module_hnf
*/
/*----------------------------------------------------------------------------*/
static void genDescList(tBuffDescriptor* pBuffDescList_p, UINT8 buffCount_p)
{
    UINT8 i;
    tBuffDescriptor* pBuffDec = pBuffDescList_p;
    tTbufDescriptor tbufDescList[] = TBUF_INIT_VEC;

    /* Generate a descriptor list which can be used in the library */
    for(i=0; i < buffCount_p; i++, pBuffDec++)
    {
        pBuffDec->pBuffBase_m = (UINT8 *)((UINT32)&hnfAppifInstance_l.tbufMemLayout_m + (UINT32)tbufDescList[i].buffOffset_m);
        pBuffDec->buffSize_m = tbufDescList[i].buffSize_m;
    }

}


/*----------------------------------------------------------------------------*/
/**
\brief    Application synchronization to the POWERLINK cycle

 \param[in] pTimeStamp_p             Time information of the current interrupt.

 \return This function always succeeds

\ingroup module_hnf
*/
/*----------------------------------------------------------------------------*/
static BOOL appCbSync(tAppIfTimeStamp* pTimeStamp_p )
{
    UNUSED_PARAMETER(pTimeStamp_p);

    /* Note: This is your place to do local synchronization. Synchronize */
    /*       your local clock here by using the provided timestamp. */

    return TRUE;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Process synchronous data

\param[in] rpdoRelTimeLow_p     Relative time low value
\param[in] pRpdoImage_p         Pointer to the RPDO objects
\param[in] pTpdoImage_p         Pointer to the TPDO objects

 \return This function always succeeds

\ingroup module_hnf
*/
/*----------------------------------------------------------------------------*/
static BOOL processSync(UINT32 rpdoRelTimeLow_p,
        tRpdoMappedObj* pRpdoImage_p,
        tTpdoMappedObj* pTpdoImage_p )
{
    BOOL fReturn = FALSE;

    UNUSED_PARAMETER(rpdoRelTimeLow_p);
    UNUSED_PARAMETER(pTpdoImage_p);

    if(hnfAppifInstance_l.pfnSpdoRxHandler_m != NULL)
    {
        /* Call register receive handler */
        hnfAppifInstance_l.pfnSpdoRxHandler_m(&pRpdoImage_p->spdo0[0], RX_SPDO0_SIZE);

        if(hnfAppifInstance_l.pfnSpdoTxCreate_m != NULL)
        {
            /* Call transmit create */
            hnfAppifInstance_l.pfnSpdoTxCreate_m();
            fReturn = TRUE;
        }
        else
        {
            errh_postFatalError(kErrSourceHnf, kErrorCallbackNotInitialized, 0);
        }
    }
    else
    {
        errh_postFatalError(kErrSourceHnf, kErrorCallbackNotInitialized, 0);
    }

    return fReturn;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Synchronous interrupt handler

\param[in] pArg_p       Interrupt handler arguments

 appif_syncIntH() implements the synchronous data interrupt. The PCP asserts
 the interrupt when periodic data is ready to transfer.

\ingroup module_hnf
*/
/*----------------------------------------------------------------------------*/
#if defined(__NIOS2__) && !defined(ALT_ENHANCED_INTERRUPT_API_PRESENT)
static void syncHandler(void* pArg_p, void* dwInt_p)
#else
static void syncHandler(void* pArg_p)
#endif
{
    UNUSED_PARAMETER(pArg_p);

    BENCHMARK_MOD_01_SET(0);

    /* Call internal synchronous process function */
    if(appif_processSync() == FALSE)
    {
        errh_postFatalError(kErrSourceHnf, kErrorSyncProcessFailed, 0);
    }

    platform_ackSyncIrq();

    BENCHMARK_MOD_01_RESET(0);

}

/*----------------------------------------------------------------------------*/
/**
\brief    Error handler callback function

\param pErrorInfo_p     Provides details about the occurred error

\ingroup module_hnf
*/
/*----------------------------------------------------------------------------*/
static void errorHandler(tAppifErrorInfo* pErrorInfo_p)
{
    tErrorDesc errInfo;

    MEMSET(&errInfo, 0, sizeof(tErrorDesc));

    errInfo.source_m = kErrSourceHnf;       /* Source is always the HNF in this case */
    errInfo.fFailSafe_m = FALSE;
    errInfo.class_m = kErrLevelFatal;       /* All errors in the HNF are critical */

    if(pErrorInfo_p != NULL)
    {
        errInfo.unit_m = (UINT8)pErrorInfo_p->srcModule_m;
        errInfo.code_m = (UINT8)pErrorInfo_p->errCode_m;
    }
    else
    {
        /* Error during posting of the error itself */
        errInfo.unit_m = kAppIfModuleExternal;
        errInfo.code_m = kAppIfGeneralError;
    }

    /* Forward error to SN error handler */
    errh_postError(&errInfo);
}

/*----------------------------------------------------------------------------*/
/**
\brief    Process the received frame from asynchronous channel 0

\param pPayload_p    Pointer to the received payload
\param size_p        The length of the received payload

\retval TRUE        Processing of receive frame successful
\retval FALSE       Error during processing

\ingroup module_hnf
*/
/*----------------------------------------------------------------------------*/
static BOOL processRxAsyncChannel0(UINT8 * pPayload_p, UINT16 size_p)
{
    BOOL fReturn = FALSE;

    if(hnfAppifInstance_l.pfnSsdoSnmtRcvHandler_m != NULL)
    {
        /* The openSAFETY stack modifies the second subframe in place
         * This modification is overwritten by the SPI stream which results in
         * a CRC error. Therefore we need to save the asynchronous payload
         * in this temporary buffer to prevent the manipulation from the
         * stream.
         */
        MEMCOPY(&hnfAppifInstance_l.ssdoSnmtRcvBuffer_m, pPayload_p, size_p);

        /* Forward frame to SHNF main module */
        if(hnfAppifInstance_l.pfnSsdoSnmtRcvHandler_m((UINT8 *)&hnfAppifInstance_l.ssdoSnmtRcvBuffer_m, size_p))
        {
            fReturn = TRUE;
        }
    }
    else
    {
        errh_postFatalError(kErrSourceHnf, kErrorCallbackNotInitialized, 0);
    }

    return fReturn;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Reformat the error message to special interface type

\param pErrDesc_p    The error descriptor from the SAPL
\param pLogData      The error message in interface form

\return TRUE on success; FALSE on error

\ingroup module_hnf
*/
/*----------------------------------------------------------------------------*/
static BOOL reformatErrorMessage(tErrorDesc * pErrDesc_p, tLogFormat * pLogData)
{
    BOOL fReturn = FALSE;
    tLogLevel errLevel;
    UINT16 errSource;

    /* Reformat the error info type */
    errLevel = reformatErrorInfo(pErrDesc_p->class_m);
    if(errLevel != kLogLevelInvalid)
    {
        ami_setUint8Le((UINT8 *)&pLogData->level_m, (UINT8)errLevel);

        errSource = (UINT16)(((UINT16)pErrDesc_p->unit_m << 8) | ((UINT8)pErrDesc_p->source_m));
        ami_setUint16Le((UINT8 *)&pLogData->source_m, errSource);

        ami_setUint32Le((UINT8 *)&pLogData->code_m, (UINT32)pErrDesc_p->code_m);
        ami_setUint32Le((UINT8 *)&pLogData->addInfo_m, pErrDesc_p->addInfo_m);

        fReturn = TRUE;
    }   /* no else: Error handled by caller */

    return fReturn;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Reformat the error level to special interface  type

\param errLevel_p    The stack error level type

\return The interface error level

\ingroup module_hnf
*/
/*----------------------------------------------------------------------------*/
static tLogLevel reformatErrorInfo(tErrLevel errLevel_p)
{
    tLogLevel levelRet = kLogLevelInvalid;

    /* Reformat error descriptor to internal logger format */
    switch(errLevel_p)
    {
        case kErrLevelInfo:
            levelRet = kLogLevelInfo;
            break;
        case kErrLevelMinor:
            levelRet = kLogLevelMinor;
            break;
        case kErrLevelFatal:
            levelRet = kLogLevelFatal;
            break;
        default:
            /* Error occurred: is handled by the calling function */
            break;
    }

    return levelRet;
}

/* \} */
