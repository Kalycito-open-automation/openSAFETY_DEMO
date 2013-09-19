/**
********************************************************************************
\file   appif-cc.c

\brief  Handles the configuration channel for incoming and outgoing objects

This modules forwards incoming object data from the occ to the local object list
and forwards objects from the user application to the PCP.

\ingroup module_cc
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

#include "appif-ccint.h"

#include "appif-streamint.h"
#include "appif-statusint.h"

#include <appif-ccobject.h>
#include <appif-ccobjectlist.h>
#include <appif-timeout.h>

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

#define CC_TX_TIMEOUT_CYCLE_COUNT        400    ///< Number of cycles after a transmit has a timeout

//------------------------------------------------------------------------------
// local types
//------------------------------------------------------------------------------

typedef struct {
    UINT8                   isLocked_m;        ///< Is buffer free for filling
    tTbufCcStructure*       pIccTxPayl_m;      ///< Pointer to the Icc transmit buffer
} tCcTxBuffer;

/**
 * \brief Parameter type of the transmit buffer
 */
typedef struct {
    tTbufNumLayout     idIccTx_m;          ///< Input transmit buffer id
    tCcTxBuffer        iccTxBuffer_m;      ///< Parameters of the Icc buffer
    tSeqNrValue        currTxSeqNr_m;      ///< Current transmit sequence number
    tTimeoutInstance   pTimeoutInst_m;     ///< Timer instance for asynchronous transmissions
} tCcTxChannel;

/**
 * \brief Parameter type of the receive buffer
 */
typedef struct {
    tTbufNumLayout     idOccRx_m;           ///< Output receive buffer id
    tTbufCcStructure*  pOccLayout_m;        ///< Pointer to the Occ transmit buffer
} tCcRxChannel;

/**
\brief CC user instance type
*/
typedef struct {
    tCcTxChannel       txChannel_m;         ///< Application outgoing channel
    tCcRxChannel       rxChannel_m;         ///< Application incoming channel
} tCcInstance;

//------------------------------------------------------------------------------
// local vars
//------------------------------------------------------------------------------

static tCcInstance          ccInstance_l;

//------------------------------------------------------------------------------
// local function prototypes
//------------------------------------------------------------------------------
static tAppIfStatus cc_initIccTxBuffer(tTbufNumLayout iccId_p);
static tAppIfStatus cc_initOccRxBuffer(tTbufNumLayout occId_p);
static tAppIfStatus cc_initCcObjects(void);
static tAppIfStatus cc_processTxObject(void);
static void cc_changeLocalSeqNr(tSeqNrValue* pSeqNr_p);
static tAppIfStatus cc_checkIccStatus(void);
static tAppIfStatus cc_handleOccRxObjects(UINT8* pBuffer_p, UINT16 bufSize_p,
        void* pUserArg_p);

//============================================================================//
//            P U B L I C   F U N C T I O N S                                 //
//============================================================================//

//------------------------------------------------------------------------------
/**
\brief    Initialize the cc module

Initialize the input and output configuration channel

\param[in]  pCcInitParam_p         Configuration channel initialization structure

\return tAppIfStatus
\retval kAppIfSuccessful           On success
\retval kAppIfConfChanInitError    Unable to initialize the configuration channel

\ingroup module_cc
*/
//------------------------------------------------------------------------------
tAppIfStatus cc_init(tCcInitParam* pCcInitParam_p)
{
    tAppIfStatus  ret = kAppIfSuccessful;

    APPIF_MEMSET(&ccInstance_l, 0 , sizeof(tCcInstance));

#ifdef _DEBUG
    if(pCcInitParam_p == NULL)
    {
        ret = kAppIfConfChanInitError;
    }
#endif

    if(ret == kAppIfSuccessful)
    {
        // Register icc tx buffer to stream module
        ret = cc_initIccTxBuffer(pCcInitParam_p->iccId_m);
        if(ret == kAppIfSuccessful)
        {
            // Register occ rx buffer to stream module
            ret = cc_initOccRxBuffer(pCcInitParam_p->occId_m);
            if(ret == kAppIfSuccessful)
            {
                // Initialize the configuration channel objects module
                ret = ccobject_init(pCcInitParam_p->pfnCritSec_p);
                if(ret == kAppIfSuccessful)
                {
                    // Initialize the cc object internal list
                    ret = cc_initCcObjects();
                    if(ret == kAppIfSuccessful)
                    {
                        // Fill local instance structure
                        ccInstance_l.txChannel_m.idIccTx_m = pCcInitParam_p->iccId_m;
                        ccInstance_l.rxChannel_m.idOccRx_m = pCcInitParam_p->occId_m;

                        // Set sequence number init value
                        ccInstance_l.txChannel_m.currTxSeqNr_m = kSeqNrValueFirst;
                    }
                }
            }
        }
    }

    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Close the configuration channel

\ingroup module_cc
*/
//------------------------------------------------------------------------------
void cc_exit(void)
{
    // Free cc object list module
    ccobject_exit();

    // Destroy timeout module
    timeout_destroy(ccInstance_l.txChannel_m.pTimeoutInst_m);
}

//------------------------------------------------------------------------------
/**
\brief    Read a PCP object via the configuration channel

\param[in]  objIdx_p           Index of the to read object
\param[in]  objSubIdx_p        Subindex of the to read object
\param[out] ppObject_p         The read object data

\return tAppIfStatus
\retval kAppIfSuccessful                   On success
\retval kAppIfConfChanObjectNotFound       Failed to read the object

\ingroup module_cc
*/
//------------------------------------------------------------------------------
tAppIfStatus cc_readObject(UINT16 objIdx_p, UINT8 objSubIdx_p,
        tConfChanObject** ppObject_p)
{
    tAppIfStatus ret = kAppIfSuccessful;

    // Read object from local object list
    ret = ccobject_readObject(objIdx_p, objSubIdx_p, ppObject_p);

    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Write a PCP object via the configuration channel

\param[in] pObject_p         The object to write

\return tAppIfStatus
\retval kAppIfSuccessful                  On success
\retval kAppIfConfChanInvalidSizeOfObj    Invalid object size
\retval kAppIfConfChanChannelBusy         Channel is currently busy writing an old object

\ingroup module_cc
*/
//------------------------------------------------------------------------------
tAppIfStatus cc_writeObject(tConfChanObject* pObject_p)
{
    tAppIfStatus ret = kAppIfSuccessful;

    if(pObject_p != NULL)
    {
        if(pObject_p->objSize_m > sizeof(UINT64) ||
           pObject_p->objSize_m == 0              )
        {
            ret = kAppIfConfChanInvalidSizeOfObj;
        }
    }
    else
    {
        ret = kAppIfConfChanInvalidParameter;
    }

    if(ret == kAppIfSuccessful)
    {
        // Check if buffer is free for filling
        if(ccInstance_l.txChannel_m.iccTxBuffer_m.isLocked_m == FALSE)
        {
            // Write object data to local object list
            ret = ccobject_writeObject(pObject_p);
            if(ret == kAppIfSuccessful)
            {
                // Fill transmit buffer
                APPIF_MEMCPY(&ccInstance_l.txChannel_m.iccTxBuffer_m.pIccTxPayl_m->objPayloadLow_m,
                        &pObject_p->objPayloadLow_m, pObject_p->objSize_m);

                // Set object index and subindex
                AmiSetWordToLe((UINT8*)&ccInstance_l.txChannel_m.iccTxBuffer_m.pIccTxPayl_m->objIdx_m,
                        pObject_p->objIdx_m);
                AmiSetByteToLe((UINT8*)&ccInstance_l.txChannel_m.iccTxBuffer_m.pIccTxPayl_m->objSubIdx_m,
                        pObject_p->objSubIdx_m);

                // Set sequence number in tx buffer
                AmiSetByteToLe((UINT8*)&ccInstance_l.txChannel_m.iccTxBuffer_m.pIccTxPayl_m->seqNr_m,
                        ccInstance_l.txChannel_m.currTxSeqNr_m);

                // Lock buffer for transmission
                ccInstance_l.txChannel_m.iccTxBuffer_m.isLocked_m = TRUE;

                // Enable transmit timer
                timeout_startTimer(ccInstance_l.txChannel_m.pTimeoutInst_m);

                // Acknowledge producing transmit buffer
                stream_ackBuffer(ccInstance_l.txChannel_m.idIccTx_m);
            }
        }
        else
        {
            ret = kAppIfConfChanChannelBusy;
        }
    }

    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Process configuration channel module

\return tAppIfStatus
\retval kAppIfSuccessful        On success
\retval Other                   Error while processing the background task

\ingroup module_cc
*/
//------------------------------------------------------------------------------
tAppIfStatus cc_process(void)
{
    tAppIfStatus ret = kAppIfSuccessful;

    // Process transmit channel objects
    ret = cc_processTxObject();

    return ret;
}


//============================================================================//
//            P R I V A T E   F U N C T I O N S                               //
//============================================================================//
/// \name Private Functions
/// \{

//------------------------------------------------------------------------------
/**
\brief    Process the transmit objects

Forward the object to the ICC buffer to update the object dictionary of the
PCP.

\return tAppIfStatus
\retval kAppIfSuccessful                On success

\ingroup module_cc
*/
//------------------------------------------------------------------------------
static tAppIfStatus cc_processTxObject(void)
{
    tAppIfStatus ret = kAppIfSuccessful;

    if(ccInstance_l.txChannel_m.iccTxBuffer_m.isLocked_m != FALSE)
    {
        // Check if channel is ready for transmission
        ret = cc_checkIccStatus();
        if(ret == kAppIfSuccessful)
        {
            // Ongoing message is acknowledged
            ccInstance_l.txChannel_m.iccTxBuffer_m.isLocked_m = FALSE;

            // Increment local sequence number
            cc_changeLocalSeqNr(&ccInstance_l.txChannel_m.currTxSeqNr_m);

            // When timer is running and ACK occurred -> Stop timer instance!
            if(timeout_isRunning(ccInstance_l.txChannel_m.pTimeoutInst_m))
            {
                timeout_stopTimer(ccInstance_l.txChannel_m.pTimeoutInst_m);
            }
        }
        else if(ret == kAppIfConfChanChannelBusy)
        {
            // Check if timeout counter is expired
            ret = timeout_checkExpire(ccInstance_l.txChannel_m.pTimeoutInst_m);
            if(ret == kAppIfTimeoutOccured)
            {
                // Timeout occurred -> Increment local sequence number!
                cc_changeLocalSeqNr(&ccInstance_l.txChannel_m.currTxSeqNr_m);

                // Unlock channel anyway!
                ccInstance_l.txChannel_m.iccTxBuffer_m.isLocked_m = FALSE;
            }

            ret = kAppIfSuccessful;
        }
    }

    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Initialize the Icc transmit buffer

\param[in] iccId_p               Id of the Icc transmit buffer

\return tAppIfStatus
\retval kAppIfSuccessful               On success
\retval kAppIfConfChanInitError        Unable to initialize timeout module
\retval kAppIfStreamInvalidBuffer      Invalid buffer! Can't register
\retval kAppIfAsyncBufferSizeMismatch  Size of the buffer is invalid

\ingroup module_cc
*/
//------------------------------------------------------------------------------
static tAppIfStatus cc_initIccTxBuffer(tTbufNumLayout iccId_p)
{
    tAppIfStatus ret = kAppIfSuccessful;
    tBuffDescriptor* pDescIccRcv;

    ret = stream_getBufferParam(iccId_p, &pDescIccRcv);
    if(ret == kAppIfSuccessful)
    {
        if(pDescIccRcv->buffSize_m == sizeof(tTbufCcStructure))
        {
            // Remember buffer address for later usage
            ccInstance_l.txChannel_m.iccTxBuffer_m.pIccTxPayl_m =
                    (tTbufCcStructure *)pDescIccRcv->pBuffBase_m;

            // Create timeout instance for transmit channel
            ccInstance_l.txChannel_m.pTimeoutInst_m = timeout_create(
                    CC_TX_TIMEOUT_CYCLE_COUNT);
            if(ccInstance_l.txChannel_m.pTimeoutInst_m == NULL)
            {
                ret = kAppIfConfChanInitError;
            }
        }
        else
        {
            ret = kAppIfConfChanBufferSizeMismatch;
        }
    }

    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Initialize the Occ receive buffer

\param[in] occId_p               Id of the Icc receive buffer

\return tAppIfStatus
\retval kAppIfSuccessful               On success
\retval kAppIfStreamInvalidBuffer      Invalid buffer! Can't register
\retval kAppIfAsyncBufferSizeMismatch  Size of the buffer is invalid

\ingroup module_cc
*/
//------------------------------------------------------------------------------
static tAppIfStatus cc_initOccRxBuffer(tTbufNumLayout occId_p)
{
    tAppIfStatus ret = kAppIfSuccessful;
    tBuffDescriptor* pDescOccTrans;

    ret = stream_getBufferParam(occId_p, &pDescOccTrans);
    if(ret == kAppIfSuccessful)
    {
        if(pDescOccTrans->buffSize_m == sizeof(tTbufCcStructure))
        {
            // Remember buffer address for later usage
            ccInstance_l.rxChannel_m.pOccLayout_m =
                    (tTbufCcStructure *)pDescOccTrans->pBuffBase_m;

            // Register object receive post action
            ret = stream_registerAction(kStreamActionPost, occId_p,
                    cc_handleOccRxObjects, NULL);
        }
        else
        {
            ret = kAppIfConfChanBufferSizeMismatch;
        }
    }

    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Initialize the list of configuration channel objects

\return tAppIfStatus
\retval kAppIfSuccessful                On success
\retval kAppIfMainInitCcObjectFailed    Init of CC object failed

\ingroup module_cc
*/
//------------------------------------------------------------------------------
static tAppIfStatus cc_initCcObjects(void)
{
    tAppIfStatus     ret = kAppIfSuccessful;
    tConfChanObject  object;
    UINT8            i;
    UINT64           paylDest = 0;
    tCcObject        initObjList[CONF_CHAN_NUM_OBJECTS] = CCOBJECT_LIST_INIT_VECTOR;

    APPIF_MEMSET(&object, 0, sizeof(tConfChanObject));

    for(i=0; i < CONF_CHAN_NUM_OBJECTS; i++)
    {
        // Assemble object for list
        object.objIdx_m = initObjList[i].objIdx;
        object.objSubIdx_m = initObjList[i].objSubIdx;
        object.objSize_m = initObjList[i].objSize;
        APPIF_MEMCPY(&object.objPayloadLow_m, &paylDest, object.objSize_m);

        // Now initialize the object in the local list
        ret = ccobject_initObject(i, &object);
        if(ret != kAppIfSuccessful)
        {
            ret = kAppIfMainInitCcObjectFailed;
            break;
        }
    }

    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Change local sequence number

\param[out] pSeqNr_p        Changed sequence number

\ingroup module_cc
*/
//------------------------------------------------------------------------------
static void cc_changeLocalSeqNr(tSeqNrValue* pSeqNr_p)
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

\return tAppIfStatus
\retval kAppIfSuccessful           Channel is free for transmission
\retval kAppIfConfChanChannelBusy  Channel is currently transmitting

\ingroup module_cc
*/
//------------------------------------------------------------------------------
static tAppIfStatus cc_checkIccStatus(void)
{
    tAppIfStatus ret = kAppIfSuccessful;
    tSeqNrValue  seqNr = kSeqNrValueInvalid;

    // Get status of transmit channel
    status_getIccStatus(&seqNr);

    // Check if old transmission is already finished!
    if(seqNr != ccInstance_l.txChannel_m.currTxSeqNr_m)
    {
        // Message in progress -> retry later!
        ret = kAppIfConfChanChannelBusy;
    }

    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Handle incoming objects from the occ buffer

\param[in] pBuffer_p        Pointer to the base address of the buffer
\param[in] bufSize_p        Size of the buffer
\param[in] pUserArg_p       The user argument

\return tAppIfStatus
\retval kAppIfSuccessful          On success

\ingroup module_cc
*/
//------------------------------------------------------------------------------
static tAppIfStatus cc_handleOccRxObjects(UINT8* pBuffer_p, UINT16 bufSize_p,
        void* pUserArg_p)
{
    tAppIfStatus ret = kAppIfSuccessful;
    tTbufCcStructure*  pOccBuff;

    // Convert to configuration channel buffer structure
    pOccBuff = (tTbufCcStructure*) pBuffer_p;

#ifdef _DEBUG
    // Check size of buffer
    if(bufSize_p != sizeof(tTbufCcStructure))
    {
        ret = kAppIfConfChanBufferSizeMismatch;
    }
#endif

    if(ret == kAppIfSuccessful)
    {
        // Increment transmit timer cycle count
        timeout_incrementCounter(ccInstance_l.txChannel_m.pTimeoutInst_m);

        // Forward receive objects to local list
        ret = ccobject_writeCurrObject(pOccBuff->objIdx_m, pOccBuff->objSubIdx_m,
                (UINT8*)&pOccBuff->objPayloadLow_m);
        if(ret == kAppIfConfChanObjListOutOfSync)
        {
            // Don't update object and wait for sync again
            ret = kAppIfSuccessful;
        }
        else if(ret == kAppIfSuccessful)
        {
            // Increment local object write pointer
            ccobject_incObjWritePointer();
        }
    }

    return ret;
}

/// \}


