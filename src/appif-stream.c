/**
********************************************************************************
\file   appif-stream.c

\brief  Streaming module for receiving/transmitting the input and output buffers

This module handles the transfer of the input of output buffers via the steam
handler. It enables to insert pre- and post actions before and after transfer.

\ingroup module_stream
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
 * \brief Element of buffer action list
 */
typedef struct {
    tTbufNumLayout  buffId_m;          ///< Id of the buffer
    tBuffAction     pfnBuffAction_m;   ///< Action to trigger
    void *          pUserArg_m;        ///< User argument of the action
} tBuffActionElem;

/**
 * \brief Instance of the stream module
 */
typedef struct {
    tBuffDescriptor  buffDescList_m[kTbufCount];            ///< List of buffer descriptors

    tHandlerParam    handlParam_m;                          ///< Parameters of the stream handler
    tStreamHandler   pfnStreamHandler_m;                    ///< Stream filling handler

    tBuffActionElem  buffPreActList_m[kTbufCount];          ///< List of buffer pre filling actions
    tBuffActionElem  buffPostActList_m[kTbufCount];         ///< List of buffer post filling actions

    tBuffSyncCb      pfnSyncCb_m;                           ///< Sync callback function
} tStreamInstance;

//------------------------------------------------------------------------------
// local vars
//------------------------------------------------------------------------------

static tStreamInstance streamInstance_l;

//------------------------------------------------------------------------------
// local function prototypes
//------------------------------------------------------------------------------
static tAppIfStatus stream_callActions(tActionType actType_p);
static UINT16 stream_calcImageSize(tTbufNumLayout firstId_p, tTbufNumLayout lastId_p);

//============================================================================//
//            P U B L I C   F U N C T I O N S                                 //
//============================================================================//

//------------------------------------------------------------------------------
/**
\brief    Initialize the stream module

\param[in]  pInitParam_p  Stream module initialization parameters

\return tAppIfStatus
\retval kAppIfSuccessful          On success
\retval kAppIfStreamInitError     Error while initializing the stream module

\ingroup module_stream
*/
//------------------------------------------------------------------------------
tAppIfStatus stream_init(tStreamInitParam* pInitParam_p)
{
    tAppIfStatus ret = kAppIfSuccessful;
    APPIF_MEMSET(&streamInstance_l, 0, sizeof(tStreamInstance));

#if _DEBUG
    if(pInitParam_p != NULL)
    {
        if(pInitParam_p->pBuffDescList_m == NULL    ||
           pInitParam_p->pfnStreamHandler_m == NULL  )
        {
            ret = kAppIfStreamInitError;
        }
    }
    else
    {
        ret = kAppIfStreamInitError;
    }

#endif

    if(ret == kAppIfSuccessful)
    {
        // Save the descriptor list internally
        APPIF_MEMCPY(&streamInstance_l.buffDescList_m,
                pInitParam_p->pBuffDescList_m,
                sizeof(streamInstance_l.buffDescList_m));

        // Remember handler of input output stream
        streamInstance_l.pfnStreamHandler_m = pInitParam_p->pfnStreamHandler_m;

        // Set consuming buffer handler parameter descriptor to first consuming buffer
        streamInstance_l.handlParam_m.consDesc_m.pBuffBase_m =
                streamInstance_l.buffDescList_m[pInitParam_p->idConsAck_m].pBuffBase_m;
        streamInstance_l.handlParam_m.consDesc_m.buffSize_m =
                stream_calcImageSize(pInitParam_p->idConsAck_m, pInitParam_p->idFirstProdBuffer_m);

        // Set producing buffer handler parameter descriptor to first producing buffer
        streamInstance_l.handlParam_m.prodDesc_m.pBuffBase_m =
                streamInstance_l.buffDescList_m[pInitParam_p->idFirstProdBuffer_m].pBuffBase_m;
        streamInstance_l.handlParam_m.prodDesc_m.buffSize_m =
                stream_calcImageSize(pInitParam_p->idFirstProdBuffer_m, kTbufCount);
    }

    return ret;
}

//------------------------------------------------------------------------------
/**
\brief   Destroy stream module

\ingroup module_stream
*/
//------------------------------------------------------------------------------
void stream_exit(void)
{
    // free internal structures
}

//------------------------------------------------------------------------------
/**
\brief   Get buffer parameters for id of buffer

\param[in]  buffId_p            Id of buffer
\param[out] ppBuffParam_p       Requested buffer parameters

\return tAppIfStatus
\retval kAppIfSuccessful            On success
\retval kAppIfStreamInvalidBuffer   Unable to find buffer in list

\ingroup module_stream
*/
//------------------------------------------------------------------------------
tAppIfStatus stream_getBufferParam(tTbufNumLayout buffId_p,
        tBuffDescriptor** ppBuffParam_p)
{
    tAppIfStatus ret = kAppIfSuccessful;

    if(buffId_p >= kTbufCount)
    {
        ret = kAppIfStreamInvalidBuffer;
    }
    else
    {
        *ppBuffParam_p = &streamInstance_l.buffDescList_m[buffId_p];
    }

    return ret;
}

//------------------------------------------------------------------------------
/**
\brief   Register a new action to a buffer

\param[in]  actType_p        Type of action (Pre- or post filling)
\param[in]  buffId_p         Id of the buffer for the action
\param[in]  pfnBuffAct_p     Pointer to the action function
\param[in]  pUserArg_p       User argument to pass to function

\return tAppIfStatus
\retval kAppIfSuccessful            On success
\retval kAppIfStreamInvalidBuffer   Invalid buffer! Can't register

\ingroup module_stream
*/
//------------------------------------------------------------------------------
tAppIfStatus stream_registerAction(tActionType actType_p, UINT8 buffId_p,
        tBuffAction pfnBuffAct_p, void * pUserArg_p)
{
    tAppIfStatus ret = kAppIfSuccessful;
    UINT8  fFreeFound = FALSE;
    UINT8  i;
    tBuffActionElem* buffActElem;

    if(pfnBuffAct_p != NULL)
    {
        switch(actType_p)
        {
            case kStreamActionPre:
            {
                buffActElem = &streamInstance_l.buffPreActList_m[0];
                break;
            }
            case kStreamActionPost:
            {
                buffActElem = &streamInstance_l.buffPostActList_m[0];
                break;
            }
            default:
            {
                ret = kAppIfStreamInvalidParameter;
            }
        }
    }
    else
    {
        ret = kAppIfStreamInvalidParameter;
    }

    if(ret == kAppIfSuccessful)
    {
        for(i=0; i < kTbufCount; i++, buffActElem++)
        {
            if(buffActElem->pfnBuffAction_m == NULL)
            {
                // Free element found -> Insert action
                buffActElem->buffId_m = buffId_p;
                buffActElem->pfnBuffAction_m = pfnBuffAct_p;
                buffActElem->pUserArg_m = pUserArg_p;

                fFreeFound = TRUE;
                break;
            }
        }

        if(fFreeFound == FALSE)
        {
            ret = kAppIfStreamNoFreeElementFound;
        }
    }

    return ret;
}

//------------------------------------------------------------------------------
/**
\brief   Register synchronous callback function

\param[in]  pfnSyncCb_p     Pointer to synchronous callback function

\ingroup module_stream
*/
//------------------------------------------------------------------------------
void stream_registerSyncCb(tBuffSyncCb pfnSyncCb_p)
{
    streamInstance_l.pfnSyncCb_m = pfnSyncCb_p;
}

//------------------------------------------------------------------------------
/**
\brief   Process the synchronous stream actions

This procedure starts the transfer of the local buffers and starts pre- or post
actions for each type of buffer.

\return tAppIfStatus
\retval kAppIfSuccessful            On success
\retval kAppIfStreamTransferError   Unable to transfer data

\ingroup module_stream
*/
//------------------------------------------------------------------------------
tAppIfStatus stream_processSync(void)
{
    tAppIfStatus ret = kAppIfSuccessful;

    // Call all pre filling actions
    ret = stream_callActions(kStreamActionPre);
    if(ret == kAppIfSuccessful)
    {
        // Call sync callback function
        if(streamInstance_l.pfnSyncCb_m != NULL)
        {
            ret = streamInstance_l.pfnSyncCb_m();
        }

        if(ret == kAppIfSuccessful)
        {
            // Transfer stream input/output data
            ret = streamInstance_l.pfnStreamHandler_m(&streamInstance_l.handlParam_m);
            if(ret == kAppIfSuccessful)
            {
                // Call all post filling actions
                ret = stream_callActions(kStreamActionPost);
            }
            else
            {
                ret = kAppIfStreamTransferError;
            }
        }

    }

    return ret;
}

//------------------------------------------------------------------------------
/**
\brief   Function for buffer acknowledging

\param[in]  buffId_p    If of the buffer to acknowledge

\ingroup module_stream
*/
//------------------------------------------------------------------------------
void stream_ackBuffer(UINT8 buffId_p)
{
    // Empty function! (No acknowledge needed with SPI!)
}

//============================================================================//
//            P R I V A T E   F U N C T I O N S                               //
//============================================================================//
/// \name Private Functions
/// \{

//------------------------------------------------------------------------------
/**
\brief   Call all buffer filling post actions

\param[in] actType_p               Pre- or post filling actions

\return tAppIfStatus
\retval kAppIfSuccessful       On success
\retval Other                  Module internal error on action

\ingroup module_stream
*/
//------------------------------------------------------------------------------
static tAppIfStatus stream_callActions(tActionType actType_p)
{
    UINT8 i;
    tAppIfStatus ret = kAppIfSuccessful;
    tBuffDescriptor*   bufferElem;
    tBuffActionElem* buffActElem;

    switch(actType_p)
    {
        case kStreamActionPre:
        {
            buffActElem = &streamInstance_l.buffPreActList_m[0];
            break;
        }
        case kStreamActionPost:
        {
            buffActElem = &streamInstance_l.buffPostActList_m[0];
            break;
        }
        default:
        {
            ret = kAppIfStreamInvalidParameter;
        }
    }

    if(ret == kAppIfSuccessful)
    {
        // Call buffer action for each buffer
        for(i=0; i < kTbufCount; i++, buffActElem++)
        {
            if(buffActElem->pfnBuffAction_m != NULL)
            {
                // Get buffer element by Id
                bufferElem = &streamInstance_l.buffDescList_m[buffActElem->buffId_m];

                ret = buffActElem->pfnBuffAction_m(bufferElem->pBuffBase_m,
                                                  bufferElem->buffSize_m,
                                                  buffActElem->pUserArg_m);
                if(ret != kAppIfSuccessful)
                {
                    // Error happened.. return!
                    break;
                }
            }
            else
            {
                // All actions carried out! return!
                break;
            }

        }
    }

    return ret;
}

//------------------------------------------------------------------------------
/**
\brief   Calculate size of transfer image

\param[in] firstId_p               First descriptor id of the image
\param[in] lastId_p                Last descriptor id of the image

\return UINT16
\retval Size            Size of the transfer image

\ingroup module_stream
*/
//------------------------------------------------------------------------------
static UINT16 stream_calcImageSize(tTbufNumLayout firstId_p, tTbufNumLayout lastId_p)
{
    UINT8 i;
    UINT16 imgSize = 0;

    for(i=firstId_p; i<lastId_p; i++)
    {
        imgSize += streamInstance_l.buffDescList_m[i].buffSize_m;
    }

    return imgSize;
}

/// \}


