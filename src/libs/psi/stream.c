/**
********************************************************************************
\file   stream.c

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

/*----------------------------------------------------------------------------*/
/* includes                                                                   */
/*----------------------------------------------------------------------------*/

#include <libpsi/internal/stream.h>

/*============================================================================*/
/*            G L O B A L   D E F I N I T I O N S                             */
/*============================================================================*/

/*----------------------------------------------------------------------------*/
/* const defines                                                              */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* module global vars                                                         */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* global function prototypes                                                 */
/*----------------------------------------------------------------------------*/


/*============================================================================*/
/*            P R I V A T E   D E F I N I T I O N S                           */
/*============================================================================*/

/*----------------------------------------------------------------------------*/
/* const defines                                                              */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* local types                                                                */
/*----------------------------------------------------------------------------*/

/**
 * \brief Element of buffer action list
 */
typedef struct {
    tTbufNumLayout  buffId_m;          /**< Id of the buffer */
    tBuffAction     pfnBuffAction_m;   /**< Action to trigger */
    void *          pUserArg_m;        /**< User argument of the action */
} tBuffActionElem;

/**
 * \brief Instance of the stream module
 */
typedef struct {
    tBuffDescriptor  buffDescList_m[kTbufCount];            /**< List of buffer descriptors */

    tHandlerParam    handlParam_m;                          /**< Parameters of the stream handler */
    tStreamHandler   pfnStreamHandler_m;                    /**< Stream filling handler */

    tBuffActionElem  buffPreActList_m[kTbufCount];          /**< List of buffer pre filling actions */
    tBuffActionElem  buffPostActList_m[kTbufCount];         /**< List of buffer post filling actions */

    tBuffSyncCb      pfnSyncCb_m;                           /**< Sync callback function */
} tStreamInstance;

/*----------------------------------------------------------------------------*/
/* local vars                                                                 */
/*----------------------------------------------------------------------------*/

static tStreamInstance streamInstance_l;

/*----------------------------------------------------------------------------*/
/* local function prototypes                                                  */
/*----------------------------------------------------------------------------*/
static BOOL stream_callActions(tActionType actType_p);
static UINT16 stream_calcImageSize(tTbufNumLayout firstId_p, tTbufNumLayout lastId_p);
static tBuffActionElem* stream_getActionList(tActionType actType_p);
static BOOL stream_callSyncCb(void);

/*============================================================================*/
/*            P U B L I C   F U N C T I O N S                                 */
/*============================================================================*/

/*----------------------------------------------------------------------------*/
/**
\brief    Initialize the stream module

\param[in]  pInitParam_p  Stream module initialization parameters

\return BOOL
\retval TRUE      On successful initialization
\retval FALSE     Unable to initialize the stream module

\ingroup module_stream
*/
/*----------------------------------------------------------------------------*/
BOOL stream_init(tStreamInitParam* pInitParam_p)
{
    BOOL fReturn = FALSE;
    PSI_MEMSET(&streamInstance_l, 0, sizeof(tStreamInstance));

    if(pInitParam_p == NULL)
    {
        error_setError(kPsiModuleStream, kPsiStreamInitError);
    }
    else
    {
        if(pInitParam_p->pBuffDescList_m == NULL    ||
           pInitParam_p->pfnStreamHandler_m == NULL  )
        {
            error_setError(kPsiModuleStream, kPsiStreamInitError);
        }
        else
        {
            /* Save the descriptor list internally */
            PSI_MEMCPY(&streamInstance_l.buffDescList_m,
                    pInitParam_p->pBuffDescList_m,
                    sizeof(streamInstance_l.buffDescList_m));

            /* Remember handler of input output stream */
            streamInstance_l.pfnStreamHandler_m = pInitParam_p->pfnStreamHandler_m;

            /* Set consuming buffer handler parameter descriptor to first consuming buffer */
            streamInstance_l.handlParam_m.consDesc_m.pBuffBase_m =
                    streamInstance_l.buffDescList_m[pInitParam_p->idConsAck_m].pBuffBase_m;
            streamInstance_l.handlParam_m.consDesc_m.buffSize_m =
                    stream_calcImageSize(pInitParam_p->idConsAck_m, pInitParam_p->idFirstProdBuffer_m);

            /* Set producing buffer handler parameter descriptor to first producing buffer */
            streamInstance_l.handlParam_m.prodDesc_m.pBuffBase_m =
                    streamInstance_l.buffDescList_m[pInitParam_p->idFirstProdBuffer_m].pBuffBase_m;
            streamInstance_l.handlParam_m.prodDesc_m.buffSize_m =
                    stream_calcImageSize(pInitParam_p->idFirstProdBuffer_m, kTbufCount);

            fReturn = TRUE;
        }
    }

    return fReturn;
}

/*----------------------------------------------------------------------------*/
/**
\brief   Destroy stream module

\ingroup module_stream
*/
/*----------------------------------------------------------------------------*/
void stream_exit(void)
{
    /* free internal structures */
}

/*----------------------------------------------------------------------------*/
/**
\brief   Get buffer parameters for id of buffer

\param[in]  buffId_p            Id of buffer

\return tBuffDescriptor*
\retval Address                 Pointer to buffer descriptor parameters
\retval NULL                    Unable to find buffer in list

\ingroup module_stream
*/
/*----------------------------------------------------------------------------*/
tBuffDescriptor* stream_getBufferParam(tTbufNumLayout buffId_p)
{
    tBuffDescriptor* buffDesc = NULL;

    if(buffId_p < kTbufCount)
    {
        buffDesc = &streamInstance_l.buffDescList_m[buffId_p];
    }

    return buffDesc;
}

/*----------------------------------------------------------------------------*/
/**
\brief   Register a new action to a buffer

\param[in]  actType_p        Type of action (Pre- or post filling)
\param[in]  buffId_p         Id of the buffer for the action
\param[in]  pfnBuffAct_p     Pointer to the action function
\param[in]  pUserArg_p       User argument to pass to function

\return BOOL
\retval TRUE         Successfully registered action to buffer
\retval FALSE        Invalid buffer! Can't register

\ingroup module_stream
*/
/*----------------------------------------------------------------------------*/
BOOL stream_registerAction(tActionType actType_p, UINT8 buffId_p,
        tBuffAction pfnBuffAct_p, void * pUserArg_p)
{
    BOOL fReturn = FALSE;
    UINT8  i;
    tBuffActionElem* pBuffActElem;

    if(pfnBuffAct_p == NULL)
    {
        error_setError(kPsiModuleStream, kPsiStreamInvalidParameter);
    }
    else
    {
        pBuffActElem = stream_getActionList(actType_p);
        if(pBuffActElem == NULL)
        {
            error_setError(kPsiModuleStream, kPsiStreamInvalidParameter);
        }
        else
        {
            for(i=0; i < kTbufCount; i++, pBuffActElem++)
            {
                if(pBuffActElem->pfnBuffAction_m == NULL)
                {
                    /* Free element found -> Insert action */
                    pBuffActElem->buffId_m = buffId_p;
                    pBuffActElem->pfnBuffAction_m = pfnBuffAct_p;
                    pBuffActElem->pUserArg_m = pUserArg_p;

                    fReturn = TRUE;
                    break;
                }
            }

            if(fReturn == FALSE)
            {
                /* Set error when list is full */
                error_setError(kPsiModuleStream, kPsiStreamNoFreeElementFound);
            }
        }
    }

    return fReturn;
}

/*----------------------------------------------------------------------------*/
/**
\brief   Register synchronous callback function

\param[in]  pfnSyncCb_p     Pointer to synchronous callback function

\ingroup module_stream
*/
/*----------------------------------------------------------------------------*/
void stream_registerSyncCb(tBuffSyncCb pfnSyncCb_p)
{
    streamInstance_l.pfnSyncCb_m = pfnSyncCb_p;
}

/*----------------------------------------------------------------------------*/
/**
\brief   Process the synchronous stream actions

This procedure starts the transfer of the local buffers and starts pre- or post
actions for each type of buffer.

\return BOOL
\retval TRUE      Successfully processed the synchronous task
\retval FALSE     Unable to transfer data or call user action

\ingroup module_stream
*/
/*----------------------------------------------------------------------------*/
BOOL stream_processSync(void)
{
    BOOL fReturn = FALSE;

    /* Call all pre filling actions */
    if(stream_callActions(kStreamActionPre) != FALSE)
    {
        /* Call synchronization function handler */
        if(stream_callSyncCb() != FALSE)
        {
            /* Transfer stream input/output data */
            if(streamInstance_l.pfnStreamHandler_m(&streamInstance_l.handlParam_m) != FALSE)
            {
                fReturn = TRUE;
            }
            else
            {
                /* Stream handler error handler */
                error_setError(kPsiModuleStream, kPsiStreamTransferError);
            }
        }
    }

    return fReturn;
}

/*----------------------------------------------------------------------------*/
/**
\brief   Process the stream module post transfer actions

This procedure triggers all post actions of the libpsi. A post action
are all tasks which are after the exchange of the input/output image.

\return BOOL
\retval TRUE      Successfully processed the post actions
\retval FALSE     Unable to process post actions

\ingroup module_stream
*/
/*----------------------------------------------------------------------------*/
BOOL stream_processPostActions(void)
{
    BOOL fReturn = FALSE;

    /* Call all post filling actions */
    if(stream_callActions(kStreamActionPost) != FALSE)
    {
        fReturn = TRUE;
    }

    return fReturn;
}

/*----------------------------------------------------------------------------*/
/**
\brief   Function for buffer acknowledging

\param[in]  buffId_p    If of the buffer to acknowledge

\ingroup module_stream
*/
/*----------------------------------------------------------------------------*/
void stream_ackBuffer(UINT8 buffId_p)
{
    UNUSED_PARAMETER(buffId_p);

    /* Empty function! (No acknowledge needed with SPI!) */
}

/*============================================================================*/
/*            P R I V A T E   F U N C T I O N S                               */
/*============================================================================*/
/* \name Private Functions */
/* \{ */

/*----------------------------------------------------------------------------*/
/**
\brief   Call all buffer filling post actions

\param[in] actType_p               Pre- or post filling actions

\return BOOL
\retval TRUE         Successfully called all buffer actions
\retval FALSE        Error while processing a buffer action

\ingroup module_stream
*/
/*----------------------------------------------------------------------------*/
static BOOL stream_callActions(tActionType actType_p)
{
    BOOL fReturn = FALSE, fRetAct;
    UINT8 i;
    tBuffDescriptor* pBuffElement;
    tBuffActionElem* pBuffActList;

    pBuffActList = stream_getActionList(actType_p);

    /* Call buffer action for each buffer */
    for(i=0; i < kTbufCount; i++, pBuffActList++)
    {
        if(pBuffActList->pfnBuffAction_m != NULL)
        {
            /* Get buffer element by Id */
            pBuffElement = &streamInstance_l.buffDescList_m[pBuffActList->buffId_m];

            fRetAct = pBuffActList->pfnBuffAction_m(pBuffElement->pBuffBase_m,
                                               pBuffElement->buffSize_m,
                                               pBuffActList->pUserArg_m);
            if(fRetAct == FALSE)
            {
                /* Error happened.. return! */
                error_setError(kPsiModuleStream, kPsiStreamProcessActionFailed);

                break;
            }
        }
        else
        {
            /* All set actions carried out! return! */
            fReturn = TRUE;
            break;
        }
    }

    if(i == kTbufCount)
    {
        /* All actions set and carried out */
        fReturn = TRUE;
    }

    return fReturn;
}

/*----------------------------------------------------------------------------*/
/**
\brief   Calculate size of transfer image

\param[in] firstId_p               First descriptor id of the image
\param[in] lastId_p                Last descriptor id of the image

\return UINT16
\retval Size            Size of the transfer image

\ingroup module_stream
*/
/*----------------------------------------------------------------------------*/
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

/*----------------------------------------------------------------------------*/
/**
\brief   Get action list for action type

\param[in] actType_p               Type of the action

\return tBuffActionElem
\retval Address            Pointer to the action list
\retval Null               Invalid action type for action list

\ingroup module_stream
*/
/*----------------------------------------------------------------------------*/
static tBuffActionElem* stream_getActionList(tActionType actType_p)
{
    tBuffActionElem* pBuffActElem = NULL;

    switch(actType_p)
    {
        case kStreamActionPre:
        {
            pBuffActElem = &streamInstance_l.buffPreActList_m[0];
            break;
        }
        case kStreamActionPost:
        {
            pBuffActElem = &streamInstance_l.buffPostActList_m[0];
            break;
        }
        default:
        {
            /* error occurred */
            break;
        }
    }

    return pBuffActElem;
}

/*----------------------------------------------------------------------------*/
/**
\brief   Call the sync callback if initialized

\return BOOL
\retval TRUE           Success on calling the synchronization callback
\retval FALSE          Error will calling the synchronization callback

\ingroup module_stream
*/
/*----------------------------------------------------------------------------*/
static BOOL stream_callSyncCb(void)
{
    BOOL fReturn = FALSE;

    if(streamInstance_l.pfnSyncCb_m != NULL)
    {
        /* Call synchronization callback function */
        if(streamInstance_l.pfnSyncCb_m() != FALSE)
        {
            fReturn = TRUE;
        }
        else
        {
            error_setError(kPsiModuleStream, kPsiStreamSyncError);
        }
    }
    else
    {
        fReturn = TRUE;
    }

    return fReturn;
}

/* \} */
