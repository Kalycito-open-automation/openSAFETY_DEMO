/**
********************************************************************************
\file   appif-async.c

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

#include "appif-asyncinst.h"
#include "appif-asyncint.h"

#include "appif-statusint.h"

#include "appif-streamint.h"

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

//------------------------------------------------------------------------------
// local vars
//------------------------------------------------------------------------------

static struct eAsyncInstance          asyncInstance_l[kNumAsyncInstCount];

//------------------------------------------------------------------------------
// local function prototypes
//------------------------------------------------------------------------------
static tAppIfStatus async_initRcvBuffer(tAsyncChanNum chanId_p,
        tTbufNumLayout rxBuffId_p);
static tAppIfStatus async_initTransBuffer(tAsyncChanNum chanId_p,
        tTbufNumLayout txBuffId_p);
static tAppIfStatus async_handleRxFrame(tAsyncInstance pInstance_p);
static tAppIfStatus async_handleTxFrame(tAsyncInstance pInstance_p);
static tAppIfStatus async_receiveFrame(UINT8* pBuffer_p, UINT16 bufSize_p,
        void* pUserArg_p);
static void async_changeLocalSeqNr(tSeqNrValue* pSeqNr_p);
static tAppIfStatus async_checkChannelStatus(tAsyncInstance pInstance_p);

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
    tAppIfStatus  ret = kAppIfSuccessful;
    tAsyncInstance pInstance = NULL;

#ifdef _DEBUG
    if(chanId_p >= kNumAsyncInstCount ||
       pInitParam_p == NULL            )
    {
        ret = kAppIfAsyncInitError;
    }
#endif

    if(ret == kAppIfSuccessful)
    {
        // Get asynchronous buffer parameters from stream module
        ret = async_initRcvBuffer(chanId_p, pInitParam_p->buffIdRx_m);
        if(ret == kAppIfSuccessful)
        {
            ret = async_initTransBuffer(chanId_p, pInitParam_p->buffIdTx_m);
            if(ret == kAppIfSuccessful)
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

\return tAppIfStatus
\retval kAppIfSuccessful              On success
\retval kAppIfAsyncTxConsSizeInvalid  Size of the payload to post is invalid
\retval kAppIfAsyncSendError          Invalid input parameters
\retval kAppIfAsyncNoFreeBuffer       Channel is currently busy

\ingroup module_async
*/
//------------------------------------------------------------------------------
tAppIfStatus async_postPayload(tAsyncInstance pInstance_p, UINT8* pPayload_p,
        UINT16 paylSize_p)
{
    tAppIfStatus ret = kAppIfSuccessful;

    if(pInstance_p != NULL &&
       pPayload_p != NULL    )
    {
        // Check if payload fits inside the buffer
        if(paylSize_p > sizeof(pInstance_p->txBuffParam_m.asyncTxBuffer_m.pAsyncTxPayl_m->tssdoTransmitData_m) ||
           paylSize_p == 0                             )
        {
            ret = kAppIfAsyncTxConsSizeInvalid;
        }
    }
    else
    {
        ret = kAppIfAsyncSendError;
    }

    if(ret == kAppIfSuccessful)
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
        }
        else
        {
            ret = kAppIfAsyncNoFreeBuffer;
        }
    }

    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Process the asynchronous module

\param[in]  pInstance_p     Asynchronous module instance

\return tAppIfStatus
\retval kAppIfSuccessful        On success
\retval Other                   Error while processing asynchronous frames

\ingroup module_async
*/
//------------------------------------------------------------------------------
tAppIfStatus async_process(tAsyncInstance pInstance_p)
{
    tAppIfStatus ret = kAppIfSuccessful;

    // Process incoming frames
    ret = async_handleRxFrame(pInstance_p);
    if(ret ==   kAppIfSuccessful)
    {
        // Process transmit frames
        ret = async_handleTxFrame(pInstance_p);
    }

    return ret;
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

\return tAppIfStatus
\retval kAppIfSuccessful               On success
\retval kAppIfStreamInvalidBuffer      Invalid buffer! Can't register
\retval kAppIfAsyncBufferSizeMismatch  Size of the buffer is invalid

\ingroup module_async
*/
//------------------------------------------------------------------------------
static tAppIfStatus async_initRcvBuffer(tAsyncChanNum chanId_p,
        tTbufNumLayout rxBuffId_p)
{
    tAppIfStatus ret = kAppIfSuccessful;
    tBuffDescriptor* pDescAsyncRcv;

    ret = stream_getBufferParam(rxBuffId_p, &pDescAsyncRcv);
    if(ret == kAppIfSuccessful)
    {
        if(pDescAsyncRcv->buffSize_m == sizeof(tTbufAsyncRxStructure))
        {
            // Remember buffer address for later usage
            asyncInstance_l[chanId_p].rxBuffParam_m.pAsyncRxBuffer_m =
                    (tTbufAsyncRxStructure *)pDescAsyncRcv->pBuffBase_m;

            // Register frame receive post action
            ret = stream_registerAction(kStreamActionPost, rxBuffId_p,
                    async_receiveFrame, (void *)&asyncInstance_l[chanId_p]);
        }
        else
        {
            ret = kAppIfAsyncBufferSizeMismatch;
        }
    }

    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Initialize the asynchronous transmit buffer

\param[in] chanId_p                 Id of the asynchronous channel
\param[in] txBuffId_p               Id of the asynchronous transmit buffer

\return tAppIfStatus
\retval kAppIfSuccessful               On success
\retval kAppIfAsyncInitError           Unable to initialize the timeout module
\retval kAppIfStreamInvalidBuffer      Invalid buffer! Can't register
\retval kAppIfAsyncBufferSizeMismatch  Size of the buffer is invalid

\ingroup module_async
*/
//------------------------------------------------------------------------------
static tAppIfStatus async_initTransBuffer(tAsyncChanNum chanId_p,
        tTbufNumLayout txBuffId_p)
{
    tAppIfStatus ret = kAppIfSuccessful;
    tBuffDescriptor* pDescAsyncTrans;

    ret = stream_getBufferParam(txBuffId_p, &pDescAsyncTrans);
    if(ret == kAppIfSuccessful)
    {
        if(pDescAsyncTrans->buffSize_m == sizeof(tTbufAsyncTxStructure))
        {
            // Remember buffer address for later usage
            asyncInstance_l[chanId_p].txBuffParam_m.asyncTxBuffer_m.pAsyncTxPayl_m =
                    (tTbufAsyncTxStructure *)pDescAsyncTrans->pBuffBase_m;

            // Initialize asynchronous transmit timeout instance
            asyncInstance_l[chanId_p].txBuffParam_m.pTimeoutInst_m = timeout_create(
                    ASYNC_TX_TIMEOUT_CYCLE_COUNT);
            if(asyncInstance_l[chanId_p].txBuffParam_m.pTimeoutInst_m == NULL)
            {
                ret = kAppIfAsyncInitError;
            }
        }
        else
        {
            ret = kAppIfAsyncBufferSizeMismatch;
        }
    }

    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Process asynchronous receive frames

\param[in] pInstance_p     Async module instance

\return tAppIfStatus
\retval kAppIfSuccessful        On success
\retval Other                   Other user error returned

\ingroup module_async
*/
//------------------------------------------------------------------------------
static tAppIfStatus async_handleRxFrame(tAsyncInstance pInstance_p)
{
    tAppIfStatus ret = kAppIfSuccessful;
    UINT8*  pRxBuffer;
    UINT16  rxBuffSize;

    if(pInstance_p->rxBuffParam_m.fRxFrameIncoming_m != FALSE)
    {
        // Frame incoming -> forward to the user
        pRxBuffer = &pInstance_p->rxBuffParam_m.pAsyncRxBuffer_m->ssdoStubDataDom_m[0];
        rxBuffSize = AmiGetWordFromLe((UINT8 *)&pInstance_p->rxBuffParam_m.pAsyncRxBuffer_m->paylSize_m);

        // Call asynchronous user handler
        ret = pInstance_p->rxBuffParam_m.pfnRxHandler_m(pRxBuffer, rxBuffSize);

        // Access finished -> Unblock channel by writing current sequence number to status field!
        status_setAsyncRxChanFlag(pInstance_p->chanId_m,
                pInstance_p->rxBuffParam_m.currRxSeqNr_m);

        pInstance_p->rxBuffParam_m.fRxFrameIncoming_m = FALSE;
    }

    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Process asynchronous transmit frames

\param[in]  pInstance_p     Asynchronous module instance

\return tAppIfStatus
\retval kAppIfSuccessful              On success

\ingroup module_async
*/
//------------------------------------------------------------------------------
static tAppIfStatus async_handleTxFrame(tAsyncInstance pInstance_p)
{
    tAppIfStatus ret = kAppIfSuccessful;

    if(pInstance_p->txBuffParam_m.asyncTxBuffer_m.isLocked_m != FALSE)
    {
        // Check if channel is ready for transmission
        ret = async_checkChannelStatus(pInstance_p);
        if(ret == kAppIfSuccessful)
        {
            // Ongoing message is acknowledged
            pInstance_p->txBuffParam_m.asyncTxBuffer_m.isLocked_m = FALSE;

            // Increment local sequence number
            async_changeLocalSeqNr(&pInstance_p->txBuffParam_m.currTxSeqNr_m);

            // When timer is running and ACK occurred -> Stop timer instance!
            if(timeout_isRunning(pInstance_p->txBuffParam_m.pTimeoutInst_m))
            {
                timeout_stopTimer(pInstance_p->txBuffParam_m.pTimeoutInst_m);
            }
        }
        else if(ret == kAppIfAsyncChannelBusy)
        {
            // Check if timeout counter is expired
            ret = timeout_checkExpire(pInstance_p->txBuffParam_m.pTimeoutInst_m);
            if(ret == kAppIfTimeoutOccured)
            {
                // Timeout occurred -> Increment local sequence number!
                async_changeLocalSeqNr(&pInstance_p->txBuffParam_m.currTxSeqNr_m);

                // Unlock channel anyway!
                pInstance_p->txBuffParam_m.asyncTxBuffer_m.isLocked_m = FALSE;
            }

            ret = kAppIfSuccessful;
        }
    }

    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Check for incoming frame from asynchronous channel

\param[in] pBuffer_p        Pointer to the base address of the buffer
\param[in] bufSize_p        Size of the buffer
\param[in] pUserArg_p       The user argument

\return tAppIfStatus
\retval kAppIfSuccessful          On success

\ingroup module_async
*/
//------------------------------------------------------------------------------
static tAppIfStatus async_receiveFrame(UINT8* pBuffer_p, UINT16 bufSize_p,
        void* pUserArg_p)
{
    tAppIfStatus ret = kAppIfSuccessful;
    tAsyncInstance pInstance;
    tTbufAsyncRxStructure*  pAsyncRxBuff;
    tSeqNrValue  currSeqNr = kSeqNrValueInvalid;

#ifdef _DEBUG
    if(pUserArg_p != NULL)
    {
        // Check size of buffer
        if(bufSize_p != sizeof(tTbufAsyncRxStructure))
        {
            ret = kAppIfAsyncBufferSizeMismatch;
        }
    }
    else
    {
        ret = kAppIfAsyncInvalidParameter;
    }
#endif

    if(ret == kAppIfSuccessful)
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
        }
        else
        {
            ret = kAppIfAsyncBufferSizeMismatch;
        }
    }

    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Change local sequence number

\param[out] pSeqNr_p        Changed sequence number

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

\return tAppIfStatus
\retval kAppIfSuccessful        Channel is free for transmission
\retval kAppIfAsyncChannelBusy  Channel is currently transmitting

\ingroup module_async
*/
//------------------------------------------------------------------------------
static tAppIfStatus async_checkChannelStatus(tAsyncInstance pInstance_p)
{
    tAppIfStatus ret = kAppIfSuccessful;
    tSeqNrValue  seqNr = kSeqNrValueInvalid;

    // Get status of transmit channel
    status_getAsyncTxChanFlag(pInstance_p->chanId_m, &seqNr);

    // Check if old transmission is already finished!
    if(seqNr != pInstance_p->txBuffParam_m.currTxSeqNr_m)
    {
        // Message in progress -> retry later!
        ret = kAppIfAsyncChannelBusy;
    }

    return ret;
}

// \}


