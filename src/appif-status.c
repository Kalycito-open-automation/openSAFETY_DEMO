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
    tTbufStatusOutStructure*  pStatusOutLayout_m;  ///< Local copy of the status output triple buffer
    tTbufNumLayout            buffOutId_m;            ///< Id of the output status register buffer
    UINT8                     iccStatus_m;            ///< Icc status register
    UINT16                    asyncTxStatus_m;        ///< Status of the asynchronous transmit channel

    tTbufStatusInStructure*   pStatusInLayout_m;  ///< Local copy of the status incoming triple buffer
    tTbufNumLayout            buffInId_m;            ///< Id of the incoming status register buffer
    UINT16                    asyncRxStatus_m;       ///< Status of the asynchronous receive channel

    tAppIfAppCbSync       pfnAppCbSync_m;       ///< Synchronous callback function
} tStatusInstance;

//------------------------------------------------------------------------------
// local vars
//------------------------------------------------------------------------------

static tStatusInstance          statusInstance_l;

//------------------------------------------------------------------------------
// local function prototypes
//------------------------------------------------------------------------------
static tAppIfStatus status_initOutBuffer(tTbufNumLayout statOutId_p);
static tAppIfStatus status_initInBuffer(tTbufNumLayout statInId_p);
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

#ifdef _DEBUG
    if(pInitParam_p != NULL)
    {
        if(pInitParam_p->pfnAppCbSync_m == NULL ||
           pInitParam_p->buffOutId_m == 0        )
        {
            ret = kAppIfStatusInitError;
        }
    }
    else
    {
        ret = kAppIfStatusInitError;
    }
#endif

    if(ret == kAppIfSuccessful)
    {
        // Remember id of the buffer
        statusInstance_l.buffOutId_m = pInitParam_p->buffOutId_m;

        // Remember synchronous callback function
        statusInstance_l.pfnAppCbSync_m = pInitParam_p->pfnAppCbSync_m;

        // Register status outgoing triple buffer
        ret = status_initOutBuffer(pInitParam_p->buffOutId_m);
        if(ret == kAppIfSuccessful)
        {
            // Register status incoming triple buffer
            ret = status_initInBuffer(pInitParam_p->buffInId_m);
        }
    }

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
\brief    Initialize the outgoing status buffer

\param[in] statOutId_p         Id of the buffer to initialize

\return tAppIfStatus
\retval kAppIfSuccessful             On success
\retval kAppIfStreamInvalidBuffer    Invalid buffer! Can't register
\retval kAppIfRpdoBufferSizeMismatch Invalid size of the buffer

\ingroup module_status
*/
//------------------------------------------------------------------------------
static tAppIfStatus status_initOutBuffer( tTbufNumLayout statOutId_p)
{
    tAppIfStatus ret = kAppIfSuccessful;
    tBuffDescriptor* pDescStatOut;

    ret = stream_getBufferParam(statOutId_p, &pDescStatOut);
    if(ret == kAppIfSuccessful)
    {
        if(pDescStatOut->buffSize_m == sizeof(tTbufStatusOutStructure))
        {
            // Remember buffer address for later usage
            statusInstance_l.pStatusOutLayout_m = (tTbufStatusOutStructure *)pDescStatOut->pBuffBase_m;

            // Register status module pre action for sync processing
            ret = stream_registerAction(kStreamActionPre, statOutId_p,
                    status_processSync, NULL);
            if(ret == kAppIfSuccessful)
            {
                // Register outgoing status module post action for status register update
                ret = stream_registerAction(kStreamActionPost, statOutId_p,
                        status_updateOutStatusReg, NULL);
            }
        }
        else
        {
            ret = kAppIfStatusBufferSizeMismatch;
        }
    }

    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Initialize the incoming status buffer

\param[in] statInId_p         Id of the buffer to initialize

\return tAppIfStatus
\retval kAppIfSuccessful             On success
\retval kAppIfStreamInvalidBuffer    Invalid buffer! Can't register
\retval kAppIfRpdoBufferSizeMismatch Invalid size of the buffer

\ingroup module_status
*/
//------------------------------------------------------------------------------
static tAppIfStatus status_initInBuffer(tTbufNumLayout statInId_p)
{
    tAppIfStatus ret = kAppIfSuccessful;
    tBuffDescriptor* pDescStatIn;

    ret = stream_getBufferParam(statInId_p, &pDescStatIn);
    if(ret == kAppIfSuccessful)
    {
        if(pDescStatIn->buffSize_m == sizeof(tTbufStatusInStructure))
        {
            // Remember buffer address for later usage
            statusInstance_l.pStatusInLayout_m = (tTbufStatusInStructure *)pDescStatIn->pBuffBase_m;

            // Register incoming status module post action for status register update
            ret = stream_registerAction(kStreamActionPost, statInId_p,
                    status_updateInStatusReg, NULL);

        }
        else
        {
            ret = kAppIfStatusBufferSizeMismatch;
        }
    }

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
    if(bufSize_p == sizeof(tTbufStatusOutStructure))
    {
        // Call synchronous callback function
        timeStamp.relTimeLow_m = AmiGetDwordFromLe((UINT8 *)&pStatusBuff->relTimeLow_m);
        timeStamp.relTimeHigh_m = AmiGetDwordFromLe((UINT8 *)&pStatusBuff->relTimeHigh_m);

        ret = statusInstance_l.pfnAppCbSync_m(&timeStamp);
    }
    else
    {
        ret = kAppIfStatusBufferSizeMismatch;
    }

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

    // Check size of buffer
    if(bufSize_p == sizeof(tTbufStatusOutStructure))
    {
        // Get CC status register
        statusInstance_l.iccStatus_m = AmiGetByteFromLe((UINT8 *)&pStatusBuff->iccStatus_m);

        // Update tx status register
        statusInstance_l.asyncTxStatus_m = AmiGetWordFromLe((UINT8 *)&pStatusBuff->asyncConsStatus_m);

        // Acknowledge buffer
        stream_ackBuffer(statusInstance_l.buffOutId_m);
    }
    else
    {
        ret = kAppIfStatusBufferSizeMismatch;
    }

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

    // Check size of buffer
    if(bufSize_p == sizeof(tTbufStatusInStructure))
    {
        // Acknowledge buffer
        stream_ackBuffer(statusInstance_l.buffInId_m);

        // Write rx status register
        AmiSetWordToLe((UINT8 *)&pStatusBuff->asyncProdStatus_m, statusInstance_l.asyncRxStatus_m);
    }
    else
    {
        ret = kAppIfStatusBufferSizeMismatch;
    }

    return ret;
}

/// \}


