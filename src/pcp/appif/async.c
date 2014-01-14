/**
********************************************************************************
\file   async.c

\brief  Creates an asynchronous channel for receive and transmit

This module handles the asynchronous channels between the PCP and the application.
For each channel a receive and a transmit buffer needs to be determined.
It forwards the received data to the POWERLINK stack which transfers the data
to the target.

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

#include <appif/internal/async.h>
#include <appif/async.h>

#include <appif/status.h>

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

extern tEplKernel PUBLIC appif_asyncObdAccessCb(tObdCbParam MEM* pParam_p);

//============================================================================//
//            P R I V A T E   D E F I N I T I O N S                           //
//============================================================================//

//------------------------------------------------------------------------------
// const defines
//------------------------------------------------------------------------------

#define ASYNC_RECEIVE_FIFO_ELEM_COUNT       5      ///< Number of elements in the receive FIFO
#define ASYNC_RX_TIMEOUT_CYCLE_COUNT        400    ///< Number of cycles after a transmit has a timeout
#define ASYNC_ARP_TIMEOUT_CYCLE_COUNT       50     ///< Number of cycles after an ARP request is retried
//------------------------------------------------------------------------------
// local types
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// local vars
//------------------------------------------------------------------------------

static struct  eAsyncInstance       asyncInstance_l[kNumAsyncInstCount];

static UINT8   nodeId_l = 0;            ///< Node id of the Cn
static UINT16  idxSsdoStub_l = 0;       ///< Index of the ssdo stub object
static UINT16  idxSsdoStubData_l = 0;   ///< Index of the ssdo stub data object

//------------------------------------------------------------------------------
// local function prototypes
//------------------------------------------------------------------------------
static tAppIfStatus async_processReceiveSm(tAsyncInstance pInstance_p);
static tAppIfStatus async_processTransmitSm(tAsyncInstance pInstance_p);
static tAsyncInstance async_getInstance(tAsyncChanNum  chanNum_p );
static tAppIfStatus async_getTargetNode(tAsyncInstance pInstance_p,
        UINT8* pTargNode_p, UINT16* pTargIdx_p, UINT8* pTargSubIdx_p);
static tAppIfStatus async_handleReceiveFrame(tAsyncInstance pInstance_p,
        UINT8* pRxBuffer_p, UINT32 buffSize_p);
static tAppIfStatus async_sendToDestTarget(tAsyncInstance pInstance_p,
        UINT8* pTargNode_p, UINT16* pTargIdx_p, UINT8* pTargSubIdx_p,
        UINT8* pMsgBuffer_p, UINT16* pBuffSize_p);
static tAppIfStatus async_postToBuffer(tAsyncInstance pInstance_p,
        UINT8* pMsgBuffer_p, UINT16 buffSize_p);
static tAppIfStatus async_grabFromBuffer(tAsyncInstance pInstance_p,
        UINT8** ppMsgBuffer_p, UINT16* pBuffSize_p);
static void async_changeLocalSeqNr(tSeqNrValue* pSeqNr_p);
static tAppIfStatus async_checkChannelStatus(tAsyncInstance pInstance_p);
static tAppIfStatus async_verifyTargetInfo(UINT8 targNode_p, UINT16 targIdx_p,
        UINT8 targSubIdx_p);

//============================================================================//
//            P U B L I C   F U N C T I O N S                                 //
//============================================================================//

//------------------------------------------------------------------------------
/**
\brief    Initialize the asynchronous channel module

\param[in] nodeId_p             Node id of the Cn
\param[in] idxSsdoStub_p        Object index of the ssdo stub
\param[in] idxSsdoStubData_p    Object index of the sddo stub address

\ingroup module_async
*/
//------------------------------------------------------------------------------
void async_init(UINT8 nodeId_p, UINT16 idxSsdoStub_p, UINT16 idxSsdoStubData_p)
{
    // Remember stub object addresses
    nodeId_l = nodeId_p;
    idxSsdoStub_l = idxSsdoStub_p;
    idxSsdoStubData_l = idxSsdoStubData_p;

    APPIF_MEMSET(&asyncInstance_l, 0 , sizeof(struct eAsyncInstance) * kNumAsyncInstCount);
}

//------------------------------------------------------------------------------
/**
\brief    Create a asynchronous channel instance

Instantiate a new asynchronous channel which provides send and receive
functionality for the upper and lower layers.

\param[in]  pInitParam_p          Asynchronous initialization structure

\return tAsyncInstance
\retval address              Pointer to the instance of the channel
\retval Null                 Unable to allocate instance

\ingroup module_async
*/
//------------------------------------------------------------------------------
tAsyncInstance async_create(tAsyncInitStruct* pInitParam_p)
{
    tAsyncInstance     pInstance = NULL;
    tTbufInitStruct    tbufInitParam;

#if _DEBUG
    if(pInitParam_p->tbufRxSize_m != sizeof(tTbufAsyncRxStructure))
    {
        goto Exit;
    }

    if(pInitParam_p->tbufTxSize_m != sizeof(tTbufAsyncTxStructure))
    {
        goto Exit;
    }

    if(pInitParam_p->chanId_m > (kNumAsyncInstCount - 1))
    {
        goto Exit;
    }
#endif

    // init the receive triple buffer module
    tbufInitParam.id_m = pInitParam_p->tbufRxId_m;
    tbufInitParam.pAckBase_m = pInitParam_p->pProdAckBase_m;
    tbufInitParam.pBase_m = (UINT8 *)pInitParam_p->pTbufRxBase_m;
    tbufInitParam.size_m = pInitParam_p->tbufRxSize_m;

    asyncInstance_l[pInitParam_p->chanId_m].prodRxParam_m.pTbufProdRxInst_m = tbuf_create(&tbufInitParam);
    if(asyncInstance_l[pInitParam_p->chanId_m].prodRxParam_m.pTbufProdRxInst_m == NULL)
    {
        goto Exit;
    }

    // init the transmit triple buffer module
    tbufInitParam.id_m = pInitParam_p->tbufTxId_m;
    tbufInitParam.pAckBase_m = pInitParam_p->pConsAckBase_m;
    tbufInitParam.pBase_m = (UINT8 *)pInitParam_p->pTbufTxBase_m;
    tbufInitParam.size_m = pInitParam_p->tbufTxSize_m;

    asyncInstance_l[pInitParam_p->chanId_m].consTxParam_m.pTbufConsTxInst_m = tbuf_create(&tbufInitParam);
    if(asyncInstance_l[pInitParam_p->chanId_m].consTxParam_m.pTbufConsTxInst_m == NULL)
    {
        goto Exit;
    }

    // Initialize the frame receive FIFO
    asyncInstance_l[pInitParam_p->chanId_m].prodRxParam_m.pRxFifoInst_m = fifo_create(
            sizeof(asyncInstance_l[pInitParam_p->chanId_m].prodRxParam_m.prodRecvBuff_m.ssdoStubDataBuff_m),
            ASYNC_RECEIVE_FIFO_ELEM_COUNT);
    if(asyncInstance_l[pInitParam_p->chanId_m].prodRxParam_m.pRxFifoInst_m == NULL)
    {
        goto Exit;
    }

    // Create timeout module for local async receive channel
    asyncInstance_l[pInitParam_p->chanId_m].prodRxParam_m.pTimeoutInst_m = timeout_create(
            ASYNC_RX_TIMEOUT_CYCLE_COUNT);
    if(asyncInstance_l[pInitParam_p->chanId_m].prodRxParam_m.pTimeoutInst_m == NULL)
    {
        goto Exit;
    }

    // Create timeout module for ARP retry count
    asyncInstance_l[pInitParam_p->chanId_m].consTxParam_m.pArpTimeoutInst_m = timeout_create(
            ASYNC_ARP_TIMEOUT_CYCLE_COUNT);
    if(asyncInstance_l[pInitParam_p->chanId_m].consTxParam_m.pArpTimeoutInst_m == NULL)
    {
        goto Exit;
    }

    // Remember channel id
    asyncInstance_l[pInitParam_p->chanId_m].instId_m = pInitParam_p->chanId_m;

    // Set initial transmit state
    asyncInstance_l[pInitParam_p->chanId_m].consTxParam_m.consTxState_m = kConsTxStateWaitForFrame;

    // Set initial receive state
    asyncInstance_l[pInitParam_p->chanId_m].prodRxParam_m.prodRxState_m = kProdRxStateWaitForFrame;

    // Set sequence number init value
    asyncInstance_l[pInitParam_p->chanId_m].prodRxParam_m.currProdSeq_m = kSeqNrValueFirst;

    // Set valid instance id
    pInstance = &asyncInstance_l[pInitParam_p->chanId_m];

Exit:
    return pInstance;
}

//------------------------------------------------------------------------------
/**
\brief    Destroy a asynchronous channel

Destroy a asynchronous channel instance which provides basic send and receive
functions for the channel.

\param[in] pInstance_p           Pointer to the instance

\ingroup module_async
*/
//------------------------------------------------------------------------------
void async_destroy(tAsyncInstance pInstance_p)
{
    if(pInstance_p != NULL)
    {
        tbuf_destroy(pInstance_p->prodRxParam_m.pTbufProdRxInst_m);

        tbuf_destroy(pInstance_p->consTxParam_m.pTbufConsTxInst_m);

        // Destroy receive FIFO instance
        fifo_destroy(pInstance_p->prodRxParam_m.pRxFifoInst_m);

        // Destroy the timeout module for the async rx channel
        timeout_destroy(pInstance_p->prodRxParam_m.pTimeoutInst_m);

        // Destroy the timeout module for the arp retry counter
        timeout_destroy(pInstance_p->consTxParam_m.pArpTimeoutInst_m);

    }

}

//------------------------------------------------------------------------------
/**
\brief    Process incoming SDO access

Background task of the incoming SDO access. Read the data from the triple buffer
and forward it to the POWERLINK stack.

\param[in] pInstance_p           Pointer to the instance

\return tAppIfStatus
\retval kAppIfSuccessful                    On success
\retval kAppIfAsyncProcessingFailed         Unable to process SDO access
\retval kAppIfConfChanWriteToObDictFailed   Unable to forward object to stack

\ingroup module_async
*/
//------------------------------------------------------------------------------
tAppIfStatus async_process(tAsyncInstance pInstance_p)
{
    tAppIfStatus ret = kAppIfSuccessful;
    tAppIfStatus chanRet = kAppIfSuccessful;
    tTimerStatus   timerState;

    if(pInstance_p == NULL)
    {
        ret = kAppIfAsyncProcessingFailed;
        goto Exit;
    }

    // Process outgoing frames
    ret = async_processReceiveSm(pInstance_p);
    if(ret != kAppIfSuccessful)
    {
        goto Exit;
    }

    // Process incoming frames
    ret = async_processTransmitSm(pInstance_p);
    if(ret != kAppIfSuccessful)
    {
        goto Exit;
    }

    // Check if timeout counter is expired
    timerState = timeout_checkExpire(pInstance_p->prodRxParam_m.pTimeoutInst_m);
    if(timerState == kTimerStateExpired)
    {
        // Timeout occurred -> Increment local sequence number!
        async_changeLocalSeqNr(&pInstance_p->prodRxParam_m.currProdSeq_m);

        ret = kAppIfSuccessful;
        goto Exit;
    }

    // Check if message is acknowledged -> Then reset timer!
    timerState = timeout_isRunning(pInstance_p->prodRxParam_m.pTimeoutInst_m);
    if(timerState == kTimerStateRunning)
    {
        chanRet = async_checkChannelStatus(pInstance_p);
        if(chanRet == kAppIfSuccessful)
        {
            timeout_stopTimer(pInstance_p->prodRxParam_m.pTimeoutInst_m);
        }
    }

Exit:
    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Handle finished SDO transfer

\param[in] pInstance_p           Pointer to the instance

\return kAppIfSuccessful

\ingroup module_async
*/
//------------------------------------------------------------------------------
tAppIfStatus async_consTxTransferFinished(tAsyncInstance pInstance_p)
{
    tAppIfStatus ret = kAppIfSuccessful;

    // set async status register flag to current sequence flag
    status_setAsyncConsChanFlag(pInstance_p->instId_m, pInstance_p->consTxParam_m.currConsSeq_m);

    // Set state machine to wait for next frame
    pInstance_p->consTxParam_m.consTxState_m = kConsTxStateWaitForFrame;

    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Handle incoming asynchronous payload

Handle incoming data from the triple buffers by checking the sequence number.
If data has arrived forward set the channel blocking and forward it to the
background task.
(This function is called in interrupt context)

\param[in] pInstance_p           Pointer to the instance

\return tAppIfStatus
\retval kAppIfSuccessful                On success
\retval kAppIfAsyncInvalidParameter     On an invalid instance
\retval other error

\ingroup module_async
*/
//------------------------------------------------------------------------------
tAppIfStatus async_handleIncoming(tAsyncInstance pInstance_p)
{
    tAppIfStatus ret = kAppIfSuccessful;
    tSeqNrValue  currSeqNr = kSeqNrValueInvalid;

    if(pInstance_p == NULL)
    {
        ret = kAppIfAsyncInvalidParameter;
        goto Exit;
    }

    // Increment cycle count transmit timer
    timeout_incrementCounter(pInstance_p->prodRxParam_m.pTimeoutInst_m);

    // Increment cycle counter for ARP retry timer
    timeout_incrementCounter(pInstance_p->consTxParam_m.pArpTimeoutInst_m);

    if(pInstance_p->consTxParam_m.consTxState_m != kConsTxStateWaitForFrame)
    {
        // Object access is currently in progress -> do nothing here!
        goto Exit;
    }

    // Acknowledge consuming buffer
    ret = tbuf_setAck(pInstance_p->consTxParam_m.pTbufConsTxInst_m);
    if(ret != kAppIfSuccessful)
    {
        goto Exit;
    }

    // Get current sequence number
    ret = tbuf_readByte(pInstance_p->consTxParam_m.pTbufConsTxInst_m,
            TBUF_ASYNCTX_SEQNR_OFF, (UINT8 *)&currSeqNr);
    if(ret != kAppIfSuccessful)
    {
        goto Exit;
    }

    // Check sequence number sanity
    if(currSeqNr != kSeqNrValueFirst && currSeqNr != kSeqNrValueSecond)
    {
        goto Exit;
    }

    if(currSeqNr != pInstance_p->consTxParam_m.currConsSeq_m)
    {
        // Switch to state process frame
        pInstance_p->consTxParam_m.consTxState_m = kConsTxStateProcessFrame;

        // Increment local sequence number
        pInstance_p->consTxParam_m.currConsSeq_m = currSeqNr;
    }

Exit:
    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Async module object access occurred

In case of an access to the local obdict.h of objects which are asynchronous
channel objects the payload needs to be forwarded to the triple buffer

\param[in] pParam_p               Object access parameter

\return  tEplKernel
\retval  kEplSuccessful        On success

\ingroup module_async
*/
//------------------------------------------------------------------------------
tEplKernel PUBLIC appif_asyncObdAccessCb(tObdCbParam MEM* pParam_p)
{
    tAppIfStatus ret = kAppIfSuccessful;
    tEplKernel eplret = kEplSuccessful;
    tAsyncInstance  pInstance;

    if(pParam_p == NULL)
    {
        eplret = kEplApiInvalidParam;
        goto Exit;
    }

    // Check if callback is on the right object
    if(pParam_p->index != idxSsdoStubData_l)
    {
        eplret = kEplObdIndexNotExist;
        goto Exit;
    }

    // Check action
    if(pParam_p->obdEvent == kObdEvPostWrite)
    {
        // Find instance handle
        pInstance = async_getInstance(pParam_p->subIndex - 1);
        if(pInstance == NULL)
        {
            eplret = kEplObdSubindexNotExist;
            goto Exit;
        }

//        // No segmented transfer allowed!
//        if(pParam_p->m_SegmentSize != pParam_p->m_TransferSize)
//        {
//            eplret = kEplObdAccessViolation;
//            goto Exit;
//        }

        // Post frame to receive FIFO
        ret = fifo_insertElement(pInstance->prodRxParam_m.pRxFifoInst_m,
                (tFifoElement) pParam_p->pArg,
                pParam_p->objSize);
        if(ret != kAppIfSuccessful)
        {
            eplret = kEplObdAccessViolation;
            goto Exit;
        }
        else
        {
            // Start timeout timer for this frame!
            timeout_startTimer(pInstance->prodRxParam_m.pTimeoutInst_m);
        }
    }


Exit:
    return eplret;
}

//============================================================================//
//            P R I V A T E   F U N C T I O N S                               //
//============================================================================//
/// \name Private Functions
/// \{

//------------------------------------------------------------------------------
/**
\brief    Process the frame receive state machine

Implements the asynchronous receive state machine. Reads the frame from the
FIFO and forwards it to the triple buffer.

\param[in] pInstance_p               Pointer to the local instance

\return  tAppIfStatus
\retval  kAppIfSuccessful            On success
\retval  kAppIfAsyncInvalidState     Invalid state machine state

\ingroup module_async
*/
//------------------------------------------------------------------------------
static tAppIfStatus async_processReceiveSm(tAsyncInstance pInstance_p)
{
    tAppIfStatus ret = kAppIfSuccessful;

    switch(pInstance_p->prodRxParam_m.prodRxState_m)
    {
        case kProdRxStateWaitForFrame:
        {
            // Get frame from receive FIFO
            ret = fifo_getElement(pInstance_p->prodRxParam_m.pRxFifoInst_m,
                    (tFifoElement)&pInstance_p->prodRxParam_m.prodRecvBuff_m.ssdoStubDataBuff_m,
                    &pInstance_p->prodRxParam_m.prodRecvBuff_m.ssdoStubSize_m);
            switch(ret)
            {
                case kAppIfFifoEmpty:
                {
                    // Nothing to do -> Check again later!
                    ret = kAppIfSuccessful;
                    goto Exit;
                }
                case kAppIfSuccessful:
                {
                    ret = async_handleReceiveFrame(pInstance_p,
                            pInstance_p->prodRxParam_m.prodRecvBuff_m.ssdoStubDataBuff_m,
                            pInstance_p->prodRxParam_m.prodRecvBuff_m.ssdoStubSize_m);
                    if(ret == kAppIfSuccessful)
                    {
                        // Frame posted successfully -> Get next frame!
                        pInstance_p->prodRxParam_m.prodRxState_m = kProdRxStateWaitForFrame;
                    }
                    else if(kAppIfAsyncNoFreeBuffer)
                    {
                        // No buffer is not a problem -> Retry until buffer gets ACK from AP!
                        ret = kAppIfSuccessful;
                        pInstance_p->prodRxParam_m.prodRxState_m = kProdRxStateRepostFrame;
                    }

                    break;
                }
                default:
                {
                    // Internal FIFO error occurred
                    goto Exit;
                }
            }
            break;
        }
        case kProdRxStateRepostFrame:
        {
            // Try to post frame to local triple buffer
            ret = async_handleReceiveFrame(pInstance_p,
                    pInstance_p->prodRxParam_m.prodRecvBuff_m.ssdoStubDataBuff_m,
                    pInstance_p->prodRxParam_m.prodRecvBuff_m.ssdoStubSize_m);
            if(ret == kAppIfSuccessful)
            {
                // Frame posted successfully -> Activate blocking mechanism
                pInstance_p->prodRxParam_m.prodRxState_m = kProdRxStateWaitForFrame;
            }
            else if(kAppIfAsyncNoFreeBuffer)
            {
                // No buffer is not a problem -> Retry until buffer gets ACK from AP!
                ret = kAppIfSuccessful;
            }

            break;
        }
        default:
        {
            ret = kAppIfAsyncInvalidState;
            break;
        }
    }

Exit:
    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Process the frame transmit state machine

Implements the asynchronous transmit state machine. Reads the frame from the
triple buffer and forwards it to the target node.

\param[in] pInstance_p               Pointer to the local instance

\return  tAppIfStatus
\retval  kAppIfSuccessful            On success
\retval  kAppIfAsyncInvalidState     Invalid state machine state

\ingroup module_async
*/
//------------------------------------------------------------------------------
static tAppIfStatus async_processTransmitSm(tAsyncInstance pInstance_p)
{
    tAppIfStatus ret = kAppIfSuccessful;
    tTimerStatus timerState;
    UINT8        targNode;
    UINT16       targIdx;
    UINT8        targSubIdx;
    UINT16       paylSize;

    // Process asynchronous channel
    switch(pInstance_p->consTxParam_m.consTxState_m)
    {
        case kConsTxStateWaitForFrame:
        {
            // Wait for new frame -> do nothing here!
            break;
        }
        case kConsTxStateProcessFrame:
        {
            // Incoming element -> Forward to other node!
            ret = async_grabFromBuffer(pInstance_p, &pInstance_p->consTxParam_m.pConsTxPayl_m,
                    &paylSize);
            if(ret != kAppIfSuccessful)
            {
                goto Exit;
            }

            // Get target node for incoming message!
            ret = async_getTargetNode(pInstance_p, &targNode, &targIdx, &targSubIdx);
            if(ret != kAppIfSuccessful)
            {
                goto Exit;
            }

            // Verify target node
            ret = async_verifyTargetInfo(targNode, targIdx, targSubIdx);
            if(ret != kAppIfSuccessful)
            {
                // TODO Signal error back to application
                ret = kAppIfSuccessful;
                goto Exit;
            }

            // Forward object access to target node
            ret = async_sendToDestTarget(pInstance_p,
                    &targNode, &targIdx, &targSubIdx,
                    pInstance_p->consTxParam_m.pConsTxPayl_m, &paylSize);
            if(ret != kAppIfSuccessful)
            {
                goto Exit;
            }

            break;
        }
        case kConsTxStateWaitForTxFinished:
        {
            // Wait until transfer is finished -> Do nothing here!
            break;
        }
        case kConsTxStateWaitForNextArpRetry:
        {
            // Check if the timer is expired
            timerState = timeout_checkExpire(pInstance_p->consTxParam_m.pArpTimeoutInst_m);
            if(timerState == kTimerStateExpired)
            {
                pInstance_p->consTxParam_m.consTxState_m = kConsTxStateProcessFrame;
                timeout_stopTimer(pInstance_p->consTxParam_m.pArpTimeoutInst_m);
            }

            break;
        }
        default:
        {
            ret = kAppIfAsyncInvalidState;
            break;
        }
    }

Exit:
    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Get instance handler from channel number


\param[in] chanNum_p             Channel number of the instance

\return  tAsyncInstance
\retval  Address        Pointer to the instance
\retval  NULL           On invalid channel number

\ingroup module_async
*/
//------------------------------------------------------------------------------
static tAsyncInstance async_getInstance(tAsyncChanNum  chanNum_p )
{
    tAsyncInstance  pInstance = NULL;

    if(chanNum_p >= kNumAsyncInstCount )
    {
        goto Exit;
    }

    // Set pointer to instance
    pInstance = &asyncInstance_l[chanNum_p];

Exit:
    return pInstance;
}

//------------------------------------------------------------------------------
/**
\brief    Read target information from local OD

Read nodeId, index and subindex from the local object dictionary.

\param[in]  pInstance_p             Pointer to the local instance
\param[out] pTargNode_p             The target node
\param[out] pTargIdx_p              The target object index
\param[out] pTargSubIdx_p           The target object subindex

\return  tAppIfStatus
\retval  kAppIfSuccessful              On success
\retval  kAppIfAsyncDestinationUnknown  Unable to find target in od

\ingroup module_async
*/
//------------------------------------------------------------------------------
static tAppIfStatus async_getTargetNode(tAsyncInstance pInstance_p,
        UINT8* pTargNode_p, UINT16* pTargIdx_p, UINT8* pTargSubIdx_p)
{
    tEplKernel eplret = kEplSuccessful;
    tAppIfStatus ret = kAppIfSuccessful;
    UINT32   accTargNode;
    UINT32   accTargSize = sizeof(accTargNode);

    // Read SSDO-Stub object to get target node, idx and subindex
    eplret = oplk_readLocalObject(idxSsdoStub_l, pInstance_p->instId_m + 1,
            &accTargNode, &accTargSize);
    if(eplret != kEplSuccessful)
    {
        ret = kAppIfAsyncDestinationUnknown;
        goto Exit;
    }

    // Acquire target node parameters
    *pTargIdx_p = (UINT16)(accTargNode & 0xFFFF);
    *pTargSubIdx_p = (UINT8)((accTargNode >> 16) & 0xFF);
    *pTargNode_p = (UINT8)((accTargNode >> 24) & 0xFF);

Exit:
    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Forward receive frame to buffer

\param[in] pInstance_p             Pointer to the local instance
\param[in] pRxBuffer_p             Pointer to the receive buffer
\param[in] buffSize_p              Size of the receive buffer

\return  tAppIfStatus
\retval  kAppIfSuccessful              On success
\retval  kAppIfTbuffWriteError         Unable to write data to buffer

\ingroup module_async
*/
//------------------------------------------------------------------------------
static tAppIfStatus async_handleReceiveFrame(tAsyncInstance pInstance_p,
        UINT8* pRxBuffer_p, UINT32 buffSize_p)
{
    tAppIfStatus ret = kAppIfSuccessful;

    // Check if channel is ready for transmission
    ret = async_checkChannelStatus(pInstance_p);
    if(ret == kAppIfAsyncChannelBusy)
    {
        ret = kAppIfSuccessful;
        goto Exit;
    }

    // Forward payload to buffer
    ret = async_postToBuffer(pInstance_p, pRxBuffer_p, (UINT16)buffSize_p);
    if(ret != kAppIfSuccessful)
    {
        goto Exit;
    }

    // Increment local sequence number
    async_changeLocalSeqNr(&pInstance_p->prodRxParam_m.currProdSeq_m);

    // Set sequence number
    ret = tbuf_writeByte(pInstance_p->prodRxParam_m.pTbufProdRxInst_m,
            TBUF_ASYNCRX_SEQNR_OFF, pInstance_p->prodRxParam_m.currProdSeq_m);
    if(ret != kAppIfSuccessful)
    {
        goto Exit;
    }

    // Acknowledge asynchronous producing receive triple buffer
    ret = tbuf_setAck(pInstance_p->prodRxParam_m.pTbufProdRxInst_m);
    if(ret != kAppIfSuccessful)
    {
        goto Exit;
    }

Exit:
    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Forward object access to target

This function calls oplk_writeObject which transfers the object access to
the destination target.

\param[in] pInstance_p             Pointer to the local instance
\param[in] pTargNode_p             The target node
\param[in] pTargIdx_p              The target object index
\param[in] pTargSubIdx_p           The target object subindex
\param[in] pMsgBuffer_p            Pointer to the message to transfer
\param[in] pBuffSize_p             Size of the message to transfer

\return  tAppIfStatus
\retval  kAppIfSuccessful                   On success
\retval  kAppIfConfChanWriteToObDictFailed  Unable to send data to target

\ingroup module_async
*/
//------------------------------------------------------------------------------
static tAppIfStatus async_sendToDestTarget(tAsyncInstance pInstance_p,
        UINT8* pTargNode_p, UINT16* pTargIdx_p, UINT8* pTargSubIdx_p,
        UINT8* pMsgBuffer_p, UINT16* pBuffSize_p)
{
    tAppIfStatus ret = kAppIfSuccessful;
    tEplKernel   eplret = kEplSuccessful;

    // Write incoming message to target node
    eplret = oplk_writeObject(&pInstance_p->consTxParam_m.sdoComConHdl_m,
            *pTargNode_p, *pTargIdx_p, *pTargSubIdx_p, // Target node, index and subindex
            pMsgBuffer_p,                              // Object data to transfer
            *pBuffSize_p,                              // Object size to transfer
            kEplSdoTypeUdp,                            // Type of SDO carrier (Always use UDP!)
            (void *)pInstance_p);                      // User argument is the instance pointer
    switch(eplret)
    {
        case kEplApiTaskDeferred:
        {
            pInstance_p->consTxParam_m.consTxState_m = kConsTxStateWaitForTxFinished;
            break;
        }
        case kEplSuccessful:
        {
            // Transfer is finished -> Free channel!
            ret = async_consTxTransferFinished(pInstance_p);
            break;
        }
        case kEplSdoUdpArpInProgress:
        {
            // ARP table is still not updated -> Retry to transmit the frame later!
            timeout_startTimer(pInstance_p->consTxParam_m.pArpTimeoutInst_m);

            pInstance_p->consTxParam_m.consTxState_m = kConsTxStateWaitForNextArpRetry;

            break;
        }
        default:
        {
            // Other error happend
            ret = kAppIfAsyncWriteToObDictFailed;
            break;
        }
    }

    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Post received frame to buffer

\param[in] pInstance_p             Pointer to the local instance
\param[in] pMsgBuffer_p            Pointer to the payload
\param[in] buffSize_p              Size of the payload

\return  tAppIfStatus
\retval  kAppIfSuccessful       On success

\ingroup module_async
*/
//------------------------------------------------------------------------------
static tAppIfStatus async_postToBuffer(tAsyncInstance pInstance_p,
        UINT8* pMsgBuffer_p, UINT16 buffSize_p)
{
    tAppIfStatus ret = kAppIfSuccessful;

    // Write payload field
    ret = tbuf_writeWord(pInstance_p->prodRxParam_m.pTbufProdRxInst_m,
            TBUF_ASYNCRX_PAYLSIZE_OFF, buffSize_p);
    if(ret != kAppIfSuccessful)
    {
        goto Exit;
    }

    // Post frame to buffer
    ret = tbuf_writeStream(pInstance_p->prodRxParam_m.pTbufProdRxInst_m,
            TBUF_ASYNCRX_SSDO_STUB_DATA_DOM_OFF, pMsgBuffer_p, buffSize_p);
    if(ret != kAppIfSuccessful)
    {
        goto Exit;
    }

Exit:
    return ret;

}

//------------------------------------------------------------------------------
/**
\brief    Read frame from the buffer

\param[in]  pInstance_p             Pointer to the local instance
\param[out] ppMsgBuffer_p           Pointer to the pointer of the payload
\param[out] pBuffSize_p             Size of the payload

\return  tEplKernel
\retval  kEplSuccessful                 On success
\retval  kAppIfAsyncTxConsSizeInvalid   Size of the payload too high

\ingroup module_async
*/
//------------------------------------------------------------------------------
static tAppIfStatus async_grabFromBuffer(tAsyncInstance pInstance_p,
        UINT8** ppMsgBuffer_p, UINT16* pBuffSize_p)
{
    tAppIfStatus ret = kAppIfSuccessful;
    UINT16      paylSize;

    ret = tbuf_readWord(pInstance_p->consTxParam_m.pTbufConsTxInst_m,
            TBUF_ASYNCTX_PAYLSIZE_OFF, &paylSize);
    if(ret != kAppIfSuccessful)
    {
        goto Exit;
    }

    // Get data pointer to local buffer
    ret = tbuf_getDataPtr(pInstance_p->consTxParam_m.pTbufConsTxInst_m,
            TBUF_ASYNCTX_TSSDO_TRANSMIT_DATA_OFF, ppMsgBuffer_p);
    if(ret != kAppIfSuccessful)
    {
        goto Exit;
    }

    // Return size of the payload
    *pBuffSize_p = paylSize;

Exit:
    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Change local sequence number

\param[out] pSeqNr_p        Sequence number to change

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
    status_getAsyncProdChanFlag(pInstance_p->instId_m, &seqNr);

    // Check if old transmission is already finished!
    if(seqNr != pInstance_p->prodRxParam_m.currProdSeq_m)
    {
        // Message in progress -> retry later!
        ret = kAppIfAsyncChannelBusy;
    }

    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Check target node information

\param[in]  targNode_p             Node id of the target
\param[in]  targIdx_p              Object index of the target
\param[in]  targSubIdx_p           Object subindex of the target

\return tAppIfStatus
\retval kAppIfSuccessful              Target information valid
\retval kAppIfAsyncInvalidTargetInfo  Invalid target information

\ingroup module_async
*/
//------------------------------------------------------------------------------
static tAppIfStatus async_verifyTargetInfo(UINT8 targNode_p, UINT16 targIdx_p,
        UINT8 targSubIdx_p)
{
    tAppIfStatus ret = kAppIfSuccessful;

    if(targNode_p == 0 || targNode_p == nodeId_l)
    {
        ret = kAppIfAsyncInvalidTargetInfo;
        goto Exit;
    }

    if(targIdx_p == 0 || targSubIdx_p == 0)
    {
        ret = kAppIfAsyncInvalidTargetInfo;
        goto Exit;
    }

Exit:
    return ret;
}

/// \}


