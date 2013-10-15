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

#include <appif/appif-statusint.h>

#include <appif/appif-streamint.h>

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
static BOOL status_initOutBuffer(tTbufNumLayout statOutId_p);
static BOOL status_initInBuffer(tTbufNumLayout statInId_p);
static BOOL status_processSync(UINT8* pBuffer_p, UINT16 bufSize_p,
        void * pUserArg_p);
static BOOL status_updateOutStatusReg(UINT8* pBuffer_p, UINT16 bufSize_p,
        void * pUserArg_p);
static BOOL status_updateInStatusReg(UINT8* pBuffer_p, UINT16 bufSize_p,
        void * pUserArg_p);

//============================================================================//
//            P U B L I C   F U N C T I O N S                                 //
//============================================================================//

//------------------------------------------------------------------------------
/**
\brief    Initialize the status module

\param[in]  pInitParam_p     Initialization structure of the status module

\return  BOOL
\retval  TRUE      Successfully initialized the status module
\retval  FALSE     Error during initialization

\ingroup module_status
*/
//------------------------------------------------------------------------------
BOOL status_init(tStatusInitParam* pInitParam_p)
{
    BOOL fReturn = FALSE;

    APPIF_MEMSET(&statusInstance_l, 0 , sizeof(tStatusInstance));

    if(pInitParam_p == NULL)
    {
        error_setError(kAppIfModuleStatus, kAppIfStatusInitError);
    }
    else
    {
        if(pInitParam_p->pfnAppCbSync_m == NULL    ||
           pInitParam_p->buffOutId_m >= kTbufCount ||
           pInitParam_p->buffInId_m >= kTbufCount   )
        {
            error_setError(kAppIfModuleStatus, kAppIfStatusInitError);
        }
        else
        {
            // Register status outgoing triple buffer
            if(status_initOutBuffer(pInitParam_p->buffOutId_m) != FALSE)
            {
                // Register status incoming triple buffer
                if(status_initInBuffer(pInitParam_p->buffInId_m) != FALSE)
                {
                    // Remember id of the buffer
                    statusInstance_l.buffOutId_m = pInitParam_p->buffOutId_m;

                    // Remember synchronous callback function
                    statusInstance_l.pfnAppCbSync_m = pInitParam_p->pfnAppCbSync_m;

                    fReturn = TRUE;
                }
                else
                {
                    error_setError(kAppIfModuleStatus, kAppIfStatusInitError);
                }
            }
            else
            {
                error_setError(kAppIfModuleStatus, kAppIfStatusInitError);
            }
        }
    }


    return fReturn;
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

\return BOOL
\retval TRUE        Successfully initialized the output buffer
\retval FALSE       Unable to initialize the output buffer

\ingroup module_status
*/
//------------------------------------------------------------------------------
static BOOL status_initOutBuffer( tTbufNumLayout statOutId_p)
{
    BOOL fReturn = FALSE;
    tBuffDescriptor* pDescStatOut;

    pDescStatOut = stream_getBufferParam(statOutId_p);
    if(pDescStatOut != NULL)
    {
        if(pDescStatOut->buffSize_m == sizeof(tTbufStatusOutStructure))
        {
            // Remember buffer address for later usage
            statusInstance_l.pStatusOutLayout_m = (tTbufStatusOutStructure *)pDescStatOut->pBuffBase_m;

            // Register status module pre action for sync processing
            if(stream_registerAction(kStreamActionPre, statOutId_p,
                    status_processSync, NULL) != FALSE)
            {
                // Register outgoing status module post action for status register update
                if(stream_registerAction(kStreamActionPost, statOutId_p,
                        status_updateOutStatusReg, NULL) != FALSE)
                {
                    fReturn = TRUE;
                }
            }
        }
        else
        {   // Invalid size of output buffer
            error_setError(kAppIfModuleStatus, kAppIfStatusBufferSizeMismatch);
        }
    }
    else
    {   // Invalid base address of output buffer
        error_setError(kAppIfModuleStatus, kAppIfStreamInvalidBuffer);
    }

    return fReturn;
}

//------------------------------------------------------------------------------
/**
\brief    Initialize the incoming status buffer

\param[in] statInId_p         Id of the buffer to initialize

\return BOOL
\retval TRUE        Successfully initialized the input buffer
\retval FALSE       Unable to initialize the input buffer

\ingroup module_status
*/
//------------------------------------------------------------------------------
static BOOL status_initInBuffer(tTbufNumLayout statInId_p)
{
    BOOL fReturn = FALSE;
    tBuffDescriptor* pDescStatIn;

    pDescStatIn = stream_getBufferParam(statInId_p);
    if(pDescStatIn != NULL)
    {
        if(pDescStatIn->buffSize_m == sizeof(tTbufStatusInStructure))
        {
            // Remember buffer address for later usage
            statusInstance_l.pStatusInLayout_m = (tTbufStatusInStructure *)pDescStatIn->pBuffBase_m;

            // Register incoming status module post action for status register update
            if(stream_registerAction(kStreamActionPost, statInId_p,
                    status_updateInStatusReg, NULL) != FALSE)
            {
                fReturn = TRUE;
            }
            else
            {   // Unable to register in buffer user action
                error_setError(kAppIfModuleStatus, kAppIfStreamInitError);
            }

        }
        else
        {
            // Invalid size of input buffer
            error_setError(kAppIfModuleStatus, kAppIfStatusBufferSizeMismatch);
        }
    }
    else
    {
        // Invalid base address of input buffer
        error_setError(kAppIfModuleStatus, kAppIfStreamInvalidBuffer);
    }

    return fReturn;
}

//------------------------------------------------------------------------------
/**
\brief    Process time synchronization task

\param[in] pBuffer_p        Pointer to the base address of the buffer
\param[in] bufSize_p        Size of the buffer
\param[in] pUserArg_p       User defined argument

\return BOOL
\retval TRUE        Successfully processed synchronous task
\retval FALSE       Error while processing sync task

\ingroup module_status
*/
//------------------------------------------------------------------------------
static BOOL status_processSync(UINT8* pBuffer_p, UINT16 bufSize_p,
        void * pUserArg_p)
{
    BOOL fReturn = FALSE;
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

        if(statusInstance_l.pfnAppCbSync_m(&timeStamp) != FALSE)
        {
            fReturn = TRUE;
        }
        else
        {
            error_setError(kAppIfModuleStatus, kAppIfStatusProcessSyncFailed);
        }
    }
    else
    {
        error_setError(kAppIfModuleStatus, kAppIfStatusBufferSizeMismatch);
    }

    return fReturn;
}

//------------------------------------------------------------------------------
/**
\brief    Process outgoing status register fields

\param[in] pBuffer_p        Pointer to the base address of the buffer
\param[in] bufSize_p        Size of the buffer
\param[in] pUserArg_p       User defined argument

\return BOOL
\retval TRUE        Successfully updated the status register
\retval FALSE       Error while updating the status register

\ingroup module_status
*/
//------------------------------------------------------------------------------
static BOOL status_updateOutStatusReg(UINT8* pBuffer_p, UINT16 bufSize_p,
        void * pUserArg_p)
{
    BOOL fReturn = FALSE;
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

        fReturn = TRUE;
    }
    else
    {
        error_setError(kAppIfModuleStatus, kAppIfStatusBufferSizeMismatch);
    }

    return fReturn;
}

//------------------------------------------------------------------------------
/**
\brief    Process incoming status register fields

\param[in] pBuffer_p        Pointer to the base address of the buffer
\param[in] bufSize_p        Size of the buffer
\param[in] pUserArg_p       User defined argument

\return BOOL
\retval TRUE        Successfully updated the status register
\retval FALSE       Error while updating the status register

\ingroup module_status
*/
//------------------------------------------------------------------------------
static BOOL status_updateInStatusReg(UINT8* pBuffer_p, UINT16 bufSize_p,
        void * pUserArg_p)
{
    BOOL fReturn = FALSE;
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

        fReturn = TRUE;
    }
    else
    {
        error_setError(kAppIfModuleStatus, kAppIfStatusBufferSizeMismatch);
    }

    return fReturn;
}

/// \}


