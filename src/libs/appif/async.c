/**
********************************************************************************
\file   async.c

\brief  Creates an asynchronous channel for receive and transmit

This module handles the asynchronous channels between the PCP and the application.
For each channel a receive and a transmit buffer needs to be determined.
It forwards the received data to the application.

\ingroup module_async
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

#include <libappif/internal/asyncinst.h>
#include <libappif/internal/async.h>

#include <libappif/internal/status.h>
#include <libappif/internal/stream.h>

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

#define ASYNC_TX_TIMEOUT_CYCLE_COUNT        400     ///< Number of cycles after a transmit has a timeout

//------------------------------------------------------------------------------
// local types
//------------------------------------------------------------------------------

/**
 * \brief Status of the asynchronous channel
 */
typedef enum {
    kChanStatusInvalid   = 0x00,    ///< Invalid channel status
    kChanStatusBusy      = 0x01,    ///< Channel is currently busy
    kChanStatusFree      = 0x02,    ///< Channel is free for transmission
} tAsyncChanStatus;

//------------------------------------------------------------------------------
// local vars
//------------------------------------------------------------------------------

static struct eAsyncInstance          asyncInstance_l[kNumAsyncInstCount];

//------------------------------------------------------------------------------
// local function prototypes
//------------------------------------------------------------------------------
static BOOL async_initReceiveBuffer(tAsyncChanNum chanId_p,
        tTbufNumLayout rxBuffId_p);
static BOOL async_initTransmitBuffer(tAsyncChanNum chanId_p,
        tTbufNumLayout txBuffId_p);
static BOOL async_handleRxFrame(tAsyncInstance pInstance_p);
static BOOL async_handleTxFrame(tAsyncInstance pInstance_p);
static BOOL async_receiveFrame(UINT8* pBuffer_p, UINT16 bufSize_p,
        void* pUserArg_p);
static void async_changeLocalSeqNr(tSeqNrValue* pSeqNr_p);
static tAsyncChanStatus async_checkChannelStatus(tAsyncInstance pInstance_p);

//============================================================================//
//            P U B L I C   F U N C T I O N S                                 //
//============================================================================//

//------------------------------------------------------------------------------
/**
\brief    Initialize the asynchronous channel module

\ingroup module_async
*/
//------------------------------------------------------------------------------
void async_init(void)
{
    APPIF_MEMSET(&asyncInstance_l, 0 , sizeof(struct eAsyncInstance) * kNumAsyncInstCount);
}


//------------------------------------------------------------------------------
/**
\brief    Create a asynchronous channel instance

Instantiate a new asynchronous channel which provides send and receive
functionality for the upper and lower layers.

\param[in]  chanId_p         Id of the asynchronous channel
\param[in]  pInitParam_p     Asynchronous module initialization structure

\return tAsyncInstance
\retval Address              Pointer to the instance of the channel
\retval Null                 Unable to allocate instance

\ingroup module_async
*/
//------------------------------------------------------------------------------
tAsyncInstance async_create(tAsyncChanNum chanId_p, tAsyncInitParam* pInitParam_p)
{
    tAsyncInstance pInstance = NULL;

    if(chanId_p >= kNumAsyncInstCount ||
       pInitParam_p == NULL            )
    {
        error_setError(kAppIfModuleAsync, kAppIfAsyncInitError);
    }
    else
    {
        // Get asynchronous buffer parameters from stream module
        if(async_initReceiveBuffer(chanId_p, pInitParam_p->buffIdRx_m) != FALSE)
        {
            if(async_initTransmitBuffer(chanId_p, pInitParam_p->buffIdTx_m) != FALSE)
            {
                // Save channel Id
                asyncInstance_l[chanId_p].chanId_m = chanId_p;

                // Save id of producing and consuming buffers
                asyncInstance_l[chanId_p].txBuffParam_m.idTxBuff_m = pInitParam_p->buffIdTx_m;
                asyncInstance_l[chanId_p].rxBuffParam_m.idRxBuff_m = pInitParam_p->buffIdRx_m;

                // Set sequence number init value
                asyncInstance_l[chanId_p].txBuffParam_m.currTxSeqNr_m = kSeqNrValueFirst;

                // Register receive handler
                asyncInstance_l[chanId_p].rxBuffParam_m.pfnRxHandler_m = pInitParam_p->pfnRxHandler_m;

                // Set valid instance id
                pInstance = &asyncInstance_l[chanId_p];
            }
        }
    }

    return pInstance;
}

//------------------------------------------------------------------------------
/**
\brief    Destroy an asynchronous channel

\param[in]  pInstance_p       The instance to destroy

\ingroup module_async
*/
//------------------------------------------------------------------------------
void async_destroy(tAsyncInstance pInstance_p)
{
    if(pInstance_p != NULL)
    {
        APPIF_MEMSET(pInstance_p, 0, sizeof(struct eAsyncInstance));

        timeout_destroy(pInstance_p->txBuffParam_m.pTimeoutInst_m);
    }
}

//------------------------------------------------------------------------------
/**
\brief    Post a frame for transmission over the asynchronous channel

\param[in]  pInstance_p     Asynchronous module instance
\param[in]  pPayload_p      Pointer to the payload to send
\param[in]  paylSize_p      Size of the payload to send

\return tAsyncTxStatus
\retval tAsyncTxStatusSuccessfull    Successfully posted payload to buffer
\retval tAsyncTxStatusBusy           Unable to post payload to asynchronous channel
\retval tAsyncTxStatusError          Error while posting payload to the transmit channel

\ingroup module_async
*/
//------------------------------------------------------------------------------
tAsyncTxStatus async_postPayload(tAsyncInstance pInstance_p, UINT8* pPayload_p,
        UINT16 paylSize_p)
{
    tAsyncTxStatus chanState = kAsyncTxStatusError;

    if(pInstance_p == NULL  ||
       pPayload_p == NULL    )
    {
        error_setError(kAppIfModuleAsync, kAppIfAsyncSendError);
    }
    else
    {
        // Check if payload fits inside the buffer
        if(paylSize_p > sizeof(pInstance_p->txBuffParam_m.asyncTxBuffer_m.pAsyncTxPayl_m->tssdoTransmitData_m) ||
           paylSize_p == 0                             )
        {
            error_setError(kAppIfModuleAsync, kAppIfAsyncTxConsSizeInvalid);
        }
        else
        {
            // Check if buffer is free for filling
            if(pInstance_p->txBuffParam_m.asyncTxBuffer_m.isLocked_m == FALSE)
            {
                // Fill buffer
                APPIF_MEMCPY(pInstance_p->txBuffParam_m.asyncTxBuffer_m.pAsyncTxPayl_m->tssdoTransmitData_m,
                        pPayload_p, paylSize_p);

                // Set transmit size
                AmiSetWordToLe((UINT8*)&pInstance_p->txBuffParam_m.asyncTxBuffer_m.pAsyncTxPayl_m->paylSize_m, paylSize_p);

                // Set sequence number in next tx buffer
                AmiSetByteToLe((UINT8*)&pInstance_p->txBuffParam_m.asyncTxBuffer_m.pAsyncTxPayl_m->seqNr_m,
                        pInstance_p->txBuffParam_m.currTxSeqNr_m);

                // Lock buffer for transmission
                pInstance_p->txBuffParam_m.asyncTxBuffer_m.isLocked_m = TRUE;

                // Enable transmit timer
                timeout_startTimer(pInstance_p->txBuffParam_m.pTimeoutInst_m);

                // Acknowledge producing transmit buffer
                stream_ackBuffer(pInstance_p->txBuffParam_m.idTxBuff_m);

                chanState = kAsyncTxStatusSuccessful;
            }
            else
            {
                chanState = kAsyncTxStatusBusy;
            }
        }
    }

    return chanState;
}

//------------------------------------------------------------------------------
/**
\brief    Process the asynchronous module

\param[in]  pInstance_p     Asynchronous module instance

\return BOOL
\retval TRUE        Asynchronous frames processed successfully
\retval FALSE       Error while processing asynchronous frames

\ingroup module_async
*/
//------------------------------------------------------------------------------
BOOL async_process(tAsyncInstance pInstance_p)
{
    BOOL fReturn = FALSE;

    // Process incoming frames
    if(async_handleRxFrame(pInstance_p) != FALSE)
    {
        // Process transmit frames
        if(async_handleTxFrame(pInstance_p) != FALSE)
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

//------------------------------------------------------------------------------
/**
\brief    Initialize the asynchronous receive buffer

\param[in] chanId_p                 Id of the asynchronous channel
\param[in] rxBuffId_p               Id of the asynchronous receive buffer

\return BOOL
\retval TRUE    Initialization successful
\retval FALSE   Error while initializing

\ingroup module_async
*/
//------------------------------------------------------------------------------
static BOOL async_initReceiveBuffer(tAsyncChanNum chanId_p,
        tTbufNumLayout rxBuffId_p)
{
    BOOL fReturn = FALSE;
    tBuffDescriptor* pDescAsyncRcv;

    pDescAsyncRcv = stream_getBufferParam(rxBuffId_p);
    if(pDescAsyncRcv != NULL)
    {
        if(pDescAsyncRcv->buffSize_m == sizeof(tTbufAsyncRxStructure))
        {
            // Remember buffer address for later usage
            asyncInstance_l[chanId_p].rxBuffParam_m.pAsyncRxBuffer_m =
                    (tTbufAsyncRxStructure *)pDescAsyncRcv->pBuffBase_m;

            // Register frame receive post action
            if(stream_registerAction(kStreamActionPost, rxBuffId_p,
                    async_receiveFrame, (void *)&asyncInstance_l[chanId_p]) != FALSE)
            {
                fReturn = TRUE;
            }
        }
        else
        {
            error_setError(kAppIfModuleAsync, kAppIfAsyncBufferSizeMismatch);
        }
    }
    else
    {
        error_setError(kAppIfModuleAsync, kAppIfAsyncInvalidBuffer);
    }

    return fReturn;
}

//------------------------------------------------------------------------------
/**
\brief    Initialize the asynchronous transmit buffer

\param[in] chanId_p                 Id of the asynchronous channel
\param[in] txBuffId_p               Id of the asynchronous transmit buffer

\return BOOL
\retval TRUE    Initialization successful
\retval FALSE   Error while initializing

\ingroup module_async
*/
//------------------------------------------------------------------------------
static BOOL async_initTransmitBuffer(tAsyncChanNum chanId_p,
        tTbufNumLayout txBuffId_p)
{
    BOOL fReturn = FALSE;
    tBuffDescriptor* pDescAsyncTrans;

    pDescAsyncTrans = stream_getBufferParam(txBuffId_p);
    if(pDescAsyncTrans != NULL)
    {
        if(pDescAsyncTrans->buffSize_m == sizeof(tTbufAsyncTxStructure))
        {
            // Remember buffer address for later usage
            asyncInstance_l[chanId_p].txBuffParam_m.asyncTxBuffer_m.pAsyncTxPayl_m =
                    (tTbufAsyncTxStructure *)pDescAsyncTrans->pBuffBase_m;

            // Initialize asynchronous transmit timeout instance
            asyncInstance_l[chanId_p].txBuffParam_m.pTimeoutInst_m = timeout_create(
                    ASYNC_TX_TIMEOUT_CYCLE_COUNT);
            if(asyncInstance_l[chanId_p].txBuffParam_m.pTimeoutInst_m != NULL)
            {
                fReturn = TRUE;
            }
            else
            {
                error_setError(kAppIfModuleAsync, kAppIfAsyncInitError);
            }
        }
        else
        {
            error_setError(kAppIfModuleAsync, kAppIfAsyncBufferSizeMismatch);
        }
    }
    else
    {
        error_setError(kAppIfModuleAsync, kAppIfAsyncInvalidBuffer);
    }

    return fReturn;
}

//------------------------------------------------------------------------------
/**
\brief    Process asynchronous receive frames

\param[in] pInstance_p     Async module instance

\return BOOL
\retval TRUE       Successfully processed asynchronous receive frame
\retval FALSE      Error while processing

\ingroup module_async
*/
//------------------------------------------------------------------------------
static BOOL async_handleRxFrame(tAsyncInstance pInstance_p)
{
    BOOL fReturn = FALSE;
    UINT8*  pRxBuffer;
    UINT16  rxBuffSize;

    if(pInstance_p->rxBuffParam_m.fRxFrameIncoming_m != FALSE)
    {
        // Frame incoming -> forward to the user
        pRxBuffer = &pInstance_p->rxBuffParam_m.pAsyncRxBuffer_m->ssdoStubDataDom_m[0];
        rxBuffSize = AmiGetWordFromLe((UINT8 *)&pInstance_p->rxBuffParam_m.pAsyncRxBuffer_m->paylSize_m);

        // Call asynchronous user handler
        if(pInstance_p->rxBuffParam_m.pfnRxHandler_m(pRxBuffer, rxBuffSize))
        {
            fReturn = TRUE;
        }
        else
        {
            error_setError(kAppIfModuleAsync, kAppIfAsyncProcessingFailed);
        }

        // Access finished -> Unblock channel by writing current sequence number to status field!
        status_setAsyncRxChanFlag(pInstance_p->chanId_m,
                pInstance_p->rxBuffParam_m.currRxSeqNr_m);

        pInstance_p->rxBuffParam_m.fRxFrameIncoming_m = FALSE;
    }
    else
    {
        // Nothing to process -> Success anyway!
        fReturn = TRUE;
    }

    return fReturn;
}

//------------------------------------------------------------------------------
/**
\brief    Process asynchronous transmit frames

\param[in]  pInstance_p     Asynchronous module instance

\return BOOL
\retval TRUE        Successfully handled transmit frame
\retval FALSE       Error while handling transmit frame

\ingroup module_async
*/
//------------------------------------------------------------------------------
static BOOL async_handleTxFrame(tAsyncInstance pInstance_p)
{
    BOOL fReturn = FALSE;
    tAsyncChanStatus  txChanState;
    tTimerStatus timerState;

    if(pInstance_p->txBuffParam_m.asyncTxBuffer_m.isLocked_m != FALSE)
    {
        // Check if channel is ready for transmission
        txChanState = async_checkChannelStatus(pInstance_p);
        if(txChanState == kChanStatusFree)
        {
            // Ongoing message is acknowledged
            pInstance_p->txBuffParam_m.asyncTxBuffer_m.isLocked_m = FALSE;

            // Increment local sequence number
            async_changeLocalSeqNr(&pInstance_p->txBuffParam_m.currTxSeqNr_m);

            // When timer is running and ACK occurred -> Stop timer instance!
            timerState = timeout_isRunning(pInstance_p->txBuffParam_m.pTimeoutInst_m);
            if(timerState == kTimerStateRunning)
            {
                timeout_stopTimer(pInstance_p->txBuffParam_m.pTimeoutInst_m);
            }

            fReturn = TRUE;
        }
        else if(txChanState == kChanStatusBusy)
        {
            // Check if timeout counter is expired
            timerState = timeout_checkExpire(pInstance_p->txBuffParam_m.pTimeoutInst_m);
            if(timerState == kTimerStateExpired)
            {
                // Timeout occurred -> Increment local sequence number!
                async_changeLocalSeqNr(&pInstance_p->txBuffParam_m.currTxSeqNr_m);

                // Unlock channel anyway!
                pInstance_p->txBuffParam_m.asyncTxBuffer_m.isLocked_m = FALSE;
            }

            fReturn = TRUE;
        }
    }
    else
    {
        // Nothing to do! -> Success!
        fReturn = TRUE;
    }

    return fReturn;
}

//------------------------------------------------------------------------------
/**
\brief    Check for incoming frame from asynchronous channel

\param[in] pBuffer_p        Pointer to the base address of the buffer
\param[in] bufSize_p        Size of the buffer
\param[in] pUserArg_p       The user argument

\return BOOL
\retval TRUE          Successfully processed receive frame
\retval FALSE         Error while processing receive frame

\ingroup module_async
*/
//------------------------------------------------------------------------------
static BOOL async_receiveFrame(UINT8* pBuffer_p, UINT16 bufSize_p,
        void* pUserArg_p)
{
    BOOL fReturn = FALSE;
    tAsyncInstance pInstance;
    tTbufAsyncRxStructure*  pAsyncRxBuff;
    tSeqNrValue  currSeqNr = kSeqNrValueInvalid;

    if(pUserArg_p == NULL)
    {
        error_setError(kAppIfModuleAsync, kAppIfAsyncInvalidParameter);
    }
    else
    {
        // Check size of buffer
        if(bufSize_p != sizeof(tTbufAsyncRxStructure))
        {
            error_setError(kAppIfModuleAsync, kAppIfAsyncBufferSizeMismatch);
        }
        else
        {
            // Get pointer to current instance
            pInstance = (tAsyncInstance) pUserArg_p;

            // Increment transmit timer cycle count
            timeout_incrementCounter(pInstance->txBuffParam_m.pTimeoutInst_m);

            // Convert to status buffer structure
            pAsyncRxBuff = (tTbufAsyncRxStructure*) pBuffer_p;

            // Check size of buffer
            if(bufSize_p == sizeof(tTbufAsyncRxStructure))
            {
                // Acknowledge buffer before access
                stream_ackBuffer(pInstance->rxBuffParam_m.idRxBuff_m);

                currSeqNr = AmiGetByteFromLe((UINT8 *)&pAsyncRxBuff->seqNr_m);

                // Check sequence number sanity
                if(currSeqNr == kSeqNrValueFirst ||
                   currSeqNr == kSeqNrValueSecond  )
                {
                    // Check sequence number against local copy
                    if(currSeqNr != pInstance->rxBuffParam_m.currRxSeqNr_m)
                    {
                        // Sequence number changed -> Frame available
                        pInstance->rxBuffParam_m.fRxFrameIncoming_m = TRUE;

                        // Increment local receive sequence number
                        pInstance->rxBuffParam_m.currRxSeqNr_m = currSeqNr;
                    }
                }

                fReturn = TRUE;
            }
            else
            {
                error_setError(kAppIfModuleAsync, kAppIfAsyncBufferSizeMismatch);
            }
        }
    }

    return fReturn;
}

//------------------------------------------------------------------------------
/**
\brief    Change local sequence number

\param[inout] pSeqNr_p        Changed sequence number

\ingroup module_async
*/
//------------------------------------------------------------------------------
static void async_changeLocalSeqNr(tSeqNrValue* pSeqNr_p)
{
    if(*pSeqNr_p == kSeqNrValueFirst)
    {
        *pSeqNr_p = kSeqNrValueSecond;
    }
    else
    {
        *pSeqNr_p = kSeqNrValueFirst;
    }
}

//------------------------------------------------------------------------------
/**
\brief    Check if channel is ready for transmission

\param[in]  pInstance_p             Pointer to the local instance

\return tChanStatus
\retval tChanStatusFree       Channel is free for transmission
\retval tChanStatusBusy       Channel is currently transmitting

\ingroup module_async
*/
//------------------------------------------------------------------------------
static tAsyncChanStatus async_checkChannelStatus(tAsyncInstance pInstance_p)
{
    tAsyncChanStatus chanStatus = kChanStatusInvalid;
    tSeqNrValue  seqNr = kSeqNrValueInvalid;

    // Get status of transmit channel
    status_getAsyncTxChanFlag(pInstance_p->chanId_m, &seqNr);

    // Check if old transmission is already finished!
    if(seqNr != pInstance_p->txBuffParam_m.currTxSeqNr_m)
    {
        // Message in progress -> retry later!
        chanStatus = kChanStatusBusy;
    }
    else
    {
        chanStatus = kChanStatusFree;
    }

    return chanStatus;
}

// \}


