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

#include <appif-tbuflayout.h>

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
 * \brief Descriptor element of descriptor list
 */
typedef struct {
    UINT8*     pBuffBase_m;    ///< Base address of the buffer
    UINT16     buffSize_m;     ///< Size of the buffer
} tBuffDescriptor;

/**
 * \brief Element of buffer action list
 */
typedef struct {
    UINT8        buffId_m;          ///< Id of the buffer
    tBuffAction  pfnBuffAction_m;   ///< Action to trigger
    void *       pUserArg_m;        ///< User argument of the action
} tBuffActionElem;

/**
 * \brief Instance of the stream module
 */
typedef struct {
    UINT8            countConsBuff_m;                       ///< Count of consuming buffers
    UINT8            countProdBuff_m;                       ///< Count of producing buffers

    tBuffDescriptor  buffDescList_m[kTbufCount];            ///< List of buffer descriptors
    tBuffSegDesc     buffSegDescList_m[kTbufCount + 1];     ///< List of segment buffer descriptors (with init descriptor)
    UINT8            countSegDesc_m;                        ///< Count of segmented descriptors in list
    UINT8            dummyBuffer_m[4];                      ///< Dummy buffer of descriptor list

    tHandlerParam    handlParam_m;                          ///< Parameters of the stream handler
    tStreamHandler   pfnStreamHandler_m;                    ///< Stream filling handler
    tAppIfCritSec    pfnEnterCritSec_m;                     ///< Pointer to critical section entry function

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
static void stream_genSegmDescList(tBuffDescriptor* pBuffDescList_p,
        tBuffSegDesc* pBuffSegDescList_p, UINT8* countDescSeg_p,
        UINT8 firstConsId_p, UINT8 firstProdId_p);

//============================================================================//
//            P U B L I C   F U N C T I O N S                                 //
//============================================================================//

//------------------------------------------------------------------------------
/**
\brief    Initialize the stream module

\param[in]  pInitParam_p  Stream module initialization parameters

\return tAppIfStatus
\retval kAppIfSuccessful          On success

\ingroup module_stream
*/
//------------------------------------------------------------------------------
tAppIfStatus stream_init(tStreamInitParam* pInitParam_p)
{
    tAppIfStatus ret = kAppIfSuccessful;

    APPIF_MEMSET(&streamInstance_l, 0, sizeof(tStreamInstance));

    if(pInitParam_p->pBuffDescList_m != NULL )
    {
        ret = kAppIfStreamInitError;
        goto Exit;
    }

    if(pInitParam_p->pfnStreamHandler_m == NULL ||
       pInitParam_p->pfnEnterCritSec_m == NULL   )
    {
        ret = kAppIfStreamInitError;
        goto Exit;
    }

    // Remember handler of input output stream
    streamInstance_l.pfnStreamHandler_m = pInitParam_p->pfnStreamHandler_m;

    // Remember handler of critical section
    streamInstance_l.pfnEnterCritSec_m = pInitParam_p->pfnEnterCritSec_m;

    // Save count of producing and consuming buffers
    streamInstance_l.countConsBuff_m = pInitParam_p->countConsBuff_m;
    streamInstance_l.countProdBuff_m = pInitParam_p->countProdBuff_m;

Exit:
    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Finish the initialization of the modules

\return kAppIfSuccessful

\ingroup module_stream
*/
//------------------------------------------------------------------------------
tAppIfStatus stream_finishModuleInit(void)
{
    tAppIfStatus ret = kAppIfSuccessful;
    // Count of consuming buffers with the ACK register
    UINT8        countConsAck = streamInstance_l.countConsBuff_m + 1;

    // Initialize dummy descriptor for SPI slave initialization
    streamInstance_l.buffSegDescList_m[0].pBuffTxBase_m = (UINT8 *)&streamInstance_l.dummyBuffer_m;
    streamInstance_l.buffSegDescList_m[0].pBuffRxBase_m = NULL;
    streamInstance_l.buffSegDescList_m[0].transSize_m = sizeof(streamInstance_l.dummyBuffer_m);

    streamInstance_l.countSegDesc_m++;

    // Generate segmented descriptor list
    stream_genSegmDescList(&streamInstance_l.buffDescList_m[0],
            &streamInstance_l.buffSegDescList_m[1], &streamInstance_l.countSegDesc_m,
            0, countConsAck);

    // Initialize stream handler parameter
    streamInstance_l.handlParam_m.pFirstSegDesc_m = &streamInstance_l.buffSegDescList_m[0];
    streamInstance_l.handlParam_m.segDescCount_m = streamInstance_l.countSegDesc_m;

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
\brief   Register a new buffer to the stream module

\param[in]  pBuffParam_p        Parameters of the buffer to register

\return tAppIfStatus
\retval kAppIfSuccessful            On success
\retval kAppIfStreamInvalidBuffer   Invalid buffer! Can't register

\ingroup module_stream
*/
//------------------------------------------------------------------------------
tAppIfStatus stream_registerBuffer(tBuffParam* pBuffParam_p )
{
    tAppIfStatus ret = kAppIfSuccessful;
    tBuffDescriptor* buffer;

    if(pBuffParam_p->buffId_m >= kTbufCount)
    {
        ret = kAppIfStreamInvalidBuffer;
        goto Exit;
    }

    if(pBuffParam_p->pBuffBase_m == NULL ||
       pBuffParam_p->buffSize_m == 0      )
    {
        ret = kAppIfStreamInvalidBuffer;
        goto Exit;
    }

    buffer = &streamInstance_l.buffDescList_m[pBuffParam_p->buffId_m];

    buffer->pBuffBase_m = pBuffParam_p->pBuffBase_m;
    buffer->buffSize_m = pBuffParam_p->buffSize_m;

Exit:
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

    if(pfnBuffAct_p == NULL)
    {
        ret = kAppIfStreamInvalidParameter;
        goto Exit;
    }

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
            goto Exit;
        }
    }

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

Exit:
    return ret;
}


void stream_registerSyncCb(tBuffSyncCb pfnSyncCb_p)
{
    streamInstance_l.pfnSyncCb_m = pfnSyncCb_p;
}

//------------------------------------------------------------------------------
/**
\brief   Update a buffer address in the stream module

\param[in]  buffId_p            Id of the buffer to register
\param[in]  pBuffBase_p         New base address of the buffer to register

\return tAppIfStatus
\retval kAppIfSuccessful            On success
\retval kAppIfStreamInvalidBuffer   Invalid buffer! Can't register

\ingroup module_stream
*/
//------------------------------------------------------------------------------
tAppIfStatus stream_updateBufferBase(UINT8 buffId_p, UINT8* pBuffBase_p)
{
    tAppIfStatus ret = kAppIfSuccessful;
    UINT8        countConsAck = streamInstance_l.countConsBuff_m + 1;

    if(buffId_p >= kTbufCount)
    {
        ret = kAppIfStreamInvalidBuffer;
        goto Exit;
    }

    if(pBuffBase_p == NULL)
    {
        ret = kAppIfStreamInvalidBuffer;
        goto Exit;
    }

    streamInstance_l.buffDescList_m[buffId_p].pBuffBase_m = pBuffBase_p;

    // Reset segmented descriptor count
    streamInstance_l.countSegDesc_m = 1;

    // Update segmented descriptor list because of new buffer base
    stream_genSegmDescList(&streamInstance_l.buffDescList_m[0],
            &streamInstance_l.buffSegDescList_m[1], &streamInstance_l.countSegDesc_m,
            0, countConsAck);

Exit:
    return ret;
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
    if(ret != kAppIfSuccessful)
    {
        goto Exit;
    }

    // Call sync callback function
    if(streamInstance_l.pfnSyncCb_m != NULL)
    {
        ret = streamInstance_l.pfnSyncCb_m();
        if(ret != kAppIfSuccessful)
        {
            goto Exit;
        }
    }

    // Transfer stream input/output data
    ret = streamInstance_l.pfnStreamHandler_m(&streamInstance_l.handlParam_m);
    if(ret != kAppIfSuccessful)
    {
        ret = kAppIfStreamTransferError;
        goto Exit;
    }

    // Call all post filling actions
    ret = stream_callActions(kStreamActionPost);
    if(ret != kAppIfSuccessful)
    {
        goto Exit;
    }

Exit:
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
            goto Exit;
        }
    }

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

Exit:
    return ret;
}

//------------------------------------------------------------------------------
/**
\brief   Generate segmented descriptor list

Splits the buffers into small segments which can be transfered via SPI
in full duplex mode.

\param[in] pBuffDescList_p              List of unsegmented descriptors
\param[out] pBuffSegDescList_p          List of segmented descriptors
\param[out] pCountDescSeg_p             Count of segmented descriptors
\param[in] firstConsId_p                Id of the first consuming buffer
\param[in] firstProdId_p                Id of the first producing buffer

\ingroup module_stream
*/
//------------------------------------------------------------------------------
static void stream_genSegmDescList(tBuffDescriptor* pBuffDescList_p,
        tBuffSegDesc* pBuffSegDescList_p, UINT8* pCountDescSeg_p,
        UINT8 firstConsId_p, UINT8 firstProdId_p)
{
    tBuffDescriptor  emptyDesc;
    tBuffDescriptor* pCurrConsDesc;
    tBuffDescriptor* pCurrProdDesc;
    UINT8   consCurrId = firstConsId_p;
    UINT8   prodCurrId = firstProdId_p;
    UINT8*  pCurrConsAddr;
    UINT8*  pCurrProdAddr;
    UINT16  prodTransfered = 0;
    UINT16  consTransfered = 0;
    UINT16  transfSize;

    // Init empty descriptor
    emptyDesc.pBuffBase_m = NULL;
    emptyDesc.buffSize_m = 0xFFFF;

    // Enter critical section
    streamInstance_l.pfnEnterCritSec_m(FALSE);

    // Loop over all buffer segments
    while(consCurrId < firstProdId_p ||
          prodCurrId < kTbufCount  )
    {
        if(prodCurrId >= kTbufCount)
            pCurrProdDesc = &emptyDesc;
        else
            pCurrProdDesc = pBuffDescList_p + prodCurrId;

        if(consCurrId >= firstProdId_p)
            pCurrConsDesc = &emptyDesc;
        else
            pCurrConsDesc = pBuffDescList_p + consCurrId;

        // Set address to read and write pointers
        pCurrProdAddr = pCurrProdDesc->pBuffBase_m + prodTransfered;
        pCurrConsAddr = pCurrConsDesc->pBuffBase_m + consTransfered;

        // Get transfer size
        if( (pCurrProdDesc->buffSize_m - prodTransfered) >
            (pCurrConsDesc->buffSize_m - consTransfered) )
        {
            transfSize = pCurrConsDesc->buffSize_m - consTransfered;
        }
        else
        {
            transfSize = pCurrProdDesc->buffSize_m - prodTransfered;
        }

        // Set result to segment descriptor list
        pBuffSegDescList_p->pBuffRxBase_m = pCurrConsAddr;
        pBuffSegDescList_p->pBuffTxBase_m = pCurrProdAddr;
        pBuffSegDescList_p->transSize_m = transfSize;

        // Forward to next element
        pBuffSegDescList_p++;
        (*pCountDescSeg_p)++;

        // Forward id and size to next segment
        if( (pCurrProdDesc->buffSize_m - prodTransfered) >
            (pCurrConsDesc->buffSize_m - consTransfered) )
        {
            // Producing buffer is bigger -> Jump to next consuming buffer!
            prodTransfered += transfSize;
            consTransfered = 0;
            consCurrId++;
        }
        else if ((pCurrProdDesc->buffSize_m - prodTransfered) <
                (pCurrConsDesc->buffSize_m - consTransfered) )
        {
            // Consuming buffer is bigger -> Jump to next producing buffer!
            consTransfered += transfSize;
            prodTransfered = 0;
            prodCurrId++;
        }
        else
        {
            // Both buffers have same size -> jump to next segment
            prodTransfered = 0;
            prodCurrId++;
            consTransfered = 0;
            consCurrId++;
        }
    }   // end of while

    // Leave critical section
    streamInstance_l.pfnEnterCritSec_m(TRUE);
}

/// \}


