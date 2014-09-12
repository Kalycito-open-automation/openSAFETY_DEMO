/**
********************************************************************************
\file   tssdo.c

\brief  Creates a channel for transmitting SSDO frames

This module handles the SSDO transmit channels between the PCP and the
application.
It forwards transmit data from the application to the PCP and finally to the
network.

\ingroup module_ssdo
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

#include <appif/internal/tssdo.h>
#include <appif/tssdo.h>

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

//============================================================================//
//            P R I V A T E   D E F I N I T I O N S                           //
//============================================================================//

//------------------------------------------------------------------------------
// const defines
//------------------------------------------------------------------------------
#define SSDO_ARP_TIMEOUT_CYCLE_COUNT       50     ///< Number of cycles after an ARP request is retried

//------------------------------------------------------------------------------
// local types
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// local vars
//------------------------------------------------------------------------------

static struct  eTssdoInstance       tssdoInstance_l[kNumSsdoInstCount];

static UINT8   nodeId_l = 0;            ///< Node id of the CN
static UINT16  idxSsdoStub_l = 0;       ///< Index of the SSDO stub object
static UINT16  idxSsdoStubData_l = 0;   ///< Index of the SSDO stub data object

//------------------------------------------------------------------------------
// local function prototypes
//------------------------------------------------------------------------------
static tAppIfStatus processTransmitSm(tTssdoInstance pInstance_p);
static tAppIfStatus getTargetNode(tTssdoInstance pInstance_p,
        UINT8* pTargNode_p, UINT16* pTargIdx_p, UINT8* pTargSubIdx_p);
static tAppIfStatus sendToDestTarget(tTssdoInstance pInstance_p,
        UINT8* pTargNode_p, UINT16* pTargIdx_p, UINT8* pTargSubIdx_p,
        UINT8* pMsgBuffer_p, UINT16* pBuffSize_p);
static tAppIfStatus grabFromBuffer(tTssdoInstance pInstance_p,
        UINT8** ppMsgBuffer_p, UINT16* pBuffSize_p);
static tAppIfStatus verifyTargetInfo(UINT8 targNode_p, UINT16 targIdx_p,
        UINT8 targSubIdx_p);

//============================================================================//
//            P U B L I C   F U N C T I O N S                                 //
//============================================================================//

//------------------------------------------------------------------------------
/**
\brief    Initialize the SSDO channel module

\param[in] nodeId_p             Node id of the Cn
\param[in] idxSsdoStub_p        Object index of the SSDO stub
\param[in] idxSsdoStubData_p    Object index of the SSDO stub address

\ingroup module_ssdo
*/
//------------------------------------------------------------------------------
void tssdo_init(UINT8 nodeId_p, UINT16 idxSsdoStub_p, UINT16 idxSsdoStubData_p)
{
    // Remember stub object addresses
    nodeId_l = nodeId_p;
    idxSsdoStub_l = idxSsdoStub_p;
    idxSsdoStubData_l = idxSsdoStubData_p;

    APPIF_MEMSET(&tssdoInstance_l, 0 , sizeof(struct eTssdoInstance) * kNumSsdoInstCount);
}

//------------------------------------------------------------------------------
/**
\brief    Create a SSDO channel instance

Instantiate a new SSDO channel which provides send and receive
functionality for the upper and lower layers.

\param[in]  pInitParam_p          SSDO initialization structure

\retval Address              Pointer to the instance of the channel
\retval NULL                 Unable to allocate instance

\ingroup module_ssdo
*/
//------------------------------------------------------------------------------
tTssdoInstance tssdo_create(tTssdoInitStruct* pInitParam_p)
{
    tTssdoInstance     pInstance = NULL;
    tTbufInitStruct    tbufInitParam;

#if _DEBUG
    if(pInitParam_p->tbufTxSize_m != sizeof(tTbufSsdoTxStructure))
    {
        goto Exit;
    }

    if(pInitParam_p->chanId_m > (kNumSsdoInstCount - 1))
    {
        goto Exit;
    }
#endif

    // init the transmit triple buffer module
    tbufInitParam.id_m = pInitParam_p->tbufTxId_m;
    tbufInitParam.pAckBase_m = pInitParam_p->pConsAckBase_m;
    tbufInitParam.pBase_m = (UINT8 *)pInitParam_p->pTbufTxBase_m;
    tbufInitParam.size_m = pInitParam_p->tbufTxSize_m;

    tssdoInstance_l[pInitParam_p->chanId_m].pTbufConsTxInst_m = tbuf_create(&tbufInitParam);
    if(tssdoInstance_l[pInitParam_p->chanId_m].pTbufConsTxInst_m == NULL)
    {
        goto Exit;
    }

    // Create timeout module for ARP retry count
    tssdoInstance_l[pInitParam_p->chanId_m].pArpTimeoutInst_m = timeout_create(
            SSDO_ARP_TIMEOUT_CYCLE_COUNT);
    if(tssdoInstance_l[pInitParam_p->chanId_m].pArpTimeoutInst_m == NULL)
    {
        goto Exit;
    }

    // Remember channel id
    tssdoInstance_l[pInitParam_p->chanId_m].instId_m = pInitParam_p->chanId_m;

    // Set initial transmit state
    tssdoInstance_l[pInitParam_p->chanId_m].consTxState_m = kConsTxStateWaitForFrame;

    // Set valid instance id
    pInstance = &tssdoInstance_l[pInitParam_p->chanId_m];

Exit:
    return pInstance;
}

//------------------------------------------------------------------------------
/**
\brief    Destroy a transmit SSDO channel

\param[in] pInstance_p           Pointer to the instance

\ingroup module_ssdo
*/
//------------------------------------------------------------------------------
void tssdo_destroy(tTssdoInstance pInstance_p)
{
    if(pInstance_p != NULL)
    {
        tbuf_destroy(pInstance_p->pTbufConsTxInst_m);

        // Destroy the timeout module for the arp retry counter
        timeout_destroy(pInstance_p->pArpTimeoutInst_m);
    }
}

//------------------------------------------------------------------------------
/**
\brief    Process the SSDO transmit asynchronous task

This function processes the SSDO transmit state machine.

\param[in] pInstance_p           Pointer to the instance

\retval kAppIfSuccessful                    On success
\retval kAppIfSsdoProcessingFailed          Unable to process SDO access
\retval kAppIfConfChanWriteToObDictFailed   Unable to forward object to stack

\ingroup module_ssdo
*/
//------------------------------------------------------------------------------
tAppIfStatus tssdo_process(tTssdoInstance pInstance_p)
{
    tAppIfStatus ret = kAppIfSuccessful;

    if(pInstance_p == NULL)
    {
        ret = kAppIfSsdoProcessingFailed;
        goto Exit;
    }

    // Process incoming frames
    ret = processTransmitSm(pInstance_p);
    if(ret != kAppIfSuccessful)
    {
        goto Exit;
    }

Exit:
    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Handle finished SDO transfer

\param[in] pInstance_p          Pointer to the instance

\retval kAppIfSuccessful        Always successful

\ingroup module_ssdo
*/
//------------------------------------------------------------------------------
tAppIfStatus tssdo_consTxTransferFinished(tTssdoInstance pInstance_p)
{
    tAppIfStatus ret = kAppIfSuccessful;

    // set ssdo status register flag to current sequence flag
    status_setSsdoConsChanFlag(pInstance_p->instId_m, pInstance_p->currConsSeq_m);

    // Set state machine to wait for next frame
    pInstance_p->consTxState_m = kConsTxStateWaitForFrame;

    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Handle incoming ssdo payload

Handle incoming data from the triple buffers by checking the sequence number.
If data has arrived forward set the channel blocking and forward it to the
background task.
(This function is called in interrupt context)

\param[in] pInstance_p           Pointer to the instance

\retval kAppIfSuccessful               On success
\retval kAppIfSsdoInvalidParameter     On an invalid instance
\retval other error

\ingroup module_ssdo
*/
//------------------------------------------------------------------------------
tAppIfStatus tssdo_handleIncoming(tTssdoInstance pInstance_p)
{
    tAppIfStatus ret = kAppIfSuccessful;
    tSeqNrValue  currSeqNr = kSeqNrValueInvalid;

    if(pInstance_p == NULL)
    {
        ret = kAppIfSsdoInvalidParameter;
        goto Exit;
    }

    // Increment cycle counter for ARP retry timer
    timeout_incrementCounter(pInstance_p->pArpTimeoutInst_m);

    if(pInstance_p->consTxState_m != kConsTxStateWaitForFrame)
    {
        // Object access is currently in progress -> do nothing here!
        goto Exit;
    }

    // Acknowledge consuming buffer
    ret = tbuf_setAck(pInstance_p->pTbufConsTxInst_m);
    if(ret != kAppIfSuccessful)
    {
        goto Exit;
    }

    // Get current sequence number
    ret = tbuf_readByte(pInstance_p->pTbufConsTxInst_m,
            TBUF_SSDOTX_SEQNR_OFF, (UINT8 *)&currSeqNr);
    if(ret != kAppIfSuccessful)
    {
        goto Exit;
    }

    // Check sequence number sanity
    if(currSeqNr != kSeqNrValueFirst && currSeqNr != kSeqNrValueSecond)
    {
        goto Exit;
    }

    if(currSeqNr != pInstance_p->currConsSeq_m)
    {
        // Switch to state process frame
        pInstance_p->consTxState_m = kConsTxStateProcessFrame;

        // Increment local sequence number
        pInstance_p->currConsSeq_m = currSeqNr;
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
\brief    Process the frame transmit state machine

Implements the ssdo transmit state machine. Reads the frame from the
triple buffer and forwards it to the target node.

\param[in] pInstance_p               Pointer to the local instance

\retval  kAppIfSuccessful            On success
\retval  kAppIfSsdoInvalidState     Invalid state machine state

\ingroup module_ssdo
*/
//------------------------------------------------------------------------------
static tAppIfStatus processTransmitSm(tTssdoInstance pInstance_p)
{
    tAppIfStatus ret = kAppIfSuccessful;
    tTimerStatus timerState;
    UINT8        targNode;
    UINT16       targIdx;
    UINT8        targSubIdx;
    UINT16       paylSize;

    // Process ssdo channel
    switch(pInstance_p->consTxState_m)
    {
        case kConsTxStateWaitForFrame:
        {
            // Wait for new frame -> do nothing here!
            break;
        }
        case kConsTxStateProcessFrame:
        {
            // Incoming element -> Forward to other node!
            ret = grabFromBuffer(pInstance_p, &pInstance_p->pConsTxPayl_m,
                    &paylSize);
            if(ret != kAppIfSuccessful)
            {
                goto Exit;
            }

            // Get target node for incoming message!
            ret = getTargetNode(pInstance_p, &targNode, &targIdx, &targSubIdx);
            if(ret != kAppIfSuccessful)
            {
                goto Exit;
            }

            // Verify target node
            ret = verifyTargetInfo(targNode, targIdx, targSubIdx);
            if(ret != kAppIfSuccessful)
            {
                // TODO Signal error back to application
                ret = kAppIfSuccessful;
                goto Exit;
            }

            // Forward object access to target node
            ret = sendToDestTarget(pInstance_p,
                    &targNode, &targIdx, &targSubIdx,
                    pInstance_p->pConsTxPayl_m, &paylSize);
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
            timerState = timeout_checkExpire(pInstance_p->pArpTimeoutInst_m);
            if(timerState == kTimerStateExpired)
            {
                pInstance_p->consTxState_m = kConsTxStateProcessFrame;
                timeout_stopTimer(pInstance_p->pArpTimeoutInst_m);
            }

            break;
        }
        default:
        {
            ret = kAppIfSsdoInvalidState;
            break;
        }
    }

Exit:
    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Read target information from local OD

Read nodeId, index and subindex from the local object dictionary.

\param[in]  pInstance_p             Pointer to the local instance
\param[out] pTargNode_p             The target node
\param[out] pTargIdx_p              The target object index
\param[out] pTargSubIdx_p           The target object subindex

\retval  kAppIfSuccessful              On success
\retval  kAppIfSsdoDestinationUnknown  Unable to find target in od

\ingroup module_ssdo
*/
//------------------------------------------------------------------------------
static tAppIfStatus getTargetNode(tTssdoInstance pInstance_p,
                                  UINT8* pTargNode_p, UINT16* pTargIdx_p,
                                  UINT8* pTargSubIdx_p)
{
    tOplkError oplkret = kErrorOk;
    tAppIfStatus ret = kAppIfSuccessful;
    UINT32   accTargNode;
    UINT32   accTargSize = sizeof(accTargNode);

    // Read SSDO-Stub object to get target node, idx and subindex
    oplkret = oplk_readLocalObject(idxSsdoStub_l, pInstance_p->instId_m + 1,
            &accTargNode, &accTargSize);
    if(oplkret != kErrorOk)
    {
        ret = kAppIfSsdoDestinationUnknown;
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
\brief    Forward object access to target

This function calls oplk_writeObject which transfers the object access to
the destination target.

\param[in] pInstance_p             Pointer to the local instance
\param[in] pTargNode_p             The target node
\param[in] pTargIdx_p              The target object index
\param[in] pTargSubIdx_p           The target object subindex
\param[in] pMsgBuffer_p            Pointer to the message to transfer
\param[in] pBuffSize_p             Size of the message to transfer

\retval  kAppIfSuccessful                   On success
\retval  kAppIfConfChanWriteToObDictFailed  Unable to send data to target

\ingroup module_ssdo
*/
//------------------------------------------------------------------------------
static tAppIfStatus sendToDestTarget(tTssdoInstance pInstance_p,
                                     UINT8* pTargNode_p, UINT16* pTargIdx_p,
                                     UINT8* pTargSubIdx_p, UINT8* pMsgBuffer_p,
                                     UINT16* pBuffSize_p)
{
    tAppIfStatus ret = kAppIfSuccessful;
    tOplkError   oplkret = kErrorOk;

    // Write incoming message to target node
    oplkret = oplk_writeObject(&pInstance_p->sdoComConHdl_m,
            *pTargNode_p, *pTargIdx_p, *pTargSubIdx_p, // Target node, index and subindex
            pMsgBuffer_p,                              // Object data to transfer
            *pBuffSize_p,                              // Object size to transfer
            kSdoTypeUdp,                               // Type of SDO carrier (Always use UDP!)
            (void *)pInstance_p);                      // User argument is the instance pointer
    switch(oplkret)
    {
        case kErrorApiTaskDeferred:
        {
            pInstance_p->consTxState_m = kConsTxStateWaitForTxFinished;
            break;
        }
        case kErrorOk:
        {
            // Transfer is finished -> Free channel!
            ret = tssdo_consTxTransferFinished(pInstance_p);
            break;
        }
        case kErrorSdoUdpArpInProgress:
        {
            // ARP table is still not updated -> Retry to transmit the frame later!
            timeout_startTimer(pInstance_p->pArpTimeoutInst_m);

            pInstance_p->consTxState_m = kConsTxStateWaitForNextArpRetry;

            break;
        }
        case  kErrorSdoComHandleBusy:
        {
            // Handle is busy -> try to retransmit later!
            break;
        }
        default:
        {
            // Other error happend
            ret = kAppIfSsdoWriteToObDictFailed;
            break;
        }
    }

    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Read frame from the buffer

\param[in]  pInstance_p             Pointer to the local instance
\param[out] ppMsgBuffer_p           Pointer to the pointer of the payload
\param[out] pBuffSize_p             Size of the payload

\retval  kAppIfSuccessful              On success
\retval  kAppIfSsdoTxConsSizeInvalid   Size of the payload too high

\ingroup module_ssdo
*/
//------------------------------------------------------------------------------
static tAppIfStatus grabFromBuffer(tTssdoInstance pInstance_p,
                                   UINT8** ppMsgBuffer_p, UINT16* pBuffSize_p)
{
    tAppIfStatus ret = kAppIfSuccessful;
    UINT16      paylSize;

    ret = tbuf_readWord(pInstance_p->pTbufConsTxInst_m,
            TBUF_SSDOTX_PAYLSIZE_OFF, &paylSize);
    if(ret != kAppIfSuccessful)
    {
        goto Exit;
    }

    // Get data pointer to local buffer
    ret = tbuf_getDataPtr(pInstance_p->pTbufConsTxInst_m,
            TBUF_SSDOTX_TSSDO_TRANSMIT_DATA_OFF, ppMsgBuffer_p);
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
\brief    Check target node information

\param[in]  targNode_p             Node id of the target
\param[in]  targIdx_p              Object index of the target
\param[in]  targSubIdx_p           Object subindex of the target

\retval kAppIfSuccessful              Target information valid
\retval kAppIfSsdoInvalidTargetInfo   Invalid target information

\ingroup module_ssdo
*/
//------------------------------------------------------------------------------
static tAppIfStatus verifyTargetInfo(UINT8 targNode_p, UINT16 targIdx_p,
                                     UINT8 targSubIdx_p)
{
    tAppIfStatus ret = kAppIfSuccessful;

    if(targNode_p == 0 || targNode_p == nodeId_l)
    {
        ret = kAppIfSsdoInvalidTargetInfo;
        goto Exit;
    }

    if(targIdx_p == 0 || targSubIdx_p == 0)
    {
        ret = kAppIfSsdoInvalidTargetInfo;
        goto Exit;
    }

Exit:
    return ret;
}

/// \}


