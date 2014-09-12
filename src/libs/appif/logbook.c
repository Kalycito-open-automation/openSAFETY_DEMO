/**
********************************************************************************
\file   logbook.c

\brief  Creates an instance of the logbook module

This module handles the logbook channels between the PCP and the application.
It simply tunnels the logbook information to the POWERLINK processor which
forwards the data to the PLC.

\ingroup module_log
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

#include <libappif/internal/logbookinst.h>
#include <libappif/internal/logbook.h>

#include <libappif/internal/status.h>
#include <libappif/internal/stream.h>

#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_LOGBOOK)) != 0)

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
 * \brief Status of the logbook channel
 */
typedef enum {
    kChanStatusInvalid   = 0x00,    ///< Invalid channel status
    kChanStatusBusy      = 0x01,    ///< Channel is currently busy
    kChanStatusFree      = 0x02,    ///< Channel is free for transmission
} tLogChanStatus;

//------------------------------------------------------------------------------
// local vars
//------------------------------------------------------------------------------

static struct eLogInstance          logInstance_l[kNumLogInstCount];

//------------------------------------------------------------------------------
// local function prototypes
//------------------------------------------------------------------------------
static BOOL log_initTransmitBuffer(tLogChanNum chanId_p,
        tTbufNumLayout txBuffId_p);
static void log_handleTxFrame(tLogInstance pInstance_p);
static BOOL log_incrTimeout(UINT8* pBuffer_p, UINT16 bufSize_p,
                            void* pUserArg_p);
static void log_changeLocalSeqNr(tSeqNrValue* pSeqNr_p);
static tLogChanStatus log_checkChannelStatus(tLogInstance pInstance_p);

//============================================================================//
//            P U B L I C   F U N C T I O N S                                 //
//============================================================================//

//------------------------------------------------------------------------------
/**
\brief    Initialize the logbook channel module

\ingroup module_log
*/
//------------------------------------------------------------------------------
void log_init(void)
{
    APPIF_MEMSET(&logInstance_l, 0 , sizeof(struct eLogInstance) * kNumLogInstCount);
}


//------------------------------------------------------------------------------
/**
\brief    Create a logbook channel instance

Instantiate a new logbook channel which provides a transmit channel for
logging data to the POWERLINK processor.

\param[in]  chanId_p         Id of the logbook channel
\param[in]  pInitParam_p     Logbook module initialization structure

\return tLogInstance
\retval Address              Pointer to the instance of the channel
\retval NULL                 Unable to allocate instance

\ingroup module_log
*/
//------------------------------------------------------------------------------
tLogInstance log_create(tLogChanNum chanId_p, tLogInitParam* pInitParam_p)
{
    tLogInstance pInstance = NULL;

    if(chanId_p >= kNumLogInstCount ||
       pInitParam_p == NULL            )
    {
        error_setError(kAppIfModuleLogbook, kAppIfLogInitError);
    }
    else
    {
        if(pInitParam_p->buffIdTx_m >= kTbufCount)
        {
            error_setError(kAppIfModuleLogbook, kAppIfLogInitError);
        }
        else
        {
            if(log_initTransmitBuffer(chanId_p, pInitParam_p->buffIdTx_m) != FALSE)
            {
                // Save channel Id
                logInstance_l[chanId_p].chanId_m = chanId_p;

                // Save id of producing and consuming buffers
                logInstance_l[chanId_p].idTxBuff_m = pInitParam_p->buffIdTx_m;

                // Set sequence number init value
                logInstance_l[chanId_p].currTxSeqNr_m = kSeqNrValueSecond;

                // Set valid instance id
                pInstance = &logInstance_l[chanId_p];
            }
        }
    }

    return pInstance;
}

//------------------------------------------------------------------------------
/**
\brief    Destroy an logbook channel

\param[in]  pInstance_p       The instance to destroy

\ingroup module_log
*/
//------------------------------------------------------------------------------
void log_destroy(tLogInstance pInstance_p)
{
    if(pInstance_p != NULL)
    {
        APPIF_MEMSET(pInstance_p, 0, sizeof(struct eLogInstance));

        timeout_destroy(pInstance_p->pTimeoutInst_m);
    }
}

//------------------------------------------------------------------------------
/**
\brief    Returns the address of the current logbook buffer

\param[in]  pInstance_p      Logbook module instance
\param[out] ppLogData_p      Pointer to the result address of the payload

\return BOOL
\retval TRUE    Success on getting the buffer
\retval FALSE   Invalid parameter passed to function

\ingroup module_log
*/
//------------------------------------------------------------------------------
BOOL log_getCurrentLogBuffer(tLogInstance pInstance_p, tLogFormat ** ppLogData_p)
{
    BOOL fReturn = FALSE;

    if(pInstance_p != NULL && ppLogData_p != NULL)
    {
        if(pInstance_p->logTxBuffer_m.isLocked_m == FALSE)
        {
            *ppLogData_p = &pInstance_p->logTxBuffer_m.pLogTxPayl_m->logData_m;
            fReturn = TRUE;
        }
    }

    return fReturn;
}

//------------------------------------------------------------------------------
/**
\brief    Post a frame for transmission over the logbook channel

\param[in]  pInstance_p     Logbook module instance
\param[in]  pLogData_p      Pointer to the logger data to send

\return tLogTxStatus
\retval kLogTxStatusSuccessful      Successfully posted payload to buffer
\retval tLogTxStatusBusy            Unable to post payload to the logbook channel
\retval kLogTxStatusError           Error while posting payload to the logbook channel

\ingroup module_log
*/
//------------------------------------------------------------------------------
tLogTxStatus log_postLogEntry(tLogInstance pInstance_p, tLogFormat* pLogData_p)
{
    tLogTxStatus chanState = kLogTxStatusError;

    if(pInstance_p == NULL  ||
       pLogData_p == NULL    )
    {
        error_setError(kAppIfModuleLogbook, kAppIfLogSendError);
    }
    else
    {
        // Check if buffer is free for filling
        if(pInstance_p->logTxBuffer_m.isLocked_m == FALSE)
        {
            // Set sequence number in next tx buffer
            ami_setUint8Le((UINT8*)&pInstance_p->logTxBuffer_m.pLogTxPayl_m->seqNr_m,
                    pInstance_p->currTxSeqNr_m);

            // Lock buffer for transmission
            pInstance_p->logTxBuffer_m.isLocked_m = TRUE;

            // Enable transmit timer
            timeout_startTimer(pInstance_p->pTimeoutInst_m);

            // Acknowledge producing transmit buffer
            stream_ackBuffer(pInstance_p->idTxBuff_m);

            chanState = kLogTxStatusSuccessful;
        }
        else
        {
            chanState = kLogTxStatusBusy;
        }
    }

    return chanState;
}

//------------------------------------------------------------------------------
/**
\brief    Process the logbook module

\param[in]  pInstance_p     Logbook module instance

\return BOOL
\retval TRUE        Logbook processed successfully
\retval FALSE       Error while processing the logbook

\ingroup module_log
*/
//------------------------------------------------------------------------------
BOOL log_process(tLogInstance pInstance_p)
{
    // Process transmit frames
    log_handleTxFrame(pInstance_p);

    return TRUE;
}

//============================================================================//
//            P R I V A T E   F U N C T I O N S                               //
//============================================================================//
/// \name Private Functions
/// \{

//------------------------------------------------------------------------------
/**
\brief    Initialize the logbook transmit buffer

\param[in] chanId_p                 Id of the logbook channel
\param[in] txBuffId_p               Id of the logbook transmit buffer

\return BOOL
\retval TRUE    Initialization successful
\retval FALSE   Error while initializing

\ingroup module_log
*/
//------------------------------------------------------------------------------
static BOOL log_initTransmitBuffer(tLogChanNum chanId_p,
                                   tTbufNumLayout txBuffId_p)
{
    BOOL fReturn = FALSE;
    tBuffDescriptor* pDescLogTrans;

    pDescLogTrans = stream_getBufferParam(txBuffId_p);
    if(pDescLogTrans != NULL               &&
       pDescLogTrans->pBuffBase_m != NULL   )
    {
        if(pDescLogTrans->buffSize_m == sizeof(tTbufLogStructure))
        {
            if(stream_registerAction(kStreamActionPost, txBuffId_p, log_incrTimeout,
                                     (void *)&logInstance_l[chanId_p]) != FALSE)
            {
                // Remember buffer address for later usage
                logInstance_l[chanId_p].logTxBuffer_m.pLogTxPayl_m =
                        (tTbufLogStructure *)pDescLogTrans->pBuffBase_m;

                // Initialize logbook transmit timeout instance
                logInstance_l[chanId_p].pTimeoutInst_m = timeout_create(
                                        LOG_TX_TIMEOUT_CYCLE_COUNT);
                if(logInstance_l[chanId_p].pTimeoutInst_m != NULL)
                {
                    fReturn = TRUE;
                }
                else
                {
                    error_setError(kAppIfModuleLogbook, kAppIfLogInitError);
                }
            }   /* no else: Error already reported in register action function */
        }
        else
        {
            error_setError(kAppIfModuleLogbook, kAppIfLogBufferSizeMismatch);
        }
    }
    else
    {
        error_setError(kAppIfModuleLogbook, kAppIfLogInvalidBuffer);
    }

    return fReturn;
}

//------------------------------------------------------------------------------
/**
\brief    Process logbook transmit frames

\param[in]  pInstance_p     Logbook module instance

\ingroup module_log
*/
//------------------------------------------------------------------------------
static void log_handleTxFrame(tLogInstance pInstance_p)
{
    tLogChanStatus  txChanState;
    tTimerStatus timerState;

    if(pInstance_p->logTxBuffer_m.isLocked_m != FALSE)
    {
        // Check if channel is ready for transmission
        txChanState = log_checkChannelStatus(pInstance_p);
        if(txChanState == kChanStatusFree)
        {
            // Ongoing message is acknowledged
            pInstance_p->logTxBuffer_m.isLocked_m = FALSE;

            // Increment local sequence number
            log_changeLocalSeqNr(&pInstance_p->currTxSeqNr_m);

            // Message was acknowledged -> Stop timer if running!
            timeout_stopTimer(pInstance_p->pTimeoutInst_m);
        }
        else
        {
            // Check if timeout counter is expired
            timerState = timeout_checkExpire(pInstance_p->pTimeoutInst_m);
            if(timerState == kTimerStateExpired)
            {
                // Timeout occurred -> Increment local sequence number!
                log_changeLocalSeqNr(&pInstance_p->currTxSeqNr_m);

                // Unlock channel anyway!
                pInstance_p->logTxBuffer_m.isLocked_m = FALSE;
            }
        }
    }
}

//------------------------------------------------------------------------------
/**
\brief    Increment logbook transmit timeout

\param[in] pBuffer_p        Pointer to the base address of the buffer
\param[in] bufSize_p        Size of the buffer
\param[in] pUserArg_p       The user argument

\return Always returns success

\ingroup module_log
*/
//------------------------------------------------------------------------------
static BOOL log_incrTimeout(UINT8* pBuffer_p, UINT16 bufSize_p,
                            void* pUserArg_p)
{
    tLogInstance pInstance;

    UNUSED_PARAMETER(bufSize_p);
    UNUSED_PARAMETER(pBuffer_p);

    // Get pointer to current instance
    pInstance = (tLogInstance) pUserArg_p;

    // Increment transmit timer cycle count
    timeout_incrementCounter(pInstance->pTimeoutInst_m);

    return TRUE;
}

//------------------------------------------------------------------------------
/**
\brief    Change local sequence number

\param[inout] pSeqNr_p        Changed sequence number

\ingroup module_log
*/
//------------------------------------------------------------------------------
static void log_changeLocalSeqNr(tSeqNrValue* pSeqNr_p)
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

\return tLogChanStatus
\retval tChanStatusFree       Channel is free for transmission
\retval tChanStatusBusy       Channel is currently transmitting

\ingroup module_log
*/
//------------------------------------------------------------------------------
static tLogChanStatus log_checkChannelStatus(tLogInstance pInstance_p)
{
    tLogChanStatus chanStatus = kChanStatusInvalid;
    tSeqNrValue  seqNr = kSeqNrValueInvalid;

    // Get status of transmit channel
    status_getLogTxChanFlag(pInstance_p->chanId_m, &seqNr);

    // Check if old transmission is already finished!
    if(seqNr != pInstance_p->currTxSeqNr_m)
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

#endif // #if (((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_LOGBOOK)) != 0)
