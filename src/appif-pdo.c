/**
********************************************************************************
\file   appif-pdo.c

\brief  Module for handling of incoming and outgoing pdo buffers

This module forwards the mapped PDO's from the incoming buffers to the user
application and back.

\ingroup module_pdo
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

#include "appif-pdoint.h"

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
* \brief PDO user instance type
*/
typedef struct
{
    UINT8           rpdoId_m;           ///< Id of the rpdo buffer
    tTbufRpdoImage* pRpdoLayout_m;      ///< Pointer to the rpdo triple buffer
    UINT32          rpdoRelTimeLow_m;   ///< Low value of the relative time
    UINT8           tpdoId_m;           ///< Id of the tpdo buffer
    tTbufTpdoImage* pTpdoLayout_m;      ///< Pointer to the tpdo triple buffer
    tAppIfPdoCb     pfnPdoCb_m;         ///< Process PDO user callback function
} tPdoInstance;

//------------------------------------------------------------------------------
// local vars
//------------------------------------------------------------------------------

static tPdoInstance          pdoInstance_l;

//------------------------------------------------------------------------------
// local function prototypes
//------------------------------------------------------------------------------

static tAppIfStatus pdo_process(void);
static tAppIfStatus pdo_initRpdoBuffer(tTbufNumLayout rpdoId_p);
static tAppIfStatus pdo_initTpdoBuffer(tTbufNumLayout tpdoId_p);
static tAppIfStatus pdo_ackTpdo(UINT8* pBuffer_p, UINT16 bufSize_p, void * pUserArg_p);
static tAppIfStatus pdo_processRpdo(UINT8* pBuffer_p, UINT16 bufSize_p, void * pUserArg_p);

//============================================================================//
//            P U B L I C   F U N C T I O N S                                 //
//============================================================================//

//------------------------------------------------------------------------------
/**
\brief    Initialize the PDO module

\param[in]  pfnPdoCb_p          PDO process user callback function
\param[in]  pPdoInitParam_p     Initialization structure of the PDO buffers

\return  tAppIfStatus
\retval  kAppIfSuccessful          On success
\retval  kAppIfPdoInitError        Unable to initialize the PDO module
\retval  Other                     Other internal error occurred

\ingroup module_pdo
*/
//------------------------------------------------------------------------------
tAppIfStatus pdo_init(tAppIfPdoCb pfnPdoCb_p, tPdoInitParam* pPdoInitParam_p)
{
    tAppIfStatus ret = kAppIfSuccessful;

    APPIF_MEMSET(&pdoInstance_l, 0 , sizeof(tPdoInstance));

#ifdef _DEBUG
    if(pfnPdoCb_p == NULL      ||
       pPdoInitParam_p == NULL  )
    {
        ret = kAppIfPdoInitError;
    }
#endif

    if(ret == kAppIfSuccessful)
    {
        if(pPdoInitParam_p->buffIdRpdo_m != 0 &&
           pPdoInitParam_p->buffIdTpdo_m != 0   )
        {
            if(pPdoInitParam_p->buffIdTpdo_m != 0)
            {
                // Initialize the Rpdo buffer
                ret = pdo_initRpdoBuffer(pPdoInitParam_p->buffIdRpdo_m);
            }

            if(pPdoInitParam_p->buffIdTpdo_m != 0 &&
               ret == kAppIfSuccessful)
            {
                // Initialize the Tpdo buffer
                ret = pdo_initTpdoBuffer(pPdoInitParam_p->buffIdTpdo_m);
            }

            if(ret == kAppIfSuccessful)
            {
                // Register PDO process function
                stream_registerSyncCb(pdo_process);

                // Copy buffer id to local instance
                pdoInstance_l.rpdoId_m = pPdoInitParam_p->buffIdRpdo_m;
                pdoInstance_l.tpdoId_m = pPdoInitParam_p->buffIdTpdo_m;

                // Remember process PDO user callback
                pdoInstance_l.pfnPdoCb_m = pfnPdoCb_p;
            }
        }
        else
        {
            ret = kAppIfPdoInitError;
        }
    }

    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Cleanup pdo module

\ingroup module_pdo
*/
//------------------------------------------------------------------------------
void pdo_exit(void)
{
    // Free module internals
}

//============================================================================//
//            P R I V A T E   F U N C T I O N S                               //
//============================================================================//
/// \name Private Functions
/// \{

//------------------------------------------------------------------------------
/**
\brief    Initialize the RPDO buffer image

\param[in] rpdoId_p               Id of the RPDO buffer

\return tAppIfStatus
\retval kAppIfSuccessful               On success
\retval kAppIfStreamInvalidBuffer      Invalid buffer! Can't register
\retval kAppIfRpdoBufferSizeMismatch   Size of the buffer is invalid

\ingroup module_pdo
*/
//------------------------------------------------------------------------------
static tAppIfStatus pdo_initRpdoBuffer(tTbufNumLayout rpdoId_p)
{
    tAppIfStatus ret = kAppIfSuccessful;
    tBuffDescriptor* pDescRpdo;

    ret = stream_getBufferParam(rpdoId_p, &pDescRpdo);
    if(ret == kAppIfSuccessful)
    {
        if(pDescRpdo->buffSize_m == sizeof(tTbufRpdoImage))
        {
            // Remember buffer address for later usage
            pdoInstance_l.pRpdoLayout_m = (tTbufRpdoImage *)pDescRpdo->pBuffBase_m;

            // Register rpdo acknowledge action
            ret = stream_registerAction(kStreamActionPre, rpdoId_p,
                    pdo_processRpdo, NULL);
        }
        else
        {
            ret = kAppIfRpdoBufferSizeMismatch;
        }
    }

    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Initialize the TPDO buffer image

\param[in] tpdoId_p               Id of the TPDO buffer

\return tAppIfStatus
\retval kAppIfSuccessful               On success
\retval kAppIfStreamInvalidBuffer      Invalid buffer! Can't register
\retval kAppIfRpdoBufferSizeMismatch   Size of the buffer is invalid

\ingroup module_pdo
*/
//------------------------------------------------------------------------------
static tAppIfStatus pdo_initTpdoBuffer(tTbufNumLayout tpdoId_p)
{
    tAppIfStatus ret = kAppIfSuccessful;
    tBuffDescriptor* pDescTpdo;

    ret = stream_getBufferParam(tpdoId_p, &pDescTpdo);
    if(ret == kAppIfSuccessful)
    {
        if(pDescTpdo->buffSize_m == sizeof(tTbufTpdoImage))
        {
            // Remember buffer address for later usage
            pdoInstance_l.pTpdoLayout_m = (tTbufTpdoImage *)pDescTpdo->pBuffBase_m;

            // Register rpdo acknowledge action
            ret = stream_registerAction(kStreamActionPost, tpdoId_p,
                    pdo_ackTpdo, NULL);
        }
        else
        {
            ret = kAppIfRpdoBufferSizeMismatch;
        }
    }

    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Process the PDO user callback function

\return tAppIfStatus
\retval kAppIfSuccessful        On success
\retval Other                   User defined error occurred

\ingroup module_pdo
*/
//------------------------------------------------------------------------------
static tAppIfStatus pdo_process(void)
{
    tAppIfStatus ret = kAppIfSuccessful;

    // Call the PDO user callback function
    if(pdoInstance_l.rpdoId_m == 0)
    {
        // The Rpdo buffer is not initialized
        ret = pdoInstance_l.pfnPdoCb_m(pdoInstance_l.rpdoRelTimeLow_m,
                    NULL,
                    &pdoInstance_l.pTpdoLayout_m->mappedObjList_m);
    }
    else if(pdoInstance_l.tpdoId_m == 0)
    {
        // The Tpdo buffer is not initialized
        ret = pdoInstance_l.pfnPdoCb_m(pdoInstance_l.rpdoRelTimeLow_m,
                    &pdoInstance_l.pRpdoLayout_m->mappedObjList_m,
                    NULL);
    }
    else
    {
        // Both PDO buffers are initialized
        ret = pdoInstance_l.pfnPdoCb_m(pdoInstance_l.rpdoRelTimeLow_m,
                    &pdoInstance_l.pRpdoLayout_m->mappedObjList_m,
                    &pdoInstance_l.pTpdoLayout_m->mappedObjList_m);
    }

    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Process the RPDO buffer

\param[in] pBuffer_p        Pointer to the base address of the buffer
\param[in] bufSize_p        Size of the buffer
\param[in] pUserArg_p       User defined argument

\return kAppIfSuccessful

\ingroup module_pdo
*/
//------------------------------------------------------------------------------
static tAppIfStatus pdo_processRpdo(UINT8* pBuffer_p, UINT16 bufSize_p, void * pUserArg_p)
{
    tAppIfStatus ret = kAppIfSuccessful;
    tTbufRpdoImage*  pRpdoImage;

    // Convert to configuration channel buffer structure
    pRpdoImage = (tTbufRpdoImage*) pBuffer_p;

    // Check size of buffer
    if(bufSize_p == sizeof(tTbufRpdoImage))
    {
        // Write relative time to local structure
        pdoInstance_l.rpdoRelTimeLow_m = AmiGetDwordFromLe((UINT8 *)&pRpdoImage->relativeTimeLow_m);

        stream_ackBuffer(pdoInstance_l.rpdoId_m);
    }
    else
    {
        ret = kAppIfRpdoBufferSizeMismatch;
    }

    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Acknowledge TPDO buffer

\param[in] pBuffer_p        Pointer to the base address of the buffer
\param[in] bufSize_p        Size of the buffer
\param[in] pUserArg_p       User defined argument

\return kAppIfSuccessful

\ingroup module_pdo
*/
//------------------------------------------------------------------------------
static tAppIfStatus pdo_ackTpdo(UINT8* pBuffer_p, UINT16 bufSize_p, void * pUserArg_p)
{
    tAppIfStatus ret = kAppIfSuccessful;

    stream_ackBuffer(pdoInstance_l.tpdoId_m);

    return ret;
}

/// \}


