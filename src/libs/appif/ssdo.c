/**
********************************************************************************
\file   ssdo.c

\brief  Creates an SSDO channel for receive and transmit

This module handles the SSDO channels between the PCP and the application.
For each channel a receive and a transmit buffer needs to be determined.
It forwards the received data to the application.

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

#include <libappif/internal/ssdoinst.h>
#include <libappif/internal/ssdo.h>

#include <libappif/internal/status.h>
#include <libappif/internal/stream.h>

#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_SSDO)) != 0)

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
 * \brief Status of the SSDO channel
 */
typedef enum {
    kChanStatusInvalid   = 0x00,    ///< Invalid channel status
    kChanStatusBusy      = 0x01,    ///< Channel is currently busy
    kChanStatusFree      = 0x02,    ///< Channel is free for transmission
} tSsdoChanStatus;

//------------------------------------------------------------------------------
// local vars
//------------------------------------------------------------------------------

static struct eSsdoInstance          ssdoInstance_l[kNumSsdoInstCount];

//------------------------------------------------------------------------------
// local function prototypes
//------------------------------------------------------------------------------
static BOOL ssdo_initReceiveBuffer(tSsdoChanNum chanId_p,
        tTbufNumLayout rxBuffId_p);
static BOOL ssdo_initTransmitBuffer(tSsdoChanNum chanId_p,
        tTbufNumLayout txBuffId_p);
static BOOL ssdo_handleRxFrame(tSsdoInstance pInstance_p);
static void ssdo_handleTxFrame(tSsdoInstance pInstance_p);
static BOOL ssdo_receiveFrame(UINT8* pBuffer_p, UINT16 bufSize_p,
        void* pUserArg_p);
static void ssdo_changeLocalSeqNr(tSeqNrValue* pSeqNr_p);
static tSsdoChanStatus ssdo_checkChannelStatus(tSsdoInstance pInstance_p);

//============================================================================//
//            P U B L I C   F U N C T I O N S                                 //
//============================================================================//

//------------------------------------------------------------------------------
/**
\brief    Initialize the SSDO channel module

\ingroup module_ssdo
*/
//------------------------------------------------------------------------------
void ssdo_init(void)
{
    APPIF_MEMSET(&ssdoInstance_l, 0 , sizeof(struct eSsdoInstance) * kNumSsdoInstCount);
}


//------------------------------------------------------------------------------
/**
\brief    Create a SSDO channel instance

Instantiate a new SSDO channel which provides send and receive
functionality for the upper and lower layers.

\param[in]  chanId_p         Id of the SSDO channel
\param[in]  pInitParam_p     SSDO module initialization structure

\return tSsdoInstance
\retval Address              Pointer to the instance of the channel
\retval Null                 Unable to allocate instance

\ingroup module_ssdo
*/
//------------------------------------------------------------------------------
tSsdoInstance ssdo_create(tSsdoChanNum chanId_p, tSsdoInitParam* pInitParam_p)
{
    tSsdoInstance pInstance = NULL;

    if(chanId_p >= kNumSsdoInstCount ||
       pInitParam_p == NULL            )
    {
        error_setError(kAppIfModuleSsdo, kAppIfSsdoInitError);
    }
    else
    {
        if(pInitParam_p->buffIdRx_m >= kTbufCount              ||
           pInitParam_p->buffIdTx_m >= kTbufCount              ||
           pInitParam_p->buffIdRx_m == pInitParam_p->buffIdTx_m )
        {
            error_setError(kAppIfModuleSsdo, kAppIfSsdoInitError);
        }
        else
        {
            // Get SSDO buffer parameters from stream module
            if(ssdo_initReceiveBuffer(chanId_p, pInitParam_p->buffIdRx_m) != FALSE)
            {
                if(ssdo_initTransmitBuffer(chanId_p, pInitParam_p->buffIdTx_m) != FALSE)
                {
                    // Save channel Id
                    ssdoInstance_l[chanId_p].chanId_m = chanId_p;

                    // Save id of producing and consuming buffers
                    ssdoInstance_l[chanId_p].txBuffParam_m.idTxBuff_m = pInitParam_p->buffIdTx_m;
                    ssdoInstance_l[chanId_p].rxBuffParam_m.idRxBuff_m = pInitParam_p->buffIdRx_m;

                    // Set sequence number init value
                    ssdoInstance_l[chanId_p].txBuffParam_m.currTxSeqNr_m = kSeqNrValueSecond;

                    // Register receive handler
                    ssdoInstance_l[chanId_p].rxBuffParam_m.pfnRxHandler_m = pInitParam_p->pfnRxHandler_m;

                    // Set valid instance id
                    pInstance = &ssdoInstance_l[chanId_p];
                }
            }
        }
    }

    return pInstance;
}

//------------------------------------------------------------------------------
/**
\brief    Destroy an SSDO channel

\param[in]  pInstance_p       The instance to destroy

\ingroup module_ssdo
*/
//------------------------------------------------------------------------------
void ssdo_destroy(tSsdoInstance pInstance_p)
{
    if(pInstance_p != NULL)
    {
        APPIF_MEMSET(pInstance_p, 0, sizeof(struct eSsdoInstance));

        timeout_destroy(pInstance_p->txBuffParam_m.pTimeoutInst_m);
    }
}

//------------------------------------------------------------------------------
/**
\brief    Post a frame for transmission over the SSDO channel

\param[in]  pInstance_p     SSDO module instance
\param[in]  pPayload_p      Pointer to the payload to send
\param[in]  paylSize_p      Size of the payload to send

\return tSsdoTxStatus
\retval tSsdoTxStatusSuccessfull    Successfully posted payload to buffer
\retval tSsdoTxStatusBusy           Unable to post payload to SSDO channel
\retval tSsdoTxStatusError          Error while posting payload to the transmit channel

\ingroup module_ssdo
*/
//------------------------------------------------------------------------------
tSsdoTxStatus ssdo_postPayload(tSsdoInstance pInstance_p, UINT8* pPayload_p,
        UINT16 paylSize_p)
{
    tSsdoTxStatus chanState = kSsdoTxStatusError;

    if(pInstance_p == NULL  ||
       pPayload_p == NULL    )
    {
        error_setError(kAppIfModuleSsdo, kAppIfSsdoSendError);
    }
    else
    {
        // Check if payload fits inside the buffer
        if(paylSize_p > sizeof(pInstance_p->txBuffParam_m.ssdoTxBuffer_m.pSsdoTxPayl_m->tssdoTransmitData_m) ||
           paylSize_p == 0                             )
        {
            error_setError(kAppIfModuleSsdo, kAppIfSsdoTxConsSizeInvalid);
        }
        else
        {
            // Check if buffer is free for filling
            if(pInstance_p->txBuffParam_m.ssdoTxBuffer_m.isLocked_m == FALSE)
            {
                // Fill buffer
                APPIF_MEMCPY(pInstance_p->txBuffParam_m.ssdoTxBuffer_m.pSsdoTxPayl_m->tssdoTransmitData_m,
                        pPayload_p, paylSize_p);

                // Set transmit size
                AmiSetWordToLe((UINT8*)&pInstance_p->txBuffParam_m.ssdoTxBuffer_m.pSsdoTxPayl_m->paylSize_m, paylSize_p);

                // Set sequence number in next tx buffer
                AmiSetByteToLe((UINT8*)&pInstance_p->txBuffParam_m.ssdoTxBuffer_m.pSsdoTxPayl_m->seqNr_m,
                        pInstance_p->txBuffParam_m.currTxSeqNr_m);

                // Lock buffer for transmission
                pInstance_p->txBuffParam_m.ssdoTxBuffer_m.isLocked_m = TRUE;

                // Enable transmit timer
                timeout_startTimer(pInstance_p->txBuffParam_m.pTimeoutInst_m);

                // Acknowledge producing transmit buffer
                stream_ackBuffer(pInstance_p->txBuffParam_m.idTxBuff_m);

                chanState = kSsdoTxStatusSuccessful;
            }
            else
            {
                chanState = kSsdoTxStatusBusy;
            }
        }
    }

    return chanState;
}

//------------------------------------------------------------------------------
/**
\brief    Process the SSDO module

\param[in]  pInstance_p     SSDO module instance

\return BOOL
\retval TRUE        SSDO frames processed successfully
\retval FALSE       Error while processing SSDO frames

\ingroup module_ssdo
*/
//------------------------------------------------------------------------------
BOOL ssdo_process(tSsdoInstance pInstance_p)
{
    BOOL fReturn = FALSE;

    // Process incoming frames
    if(ssdo_handleRxFrame(pInstance_p) != FALSE)
    {
        // Process transmit frames
        ssdo_handleTxFrame(pInstance_p);

        fReturn = TRUE;
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
\brief    Initialize the SSDO receive buffer

\param[in] chanId_p                 Id of the SSDO channel
\param[in] rxBuffId_p               Id of the SSDO receive buffer

\return BOOL
\retval TRUE    Initialization successful
\retval FALSE   Error while initializing

\ingroup module_ssdo
*/
//------------------------------------------------------------------------------
static BOOL ssdo_initReceiveBuffer(tSsdoChanNum chanId_p,
        tTbufNumLayout rxBuffId_p)
{
    BOOL fReturn = FALSE;
    tBuffDescriptor* pDescSsdoRcv;

    pDescSsdoRcv = stream_getBufferParam(rxBuffId_p);
    if(pDescSsdoRcv->pBuffBase_m != NULL)
    {
        if(pDescSsdoRcv->buffSize_m == sizeof(tTbufSsdoRxStructure))
        {
            // Remember buffer address for later usage
            ssdoInstance_l[chanId_p].rxBuffParam_m.pSsdoRxBuffer_m =
                    (tTbufSsdoRxStructure *)pDescSsdoRcv->pBuffBase_m;

            // Register frame receive post action
            if(stream_registerAction(kStreamActionPost, rxBuffId_p,
                    ssdo_receiveFrame, (void *)&ssdoInstance_l[chanId_p]) != FALSE)
            {
                fReturn = TRUE;
            }
        }
        else
        {
            error_setError(kAppIfModuleSsdo, kAppIfSsdoBufferSizeMismatch);
        }
    }
    else
    {
        error_setError(kAppIfModuleSsdo, kAppIfSsdoInvalidBuffer);
    }

    return fReturn;
}

//------------------------------------------------------------------------------
/**
\brief    Initialize the SSDO transmit buffer

\param[in] chanId_p                 Id of the SSDO channel
\param[in] txBuffId_p               Id of the SSDO transmit buffer

\return BOOL
\retval TRUE    Initialization successful
\retval FALSE   Error while initializing

\ingroup module_ssdo
*/
//------------------------------------------------------------------------------
static BOOL ssdo_initTransmitBuffer(tSsdoChanNum chanId_p,
        tTbufNumLayout txBuffId_p)
{
    BOOL fReturn = FALSE;
    tBuffDescriptor* pDescSsdoTrans;

    pDescSsdoTrans = stream_getBufferParam(txBuffId_p);
    if(pDescSsdoTrans->pBuffBase_m != NULL)
    {
        if(pDescSsdoTrans->buffSize_m == sizeof(tTbufSsdoTxStructure))
        {
            // Remember buffer address for later usage
            ssdoInstance_l[chanId_p].txBuffParam_m.ssdoTxBuffer_m.pSsdoTxPayl_m =
                    (tTbufSsdoTxStructure *)pDescSsdoTrans->pBuffBase_m;

            // Initialize SSDO transmit timeout instance
            ssdoInstance_l[chanId_p].txBuffParam_m.pTimeoutInst_m = timeout_create(
                    SSDO_TX_TIMEOUT_CYCLE_COUNT);
            if(ssdoInstance_l[chanId_p].txBuffParam_m.pTimeoutInst_m != NULL)
            {
                fReturn = TRUE;
            }
            else
            {
                error_setError(kAppIfModuleSsdo, kAppIfSsdoInitError);
            }
        }
        else
        {
            error_setError(kAppIfModuleSsdo, kAppIfSsdoBufferSizeMismatch);
        }
    }
    else
    {
        error_setError(kAppIfModuleSsdo, kAppIfSsdoInvalidBuffer);
    }

    return fReturn;
}

//------------------------------------------------------------------------------
/**
\brief    Process SSDO receive frames

\param[in] pInstance_p     SSDO module instance

\return BOOL
\retval TRUE       Successfully processed SSDO receive frame
\retval FALSE      Error while processing

\ingroup module_ssdo
*/
//------------------------------------------------------------------------------
static BOOL ssdo_handleRxFrame(tSsdoInstance pInstance_p)
{
    BOOL fReturn = FALSE;
    UINT8*  pRxBuffer;
    UINT16  rxBuffSize;

    if(pInstance_p->rxBuffParam_m.fRxFrameIncoming_m != FALSE)
    {
        // Frame incoming -> forward to the user
        pRxBuffer = &pInstance_p->rxBuffParam_m.pSsdoRxBuffer_m->ssdoStubDataDom_m[0];
        rxBuffSize = AmiGetWordFromLe((UINT8 *)&pInstance_p->rxBuffParam_m.pSsdoRxBuffer_m->paylSize_m);

        // Call SSDO user handler
        if(pInstance_p->rxBuffParam_m.pfnRxHandler_m(pRxBuffer, rxBuffSize))
        {
            fReturn = TRUE;
        }
        else
        {
            error_setError(kAppIfModuleSsdo, kAppIfSsdoProcessingFailed);
        }

        // Access finished -> Unblock channel by writing current sequence number to status field!
        status_setSsdoRxChanFlag(pInstance_p->chanId_m,
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
\brief    Process SSDO transmit frames

\param[in]  pInstance_p     SSDO module instance

\ingroup module_ssdo
*/
//------------------------------------------------------------------------------
static void ssdo_handleTxFrame(tSsdoInstance pInstance_p)
{
    tSsdoChanStatus  txChanState;
    tTimerStatus timerState;

    if(pInstance_p->txBuffParam_m.ssdoTxBuffer_m.isLocked_m != FALSE)
    {
        // Check if channel is ready for transmission
        txChanState = ssdo_checkChannelStatus(pInstance_p);
        if(txChanState == kChanStatusFree)
        {
            // Ongoing message is acknowledged
            pInstance_p->txBuffParam_m.ssdoTxBuffer_m.isLocked_m = FALSE;

            // Increment local sequence number
            ssdo_changeLocalSeqNr(&pInstance_p->txBuffParam_m.currTxSeqNr_m);

            // Message was acknowledged -> Stop timer if running!
            timeout_stopTimer(pInstance_p->txBuffParam_m.pTimeoutInst_m);
        }
        else
        {
            // Check if timeout counter is expired
            timerState = timeout_checkExpire(pInstance_p->txBuffParam_m.pTimeoutInst_m);
            if(timerState == kTimerStateExpired)
            {
                // Timeout occurred -> Increment local sequence number!
                ssdo_changeLocalSeqNr(&pInstance_p->txBuffParam_m.currTxSeqNr_m);

                // Unlock channel anyway!
                pInstance_p->txBuffParam_m.ssdoTxBuffer_m.isLocked_m = FALSE;
            }
        }
    }
}

//------------------------------------------------------------------------------
/**
\brief    Check for incoming frame from SSDO channel

\param[in] pBuffer_p        Pointer to the base address of the buffer
\param[in] bufSize_p        Size of the buffer
\param[in] pUserArg_p       The user argument

\return BOOL
\retval TRUE          Successfully processed receive frame

\ingroup module_ssdo
*/
//------------------------------------------------------------------------------
static BOOL ssdo_receiveFrame(UINT8* pBuffer_p, UINT16 bufSize_p,
        void* pUserArg_p)
{
    tSsdoInstance pInstance;
    tTbufSsdoRxStructure*  pSsdoRxBuff;
    tSeqNrValue  currSeqNr = kSeqNrValueInvalid;

    UNUSED_PARAMETER(bufSize_p);

    // Get pointer to current instance
    pInstance = (tSsdoInstance) pUserArg_p;

    // Increment transmit timer cycle count
    timeout_incrementCounter(pInstance->txBuffParam_m.pTimeoutInst_m);

    // Convert to status buffer structure
    pSsdoRxBuff = (tTbufSsdoRxStructure*) pBuffer_p;

    // Acknowledge buffer before access
    stream_ackBuffer(pInstance->rxBuffParam_m.idRxBuff_m);

    currSeqNr = (tSeqNrValue)AmiGetByteFromLe((UINT8 *)&pSsdoRxBuff->seqNr_m);

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

    return TRUE;
}

//------------------------------------------------------------------------------
/**
\brief    Change local sequence number

\param[inout] pSeqNr_p        Changed sequence number

\ingroup module_ssdo
*/
//------------------------------------------------------------------------------
static void ssdo_changeLocalSeqNr(tSeqNrValue* pSeqNr_p)
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

\ingroup module_ssdo
*/
//------------------------------------------------------------------------------
static tSsdoChanStatus ssdo_checkChannelStatus(tSsdoInstance pInstance_p)
{
    tSsdoChanStatus chanStatus = kChanStatusInvalid;
    tSeqNrValue  seqNr = kSeqNrValueInvalid;

    // Get status of transmit channel
    status_getSsdoTxChanFlag(pInstance_p->chanId_m, &seqNr);

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

#endif // #if (((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_SSDO)) != 0)
