/**
********************************************************************************
\file   appif-status.c

\brief  Status module for synchronization information forwarding

This module forwards the time information to the user application.
It also provides the asynchronous channel status information.

\ingroup module_status
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

//------------------------------------------------------------------------------
// local types
//------------------------------------------------------------------------------

/**
\brief status user instance type

The status instance holds the status information of this module
*/
typedef struct
{
    tTbufStatusOutStructure  tbufStatusOutLayout_m;  ///< Local copy of the status output triple buffer
    UINT8                    buffOutId_m;            ///< Id of the output status register buffer
    UINT8                    iccStatus_m;            ///< Icc status register
    UINT16                   asyncTxStatus_m;        ///< Status of the asynchronous transmit channel

    tTbufStatusInStructure  tbufStatusInLayout_m;  ///< Local copy of the status incoming triple buffer
    UINT8                   buffInId_m;            ///< Id of the incoming status register buffer
    UINT16                  asyncRxStatus_m;       ///< Status of the asynchronous receive channel

    tAppIfAppCbSync       pfnAppCbSync_m;       ///< Synchronous callback function
} tStatusInstance;

//------------------------------------------------------------------------------
// local vars
//------------------------------------------------------------------------------

static tStatusInstance          statusInstance_l;

//------------------------------------------------------------------------------
// local function prototypes
//------------------------------------------------------------------------------
static tAppIfStatus status_initBuffer( UINT8 buffId_p, UINT8* buffBase_p,
        UINT16 buffSize_p);
static tAppIfStatus status_processSync(UINT8* pBuffer_p, UINT16 bufSize_p,
        void * pUserArg_p);
static tAppIfStatus status_updateOutStatusReg(UINT8* pBuffer_p, UINT16 bufSize_p,
        void * pUserArg_p);
static tAppIfStatus status_updateInStatusReg(UINT8* pBuffer_p, UINT16 bufSize_p,
        void * pUserArg_p);

//============================================================================//
//            P U B L I C   F U N C T I O N S                                 //
//============================================================================//

//------------------------------------------------------------------------------
/**
\brief    Initialize the status module

\param[in]  pInitParam_p     Initialization structure of the status module

\return  tAppIfStatus
\retval  kAppIfSuccessful          On success
\retval  kAppIfStatusInitError     Unable to init the status module

\ingroup module_status
*/
//------------------------------------------------------------------------------
tAppIfStatus status_init(tStatusInitParam* pInitParam_p)
{
    tAppIfStatus ret = kAppIfSuccessful;

    APPIF_MEMSET(&statusInstance_l, 0 , sizeof(tStatusInstance));

    if(pInitParam_p == NULL)
    {
        ret = kAppIfStatusInitError;
        goto Exit;
    }

    if(pInitParam_p->pfnAppCbSync_m == NULL ||
       pInitParam_p->buffOutId_m == 0           )
    {
        ret = kAppIfStatusInitError;
        goto Exit;
    }

    // Remember id of the buffer
    statusInstance_l.buffOutId_m = pInitParam_p->buffOutId_m;

    // Remember synchronous callback function
    statusInstance_l.pfnAppCbSync_m = pInitParam_p->pfnAppCbSync_m;

    // Register status outgoing triple buffer
    ret = status_initBuffer(pInitParam_p->buffOutId_m,
            (UINT8 *)&statusInstance_l.tbufStatusOutLayout_m,
            sizeof(tTbufStatusOutStructure));
    if(ret != kAppIfSuccessful)
    {
        goto Exit;
    }

    // Register status incoming triple buffer
    ret = status_initBuffer(pInitParam_p->buffInId_m,
            (UINT8 *)&statusInstance_l.tbufStatusInLayout_m,
            sizeof(tTbufStatusInStructure));
    if(ret != kAppIfSuccessful)
    {
        goto Exit;
    }

    // Register status module pre action for sync processing
    ret = stream_registerAction(kStreamActionPre, pInitParam_p->buffOutId_m,
            status_processSync, NULL);
    if(ret != kAppIfSuccessful)
    {
        goto Exit;
    }

    // Register outgoing status module post action for status register update
    ret = stream_registerAction(kStreamActionPost, pInitParam_p->buffOutId_m,
            status_updateOutStatusReg, NULL);
    if(ret != kAppIfSuccessful)
    {
        goto Exit;
    }

    // Register incoming status module post action for status register update
    ret = stream_registerAction(kStreamActionPost, pInitParam_p->buffInId_m,
            status_updateInStatusReg, NULL);
    if(ret != kAppIfSuccessful)
    {
        goto Exit;
    }

Exit:
    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Cleanup status module

\ingroup module_status
*/
//------------------------------------------------------------------------------
void status_exit(void)
{
    // Free module internals
}

//------------------------------------------------------------------------------
/**
\brief    Get Icc status register

\param[out]  pSeqNr_p        The current status of the channel

\ingroup module_status
*/
//------------------------------------------------------------------------------
void status_getIccStatus(tSeqNrValue* pSeqNr_p)
{
    // Reformat to sequence number type
    if(CHECK_BIT(statusInstance_l.iccStatus_m, STATUS_ICC_BUSY_FLAG_POS))
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
\brief    Set the asynchronous receive channel to next element

\param[in] chanNum_p     Id of the channel to mark as busy
\param[in] seqNr_p       The value of the sequence number

\ingroup module_status
*/
//------------------------------------------------------------------------------
void status_setAsyncRxChanFlag(UINT8 chanNum_p, tSeqNrValue seqNr_p)
{
    if(seqNr_p == kSeqNrValueFirst)
    {
        statusInstance_l.asyncRxStatus_m &= ~(1<<chanNum_p);
    }
    else
    {
        statusInstance_l.asyncRxStatus_m |= (1<<chanNum_p);
    }

}

//------------------------------------------------------------------------------
/**
\brief    Get the asynchronous transmit channel status

\param[in]  chanNum_p     Id of the channel to mark as busy
\param[out] pSeqNr_p      The value of the sequence number

\ingroup module_status
*/
//------------------------------------------------------------------------------
void status_getAsyncTxChanFlag(UINT8 chanNum_p, tSeqNrValue* pSeqNr_p)
{
    // Reformat to sequence number type
    if(CHECK_BIT(statusInstance_l.asyncTxStatus_m,chanNum_p))
    {
        *pSeqNr_p = kSeqNrValueSecond;
    }
    else
    {
        *pSeqNr_p = kSeqNrValueFirst;
    }
}

//============================================================================//
//            P R I V A T E   F U N C T I O N S                               //
//============================================================================//
/// \name Private Functions
/// \{

//------------------------------------------------------------------------------
/**
\brief    Initialize a status buffer

\param[in] buffId_p            Id of the buffer to initialize
\param[in] buffBase_p          Base address of the buffer
\param[in] buffSize_p          Size of the buffer

\return tAppIfStatus
\retval kAppIfSuccessful            On success
\retval kAppIfStreamInvalidBuffer   Invalid buffer! Can't register

\ingroup module_status
*/
//------------------------------------------------------------------------------
static tAppIfStatus status_initBuffer( UINT8 buffId_p, UINT8* buffBase_p,
        UINT16 buffSize_p)
{
    tAppIfStatus ret = kAppIfSuccessful;
    tBuffParam buffParam;

    // Register rpdo buffer to stream module
    buffParam.buffId_m = buffId_p;
    buffParam.pBuffBase_m = buffBase_p;
    buffParam.buffSize_m = buffSize_p;

    ret = stream_registerBuffer(&buffParam);
    if(ret != kAppIfSuccessful)
    {
        goto Exit;
    }

Exit:
    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Process time synchronization task

\param[in] pBuffer_p        Pointer to the base address of the buffer
\param[in] bufSize_p        Size of the buffer
\param[in] pUserArg_p       User defined argument

\return tAppIfStatus
\retval kAppIfSuccessful                  On success
\retval kAppIfStatusBufferSizeMismatch    Invalid buffer to process
\retval Other                             Other user error occurred

\ingroup module_status
*/
//------------------------------------------------------------------------------
static tAppIfStatus status_processSync(UINT8* pBuffer_p, UINT16 bufSize_p,
        void * pUserArg_p)
{
    tAppIfStatus ret = kAppIfSuccessful;
    tAppIfTimeStamp        timeStamp;
    tTbufStatusOutStructure*  pStatusBuff;

    // Convert to status buffer structure
    pStatusBuff = (tTbufStatusOutStructure*) pBuffer_p;

    // Check size of buffer
    if(bufSize_p != sizeof(tTbufStatusOutStructure))
    {
        ret = kAppIfStatusBufferSizeMismatch;
        goto Exit;
    }

    // Call synchronous callback function
    timeStamp.relTimeLow_m = AmiGetDwordFromLe((UINT8 *)&pStatusBuff->relTimeLow_m);
    timeStamp.relTimeHigh_m = AmiGetDwordFromLe((UINT8 *)&pStatusBuff->relTimeHigh_m);

    ret = statusInstance_l.pfnAppCbSync_m(&timeStamp);
    if(ret != kAppIfSuccessful)
    {
        goto Exit;
    }

Exit:
    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Process outgoing status register fields

\param[in] pBuffer_p        Pointer to the base address of the buffer
\param[in] bufSize_p        Size of the buffer
\param[in] pUserArg_p       User defined argument

\return tAppIfStatus
\retval kAppIfSuccessful          On success

\ingroup module_status
*/
//------------------------------------------------------------------------------
static tAppIfStatus status_updateOutStatusReg(UINT8* pBuffer_p, UINT16 bufSize_p,
        void * pUserArg_p)
{
    tAppIfStatus ret = kAppIfSuccessful;
    tTbufStatusOutStructure*  pStatusBuff;

    // Convert to status buffer structure
    pStatusBuff = (tTbufStatusOutStructure*) pBuffer_p;

#ifdef _DEBUG
    // Check size of buffer
    if(bufSize_p != sizeof(tTbufStatusOutStructure))
    {
        ret = kAppIfStatusBufferSizeMismatch;
        goto Exit;
    }
#endif

    // Get CC status register
    statusInstance_l.iccStatus_m = AmiGetByteFromLe((UINT8 *)&pStatusBuff->iccStatus_m);

    // Update tx status register
    statusInstance_l.asyncTxStatus_m = AmiGetWordFromLe((UINT8 *)&pStatusBuff->asyncConsStatus_m);

    // Acknowledge buffer
    stream_ackBuffer(statusInstance_l.buffOutId_m);

#ifdef _DEBUG
Exit:
#endif
    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Process incoming status register fields

\param[in] pBuffer_p        Pointer to the base address of the buffer
\param[in] bufSize_p        Size of the buffer
\param[in] pUserArg_p       User defined argument

\return tAppIfStatus
\retval kAppIfSuccessful          On success

\ingroup module_status
*/
//------------------------------------------------------------------------------
static tAppIfStatus status_updateInStatusReg(UINT8* pBuffer_p, UINT16 bufSize_p,
        void * pUserArg_p)
{
    tAppIfStatus ret = kAppIfSuccessful;
    tTbufStatusInStructure*  pStatusBuff;

    // Convert to status buffer structure
    pStatusBuff = (tTbufStatusInStructure*) pBuffer_p;

#ifdef _DEBUG
    // Check size of buffer
    if(bufSize_p != sizeof(tTbufStatusInStructure))
    {
        ret = kAppIfStatusBufferSizeMismatch;
        goto Exit;
    }
#endif

    // Acknowledge buffer
    stream_ackBuffer(statusInstance_l.buffInId_m);

    // Write rx status register
    AmiSetWordToLe((UINT8 *)&pStatusBuff->asyncProdStatus_m, statusInstance_l.asyncRxStatus_m);

#ifdef _DEBUG
Exit:
#endif
    return ret;
}



/// \}


