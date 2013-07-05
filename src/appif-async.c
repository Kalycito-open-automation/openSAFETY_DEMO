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
* Copyright © 2013 BERNECKER + RAINER, AUSTRIA, 5142 EGGELSBERG, B&R STRASSE 1
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

#define ASYNC_TX_TIMEOUT_CYCLE_COUNT        400    ///< Number of cycles after a transmit has a timeout

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
\brief    Initialize the async module

\ingroup module_async
*/
//------------------------------------------------------------------------------
tAppIfStatus async_init(void)
{
    tAppIfStatus ret = kAppIfSuccessful;

    APPIF_MEMSET(&asyncInstance_l, 0 , sizeof(struct eAsyncInstance) * kNumAsyncInstCount);

    return ret;
}


//------------------------------------------------------------------------------
/**
\brief    Create a asynchronous channel instance

Instantiate a new asynchronous channel which provides send and receive
functionality for the upper and lower layers.

\param  chanId_m             Id of the asynchronous channel
\param  pInitParam_p         Asynchronous module initialization structure

\return tAsyncInstance
\retval Address              Pointer to the instance of the channel
\retval Null                 Unable to allocate instance

\ingroup module_async
*/
//------------------------------------------------------------------------------
tAsyncInstance async_create(tAsyncChanNum chanId_p, tAsyncInitParam* pInitParam_p)
{
    tAppIfStatus  ret;
    tAsyncInstance pInstance = NULL;
    tBuffParam buffParam;
    UINT8 idCurrTxBuffer = asyncInstance_l[chanId_p].txBuffParam_m.currTxBuffer_m;

    if(chanId_p >= kNumAsyncInstCount)
    {
        goto Exit;
    }

    // Register async rx buffer to stream module
    buffParam.buffId_m = pInitParam_p->buffIdRx_m;
    buffParam.pBuffBase_m = (UINT8 *)&asyncInstance_l[chanId_p].rxBuffParam_m.asyncRxBuffer_m;
    buffParam.buffSize_m = sizeof(tTbufAsyncRxStructure);

    ret = stream_registerBuffer(&buffParam);
    if(ret != kAppIfSuccessful)
    {
        pInstance = NULL;
        goto Exit;
    }

    // Register async tx buffer to stream module
    buffParam.buffId_m = pInitParam_p->buffIdTx_m;
    buffParam.pBuffBase_m = (UINT8 *)&asyncInstance_l[chanId_p].txBuffParam_m.ccTxBuffer_m[idCurrTxBuffer].asyncTxPayl_m;
    buffParam.buffSize_m = sizeof(tTbufAsyncTxStructure);

    ret = stream_registerBuffer(&buffParam);
    if(ret != kAppIfSuccessful)
    {
        pInstance = NULL;
        goto Exit;
    }

    // Initialize asynchronous transmit timeout instance
    asyncInstance_l[chanId_p].txBuffParam_m.pTimeoutInst_m = timeout_create(
            kTimeoutNumApAsyncTransmit,
            ASYNC_TX_TIMEOUT_CYCLE_COUNT);
    if(asyncInstance_l[chanId_p].txBuffParam_m.pTimeoutInst_m == NULL)
    {
        goto Exit;
    }

    // Register frame receive post action
    ret = stream_registerAction(kStreamActionPost, pInitParam_p->buffIdRx_m,
            async_receiveFrame, (void *)&asyncInstance_l[chanId_p]);
    if(ret != kAppIfSuccessful)
    {
        pInstance = NULL;
        goto Exit;
    }

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

Exit:
    return pInstance;
}

//------------------------------------------------------------------------------
/**
\brief    Destroy an asynchronous channel

\param  pInstance_p       The instance to destroy

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
\brief    Send a frame through the asynchronous channel

\param  pInstance_p     [IN] Async module instance
\param  pPayload_p      [IN] Pointer to the payload to send
\param  paylSize_p      [IN] Size of the payload to send

\ingroup module_async
*/
//------------------------------------------------------------------------------
tAppIfStatus async_sendFrame(tAsyncInstance pInstance_p, UINT8* pPayload_p,
        UINT16 paylSize_p)
{
    tAppIfStatus ret = kAppIfSuccessful;
    UINT8        fillBuffId;

    if(pInstance_p == NULL)
    {
        ret = kAppIfAsyncSendError;
        goto Exit;
    }

    if(paylSize_p > sizeof(pInstance_p->txBuffParam_m.ccTxBuffer_m[0].asyncTxPayl_m) ||
       paylSize_p == 0                             )
    {
        ret = kAppIfAsyncSendError;
        goto Exit;
    }

    // Get buffer id for filling buffer
    fillBuffId = pInstance_p->txBuffParam_m.currTxBuffer_m ^ 1;

    // Check if buffer is free for filling
    if(pInstance_p->txBuffParam_m.ccTxBuffer_m[fillBuffId].isLocked_m != FALSE)
    {
        ret = kAppIfAsyncNoFreeBuffer;
        goto Exit;
    }

    // Fill buffer
    APPIF_MEMCPY(pInstance_p->txBuffParam_m.ccTxBuffer_m[fillBuffId].asyncTxPayl_m.tssdoTransmitData_m,
            pPayload_p, paylSize_p);

    // Set transmit size
    AmiSetWordToLe((UINT8*)&pInstance_p->txBuffParam_m.ccTxBuffer_m[fillBuffId].asyncTxPayl_m.paylSize_m, paylSize_p);

    // Lock buffer for transmission
    pInstance_p->txBuffParam_m.ccTxBuffer_m[fillBuffId].isLocked_m = TRUE;

    // Enable transmit timer
    timeout_startTimer(pInstance_p->txBuffParam_m.pTimeoutInst_m);

Exit:
    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Process the asynchronous module

\param  pInstance_p     [IN] Async module instance

\return tAppIfStatus
\retval kAppIfSuccessful        On success
\retval Other error             Error while processing asynchronous frames

\ingroup module_async
*/
//------------------------------------------------------------------------------
tAppIfStatus async_process(tAsyncInstance pInstance_p)
{
    tAppIfStatus ret = kAppIfSuccessful;

    // Process incoming frames
    ret = async_handleRxFrame(pInstance_p);
    if(ret !=   kAppIfSuccessful)
    {
        goto Exit;
    }

    // Process transmit frames
    ret = async_handleTxFrame(pInstance_p);
    if(ret !=   kAppIfSuccessful)
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

//------------------------------------------------------------------------------
/**
\brief    Process asynchronous receive frames

\param  pInstance_p     [IN] Async module instance

\return tAppIfStatus
\retval kAppIfSuccessful        On success
\retval Other error             Other user error returned

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
        pRxBuffer = &pInstance_p->rxBuffParam_m.asyncRxBuffer_m.ssdoStubDataDom_m[0];
        rxBuffSize = AmiGetWordFromLe((UINT8 *)&pInstance_p->rxBuffParam_m.asyncRxBuffer_m.paylSize_m);

        // Call asynchronous user handler
        ret = pInstance_p->rxBuffParam_m.pfnRxHandler_m(pInstance_p, pRxBuffer, rxBuffSize);

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

\param  pInstance_p     [IN] Async module instance

\return tAppIfStatus
\retval kAppIfSuccessful              On success
\retval kAppIfStreamInvalidBuffer     Unable to update buffer base

\ingroup module_async
*/
//------------------------------------------------------------------------------
static tAppIfStatus async_handleTxFrame(tAsyncInstance pInstance_p)
{
    tAppIfStatus ret = kAppIfSuccessful;
    UINT8 currTxBuff;
    UINT8 nextTxBuff;

    // Check if timeout counter is expired
    ret = timeout_checkExpire(pInstance_p->txBuffParam_m.pTimeoutInst_m);
    if(ret == kAppIfTimeoutOccured)
    {
        // Timeout occurred -> Increment local sequence number!
        async_changeLocalSeqNr(&pInstance_p->txBuffParam_m.currTxSeqNr_m);
    }

    // Check if channel is ready for transmission
    ret = async_checkChannelStatus(pInstance_p);
    if(ret == kAppIfAsyncChannelBusy)
    {
        ret = kAppIfSuccessful;
        goto Exit;
    }

    // When timer is running and ACK occurred -> Stop timer instance!
    if(timeout_isRunning(pInstance_p->txBuffParam_m.pTimeoutInst_m))
    {
        timeout_stopTimer(pInstance_p->txBuffParam_m.pTimeoutInst_m);
    }

    // Select next transmit buffer
    currTxBuff = pInstance_p->txBuffParam_m.currTxBuffer_m;
    nextTxBuff = currTxBuff ^ 1;

    // Switch to next buffer if there is something new to transmit
    if(pInstance_p->txBuffParam_m.ccTxBuffer_m[nextTxBuff].isLocked_m != FALSE   )
    {
        // Unlock current buffer
        pInstance_p->txBuffParam_m.ccTxBuffer_m[currTxBuff].isLocked_m = FALSE;

        // Increment local sequence number
        async_changeLocalSeqNr(&pInstance_p->txBuffParam_m.currTxSeqNr_m);

        // Set sequence number in next tx buffer
        AmiSetByteToLe((UINT8*)&pInstance_p->txBuffParam_m.ccTxBuffer_m[nextTxBuff].asyncTxPayl_m.seqNr_m,
                pInstance_p->txBuffParam_m.currTxSeqNr_m);

        // Next buffer is filled and can be transmitted
        ret = stream_updateBufferBase(pInstance_p->txBuffParam_m.idTxBuff_m,
                (UINT8 *)&pInstance_p->txBuffParam_m.ccTxBuffer_m[nextTxBuff].asyncTxPayl_m);
        if(ret != kAppIfSuccessful)
        {
            goto Exit;
        }

        // Set next buffer to current buffer
        pInstance_p->txBuffParam_m.currTxBuffer_m = nextTxBuff;
    }

    // Acknowledge producing transmit buffer
    stream_ackBuffer(pInstance_p->txBuffParam_m.idTxBuff_m);

Exit:
    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Check for incoming frame from async channel

\param pBuffer_p        [IN] Pointer to the base address of the buffer
\param bufSize_p        [IN] Size of the buffer
\param pUserArg_p       [IN] The user argument

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
    if(pUserArg_p == NULL)
    {
        ret = kAppIfAsyncInvalidParameter;
        goto Exit;
    }

    // Check size of buffer
    if(bufSize_p != sizeof(tTbufAsyncRxStructure))
    {
        ret = kAppIfAsyncBufferSizeMismatch;
        goto Exit;
    }
#endif

    // Get pointer to current instance
    pInstance = (tAsyncInstance) pUserArg_p;

    // Increment transmit timer cycle count
    timeout_incrementCounter(pInstance->txBuffParam_m.pTimeoutInst_m);

    // Convert to status buffer structure
    pAsyncRxBuff = (tTbufAsyncRxStructure*) pBuffer_p;

    // Check size of buffer
    if(bufSize_p != sizeof(tTbufAsyncRxStructure))
    {
        ret = kAppIfAsyncBufferSizeMismatch;
        goto Exit;
    }

    // Acknowledge buffer before access
    stream_ackBuffer(pInstance->rxBuffParam_m.idRxBuff_m);

    currSeqNr = AmiGetByteFromLe((UINT8 *)&pAsyncRxBuff->seqNr_m);

    // Check sequence number sanity
    if(currSeqNr != kSeqNrValueFirst &&
       currSeqNr != kSeqNrValueSecond  )
    {
        goto Exit;
    }

    // Check sequence number
    if(currSeqNr != pInstance->rxBuffParam_m.currRxSeqNr_m)
    {
        // Sequence number changed -> Frame available
        pInstance->rxBuffParam_m.fRxFrameIncoming_m = TRUE;

        // Increment local receive sequence number
        pInstance->rxBuffParam_m.currRxSeqNr_m = currSeqNr;
    }

Exit:
    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Change local sequence number

\param pSeqNr_p        [OUT] Changed sequence number

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

\param  pInstance_p             [IN] Pointer to the local instance

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


