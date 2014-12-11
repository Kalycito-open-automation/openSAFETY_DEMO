/**
********************************************************************************
\file   shnf.c

\brief  Safety hardware near firmware managing module

This module provides implements the interface to the underlying fieldbus.
It forwards receive frames to the stack and transports the transmit frames
to the HNF. Also the frame CRCs are calculated inside this module.

\ingroup module_shnf
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
#include <shnf/shnf.h>

#include <shnf/hnf.h>
#include <shnf/statehandler.h>

#include <SODapi.h>
#include <SNMTSapi.h>
#include <SSCapi.h>
#include <SPDOapi.h>
#include <SCFMapi.h>
#include <SHNF.h>

#include <oschecksum/crc.h>

/*============================================================================*/
/*            G L O B A L   D E F I N I T I O N S                             */
/*============================================================================*/

/*----------------------------------------------------------------------------*/
/* const defines                                                              */
/*----------------------------------------------------------------------------*/

/* Count of transmit channels (0=Sync0; 1=Async0) */
#define TX_CHANNEL_COUNT            2

#define TX_CHANNEL_SPDO             0
#define TX_CHANNEL_SSDO_SNMT        1

/* Positions of fields inside a frame */
#define FRAME_OFFSET_ADRID      0x1
#define FRAME_OFFSET_LENGTH     0x2

/* Bitmasks to decode a safety frame */
#define ID_FRAME_MASK               0xE0    /**< Mask to identify a frame in  the id field (Bit: 5, 6, 7) */
#define ID_SSDO_SLIM_MASK           0xF8    /**< Mask to identify a SSDO SLIM frame (Bit: 3, 4, 5, 6, 7) */

/* Id's to identify a safety frame by it's ID */
#define SPDO_FRAME_TYPE        0xC0
#define SNMT_FRAME_TYPE        0xA0
#define SSDO_FRAME_TYPE        0xE0
#define SSDOSLIM_FRAME_TYPE    0xE8

/* Position of subframe 1 in slim frame */
#define SLIM_FRAME_SUB1_POS_CRC8     6
#define SLIM_FRAME_SUB1_POS_CRC16    7

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
 * \brief SSDO/SNMT receive states
 */
typedef enum
{
    kSsdoRxStatusInvalid = 0x0,
    kSsdoRxStatusReady   = 0x1,
    kSsdoRxStatusBusy    = 0x2,
} tSsdoSnmtRxStatus;

typedef struct
{
    UINT16 frameLen_m;                            /**< Length of the frame */
    UINT8 txBuffer_m[SSC_k_MAX_TEL_LEN_LONG];     /**< Temporary buffer of the frame to fill */
    BOOLEAN isSlim;                               /**< True if the frame is a SSDO slim frame */
} tTxDescriptor;

/**
 * \brief Module instance structure
 */
typedef struct
{
    tSsdoSnmtRxStatus  ssdoRxStatus_m;                  /**< Status of the receive state machine */
    tTxDescriptor      txDesc_m[TX_CHANNEL_COUNT];      /**< Transmit descriptor of each transmit channel */
} tShnfInstance;

static tShnfInstance shnfInstance_l SAFE_INIT_SEKTOR;

/**
 * @var SHNF_aaulConnValidBit
 * @brief This variable contains the connection valid bit of the SPDOs.
 *
 * It is a bit field which contains one bit each RxSPDO. It is to be hosted by the SHNF.
 */
UINT32 SHNF_aaulConnValidBit[EPLS_cfg_MAX_INSTANCES][(SPDO_cfg_MAX_NO_RX_SPDO + 31) / 32];

/*----------------------------------------------------------------------------*/
/* local vars                                                                 */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* local function prototypes                                                  */
/*----------------------------------------------------------------------------*/
static void buildTxSpdoFrame(void);
static BOOLEAN processRxSsdoSnmtFrame(UINT8* pPayload_p, UINT16 paylLen_p);
static void processRxSpdoFrame(UINT8* pPayload_p, UINT16 paylLen_p);

static UINT16 getFrameLength(const UINT8 * pPaylLen_p);
static BOOLEAN prepareTransmitFrame(UINT8 * pTargBuffer_p, UINT16 targBuffLen_p,
                                   UINT8 * pSrcBuffer_p, UINT16 srcBuffLen_p,
                                   BOOLEAN fIsSlim_p);

/*============================================================================*/
/*            P U B L I C   F U N C T I O N S                                 */
/*============================================================================*/

/*----------------------------------------------------------------------------*/
/**
\brief    Initialize the SHNF managing module

\param[in] pInitParam_p    Pointer to the init parameters

The hardware near firmware provides the interface to the underlying
black channel.

\retval TRUE    Init of the SHNF successfully
\retval FALSE   Error on initialization

\ingroup module_shnf
*/
/*----------------------------------------------------------------------------*/
BOOLEAN shnf_init(tShnfInitParam * pInitParam_p)
{
    BOOLEAN fReturn = FALSE;
    tHnfInit hnfInitParam;

    MEMSET(&shnfInstance_l, 0, sizeof(tShnfInstance));
    MEMSET(&hnfInitParam, 0, sizeof(tHnfInit));

    if(pInitParam_p != NULL)
    {
        if(pInitParam_p->pfnProcSync_m != NULL  &&
           pInitParam_p->pfnSyncronize_m != NULL )
        {
            /* Initialize the local instance structure */
            shnfInstance_l.ssdoRxStatus_m = kSsdoRxStatusReady;

            /* Setup the HNF initialization parameters */
            hnfInitParam.asyncRcvChan0Handler_m = processRxSsdoSnmtFrame;
            hnfInitParam.syncRcvHandler_m = processRxSpdoFrame;
            hnfInitParam.syncTxBuild_m = buildTxSpdoFrame;
            hnfInitParam.pfnProcSync_m = pInitParam_p->pfnProcSync_m;
            hnfInitParam.pfnSyncronize_m = pInitParam_p->pfnSyncronize_m;

            /* Initialize the slim interface HNF */
            if(hnf_init(&hnfInitParam))
            {
                /* Initialize the consecutive time module */
                fReturn = TRUE;
            }
        }
        else
        {
            errh_postFatalError(kErrSourceShnf, kErrorInvalidParameter, 0);
        }
    }
    else
    {
        errh_postFatalError(kErrSourceShnf, kErrorInvalidParameter, 0);
    }

    return fReturn;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Shutdown the SHNF managing module

\ingroup module_shnf
*/
/*----------------------------------------------------------------------------*/
void shnf_exit(void)
{
    hnf_exit();
}

/*----------------------------------------------------------------------------*/
/**
\brief    Reset all SHNF internals

\note This function is called on a cycle time violation

\ingroup module_shnf
*/
/*----------------------------------------------------------------------------*/
void shnf_reset(void)
{
    /* Reset the connection valid bit field */
    MEMSET(&SHNF_aaulConnValidBit[B_INSTNUMidx][0],0,sizeof(SHNF_aaulConnValidBit[B_INSTNUMidx]));
}

/*----------------------------------------------------------------------------*/
/**
\brief    Process the SHNF background task

Needs to be called periodically and processes the asynchronous data of the
hardware near firmware.

\ingroup module_shnf
*/
/*----------------------------------------------------------------------------*/
BOOLEAN shnf_process(void)
{
    BOOLEAN fReturn = FALSE;
    UINT32 consTime;
    SSC_t_PROCESS ssdoProcRet;
    /* number of free management frames can be sent per call of the SSC_BuildTxFrames */
    UINT8 freeMngtFrmsCount = 1U;

    switch(shnfInstance_l.ssdoRxStatus_m)
    {
        case kSsdoRxStatusReady:
        {
            if(hnf_processAsync())
            {
                if(stateh_getSnState() == kSnStateOperational)
                {
                    consTime = constime_getTime();

                    /* Guard timeout is checked in operational cyclically */
                    SNMTS_TimerCheck(B_INSTNUM_ consTime, &freeMngtFrmsCount);
                }
                fReturn = TRUE;
            }
            break;
        }
        case kSsdoRxStatusBusy:
        {
            consTime = constime_getTime();

            /* Call SSDO/SNMT process function with null argument to continue processing */
            ssdoProcRet = SSC_ProcessSNMTSSDOFrame(B_INSTNUM_ consTime, NULL, 0);
            if(ssdoProcRet == SSC_k_OK)
            {
                hnf_finishedAsyncRxChannel0();
                shnfInstance_l.ssdoRxStatus_m = kSsdoRxStatusReady;
            }

            fReturn = TRUE;

            break;
        }
        default:
        {
            errh_postFatalError(kErrSourceShnf, kErrorInvalidState, 0);
            break;
        }
    }

    return fReturn;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Enable the synchronous interrupt

\ingroup module_shnf
*/
/*----------------------------------------------------------------------------*/
void shnf_enableSyncIr(void)
{
    hnf_enableSyncIr();
}

/*
 * This function provides a reference to an empty memory block. The memory block is
 * used to store an openSAFETY frame to be transmitted on the field bus. Frame header
 * and frame data are stored within this block. The frame data is filled in the specific
 * unit (SPDO, SNMTM, SNMTS, SSDOC, SSDOS) but the frame header is only build in unit SFS.
 *
 * After write access the memory block has to be marked as "ready to process".
 * To do so the function \see SHNF_MarkTxMemBlock has to be called.
 *
 * \see SHNF_MarkTxMemBlock
 *
 * \warning The first part of the memory block contains the openSAFETY sub-frame TWO
 *          and the second part the openSAFETY sub-frame ONE.
 *
 * \param B_INSTNUM instance number
 * \param w_blockSize size of the requested memory block in bytes
 * \param e_telType telegram type (SPDO / SSDO / SNMT)
 * \param w_txSpdoNum Number of the Tx SPDO (first SPDO number is 0). It is only
 *              relevant, if telegram type is SPDO.
 *
 * \return - == NULL  - memory allocation failed, no memory available
 *         - <> NULL  - memory allocation succeeded, reference to requested memory
 */
UINT8 * SHNF_GetTxMemBlock(BYTE_B_INSTNUM_ UINT16 w_blockSize,
                           SHNF_t_TEL_TYPE e_telType, UINT16 w_txSpdoNum)
{
    UINT8 *pResBuffer = (UINT8 *)NULL;
    tTxDescriptor * pTxDesc = (tTxDescriptor*)NULL;

#if (EPLS_cfg_MAX_INSTANCES > 1)
    UNUSED_PARAMETER(b_instNum);    /* to avoid compiler warnings */
#endif

    UNUSED_PARAMETER(w_txSpdoNum);

    /* Check if frame is synchronous */
    if(e_telType == SHNF_k_SPDO)
    {
        pTxDesc = &shnfInstance_l.txDesc_m[TX_CHANNEL_SPDO];
        /* Remember size of the frame */
        pTxDesc->frameLen_m = w_blockSize;
        /* Set return value to current buffer */
        pResBuffer = pTxDesc->txBuffer_m;
    }
    else
    {
        /* Frame needs the asynchronous buffer */
        pTxDesc = &shnfInstance_l.txDesc_m[TX_CHANNEL_SSDO_SNMT];

        if(e_telType == SHNF_k_SSDO ||
           e_telType == SHNF_k_SNMT)
        {
            /* Remember size of the frame */
            pTxDesc->frameLen_m = w_blockSize;
            /* Set return value to current buffer */
            pResBuffer = &pTxDesc->txBuffer_m[0];
            pTxDesc->isSlim = FALSE;
        }
        else if (e_telType == SHNF_k_SSDO_SLIM)
        {
            /* Remember size of the frame */
            pTxDesc->frameLen_m = w_blockSize;
            /* Set return value to current buffer */
            pResBuffer = &pTxDesc->txBuffer_m[0];
            pTxDesc->isSlim = TRUE;
        }
        else
        {
            /* Invalid telegram type -> Return NULL */
            errh_postFatalError(kErrSourceShnf, kErrorInvalidFrameType, 0);
        }
    }

    return pResBuffer;

}

/*
 * This function marks a requested memory block as "ready to process". That means all
 * necessary data and frame header info for the openSAFETY frame was written into the
 * memory block by the openSAFETY Stack and the frame can be transmitted on the field bus.
 * To get a reference to an empty memory block the function \see SHNF_GetTxMemBlock()
 * has to be called before.
 *
 * \see SHNF_GetTxMemBlock
 *
 * \warning The first part of the memory block contains the openSAFETY sub-frame TWO
 *          and the second part the openSAFETY sub-frame ONE.
 *
 * \param B_INSTNUM instance number
 * \param pb_memBlock reference to the memory to be marked
 *
 * \return - TRUE  - memory block marked successfully
 *         - FALSE - memory block NOT marked, error returned
 */
BOOLEAN SHNF_MarkTxMemBlock(BYTE_B_INSTNUM_ const UINT8 *pb_memBlock)
{
    BOOLEAN fReturn = FALSE;
    tTxDescriptor * pTxDesc = (tTxDescriptor*)NULL;
    UINT8 * pTargBuffer = (UINT8 *)NULL;
    UINT16 targBuffLen = 0;

#if (EPLS_cfg_MAX_INSTANCES > 1)
    UNUSED_PARAMETER(b_instNum); /* to avoid compiler warnings */
#endif

    if(pb_memBlock == shnfInstance_l.txDesc_m[TX_CHANNEL_SPDO].txBuffer_m)
    {
        /* Frame to transmit is a SPDO frame */
        pTxDesc = &shnfInstance_l.txDesc_m[TX_CHANNEL_SPDO];

        /* Get transmit buffer from HNF */
        if(hnf_getSyncTxBuffer(&pTargBuffer, &targBuffLen))
        {
            if(prepareTransmitFrame(pTargBuffer, targBuffLen,
                                    pTxDesc->txBuffer_m, pTxDesc->frameLen_m,
                                    FALSE))
            {
                DEBUG_TRACE(DEBUG_LVL_SHNF, "Snd TPDO\n");

                /* Forward filled buffer to HNF */
                if(hnf_postSyncTx(pTargBuffer, targBuffLen))
                {
                    fReturn = TRUE;
                }
                else
                {
                    errh_postFatalError(kErrSourceShnf, kErrorSyncFramePostingFailed, 0);
                }
            }
            else
            {
                errh_postFatalError(kErrSourceShnf, kErrorSyncFrameCopyFailed, 0);
            }
        }
        else
        {
            errh_postFatalError(kErrSourceShnf, kErrorSyncFrameNoBuffer, 0);
        }
    }
    else if(pb_memBlock == shnfInstance_l.txDesc_m[TX_CHANNEL_SSDO_SNMT].txBuffer_m)
    {
        /* Frame to transmit is an asynchronous frame */
        pTxDesc = &shnfInstance_l.txDesc_m[TX_CHANNEL_SSDO_SNMT];

        /* Get asynchronous target buffer from hnf */
        if(hnf_getAsyncTxBufferChannel0(&pTargBuffer, &targBuffLen))
        {
            /* Prepare asynchronous target buffer and swap frame */
            if(prepareTransmitFrame(pTargBuffer, targBuffLen,
                                    pTxDesc->txBuffer_m, pTxDesc->frameLen_m,
                                    pTxDesc->isSlim))
            {
                DEBUG_TRACE(DEBUG_LVL_SHNF, "Snd SSDO/SNMT\n");

                /* Post transmit frame to hnf */
                if(hnf_postAsyncTxChannel0(pTargBuffer, pTxDesc->frameLen_m))
                {
                    fReturn = TRUE;
                }
                else
                {
                    errh_postFatalError(kErrSourceShnf, kErrorAsyncFramePostingFailed, 0);
                }
            }
            else
            {
                errh_postFatalError(kErrSourceShnf, kErrorAsyncFrameCopyFailed, 0);
            }
        }
        else
        {
            /* No buffer is not a fatal problem -> Executing can continue later */
            errh_postMinorError(kErrSourceShnf, kErrorAsyncFrameNoBuffer, 0);
        }
    }
    else
    {
        /* Invalid memory block prepared for transmission */
        errh_postFatalError(kErrSourceShnf, kErrorInvalidTxMemory, 0);
    }

    return fReturn;
}

/**
 * This function returns a CRC checksum over a sub frame with payload data of
 * of maximum 8 bytes. The result is a 8 bit CRC.
 *
 * \param b_initCrc initial CRC (always called with 0 by the EPLS software)
 * \param l_subFrameLength subframe length in bytes
 * \param pv_subFrame reference to subframe
 *
 * \return 0 .. 255 - 8 bit CRC check sum
 */
UINT8 HNFiff_Crc8CalcSwp(UINT8 b_initCrc, INT32 l_subFrameLength,
                         const void *pv_subFrame)
{
    return crc8Checksum(l_subFrameLength, (UINT8*)pv_subFrame, b_initCrc);
}


/**
 * This function returns a CRC checksum over a sub frame with payload data of
 * minimum 9 bytes and maximum 254 bytes. The result is a 16 bit CRC.
 *
 * \param w_initCrc initial CRC (always called with 0 by the EPLS software)
 * \param l_subFrameLength subframe length in bytes
 * \param pv_subFrame reference to subframe
 *
 * \return 0 .. 65535 - 16 bit CRC check sum
 */
UINT16 HNFiff_Crc16CalcSwp(UINT16 w_initCrc, INT32 l_subFrameLength,
                           const void *pv_subFrame)
{
    return crc16Checksum_AC9A(l_subFrameLength, pv_subFrame, w_initCrc);
}

/**
 * This function returns a CRC checksum over a sub frame with payload data of
 * minimum 9 bytes and maximum 254 bytes. The result is a 16 bit CRC.
 *
 * \param w_initCrc initial CRC (always called with 0 by the EPLS software)
 * \param l_subFrameLength subframe length in bytes
 * \param pv_subFrame reference to subframe
 *
 * \return 0 .. 65535 - 16 bit CRC check sum
 */
UINT16 HNFiff_Crc16_755B_CalcSwp(UINT16 w_initCrc, INT32 l_subFrameLength,
                                 const void *pv_subFrame)
{
    return crc16Checksum(l_subFrameLength, (UINT8*)pv_subFrame, w_initCrc);
}

/*============================================================================*/
/*            P R I V A T E   F U N C T I O N S                               */
/*============================================================================*/
/* \name Private Functions */
/* \{ */

/*----------------------------------------------------------------------------*/
/**
\brief    Triggers the creation of an SPDO transmit frame

This function is called periodically in every cycle to ensure that a transmit
spdo is created in every cycle.

\ingroup module_shnf
*/
/*----------------------------------------------------------------------------*/
static void buildTxSpdoFrame(void)
{
    UINT8 numFreeSpdoFrms = 1U; /* number of free SPDO frames can be sent per call of the SSC_BuildTxFrames */
    UINT32 consTime;

    if(stateh_getSnState() == kSnStateOperational)
    {
        consTime = constime_getTime();

        DEBUG_TRACE(DEBUG_LVL_SHNF, "Build TSPDO\n");

        /* SPDO frames are built */
        SPDO_BuildTxSpdo(B_INSTNUM_ consTime, &numFreeSpdoFrms);
    }
}

/*----------------------------------------------------------------------------*/
/**
\brief    Process the incoming receive SSDO/SNMT frame

\param pPayload_p   Pointer to the incoming payload
\param paylLen_p    The length of the payload

\retval TRUE    Successfully processed the incoming frame
\retval FALSE   Stack is currently busy

\ingroup module_shnf
*/
/*----------------------------------------------------------------------------*/
static BOOLEAN processRxSsdoSnmtFrame(UINT8* pPayload_p, UINT16 paylLen_p)
{
    BOOLEAN fReturn = FALSE;
    UINT32 consTime;
    SSC_t_PROCESS procRet;

    if(stateh_getSnState() > kSnStateInitializing)
    {
        if(shnfInstance_l.ssdoRxStatus_m == kSsdoRxStatusReady)
        {
            consTime = constime_getTime();

            DEBUG_TRACE(DEBUG_LVL_SHNF, "Rcv SSDO/SNMT\n");

            /* Forward frame to stack */
            procRet = SSC_ProcessSNMTSSDOFrame(B_INSTNUM_ consTime, pPayload_p, paylLen_p);
            if(procRet == SSC_k_BUSY)
            {
                /* Frame is passed to the stack but takes further calls of the process function to finish */
                shnfInstance_l.ssdoRxStatus_m = kSsdoRxStatusBusy;
                fReturn = TRUE;
            }
            else if(procRet == SSC_k_OK)
            {
                /* Frame is finished without change to busy */
                fReturn = TRUE;
            }
        }   /* no else: frame can only be posted when state machine is ready -> return false! */
    }
    else
    {
        /* Ignore frame -> Return success! */
        fReturn = TRUE;
    }

    return fReturn;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Process the incoming receive SPDO frame

\param pPayload_p   Pointer to the incoming payload
\param paylLen_p    Size of the incoming payload

\ingroup module_shnf
*/
/*----------------------------------------------------------------------------*/
static void processRxSpdoFrame(UINT8* pPayload_p, UINT16 paylLen_p)
{
    UINT32 consTime = 0;
    const UINT8 * pLenField = NULL;
    UINT16 paylLen = 0;

    /**
     * The parameter paylLen_p always provides the channel size but not
     * the real size of the frame. Therefore we need to get the frame
     * size out of the payload.
     */
    UNUSED_PARAMETER(paylLen_p);

    if(pPayload_p != NULL)
    {
        /* Only forward receive frames when the SN is in operational state */
        if(stateh_getSnState() == kSnStateOperational)
        {
            /* Get payload length field from frame */
            pLenField = &pPayload_p[FRAME_OFFSET_LENGTH];
            if(*pLenField > 0)
            {
                DEBUG_TRACE(DEBUG_LVL_SHNF, "Rcv RSPDO\n");

                consTime = constime_getTime();

                /* Calculate real size of frame by using the length field */
                paylLen = getFrameLength(pLenField);

                /* Forward frame to stack */
                SPDO_ProcessRxSpdo(B_INSTNUM_ consTime, pPayload_p, paylLen);

#if (SPDO_cfg_MAX_NO_RX_SPDO != 0)
                /* SCT timeout is checked */
                SPDO_CheckRxTimeout(B_INSTNUM_ consTime);
#endif
            }
        }
    }
    else
    {
        errh_postFatalError(kErrSourceShnf, kErrorInvalidParameter, 0);
    }

}

/*----------------------------------------------------------------------------*/
/**
\brief    Returns the length of a frame by it's payload length

\param pPaylLen_p   Pointer to the length field of the payload

\return The size of the whole frame

\ingroup module_shnf
*/
/*----------------------------------------------------------------------------*/
static UINT16 getFrameLength(const UINT8 * pPaylLen_p)
{
    UINT16 payLenRes = 0;

    if(*pPaylLen_p < 9)
    {
        /* Short frame with CRC8 */
        payLenRes = (*pPaylLen_p * 2) + (EPLS_k_MAX_HDR_LEN - 2);
    }
    else
    {
        /* Short frame with CRC16 */
        payLenRes = (*pPaylLen_p * 2) + EPLS_k_MAX_HDR_LEN;
    }

    return payLenRes;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Prepare transmit frame for transmission

This function swaps subframe1 and subframe2 and provides the result in the
target buffer.

\param pTargBuffer_p   Pointer to the target buffer
\param targBuffLen_p   Length of the target buffer
\param pSrcBuffer_p    Pointer to the source frame
\param srcBuffLen_p    Length of the frame
\param fIsSlim_p       TRUE if the frame is an SSDO slim frame

\return TRUE if the target buffer is valid; FALSE otherwise

\ingroup module_shnf
*/
/*----------------------------------------------------------------------------*/
static BOOLEAN prepareTransmitFrame(UINT8 * pTargBuffer_p, UINT16 targBuffLen_p,
                                   UINT8 * pSrcBuffer_p, UINT16 srcBuffLen_p,
                                   BOOLEAN fIsSlim_p)
{
    BOOLEAN fReturn = FALSE;
    UINT16 subFrame1Pos = 0;
    UINT16 subFrame2Targ = 0;
    UINT16 subFrame1Len = 0;
    UINT16 subFrame2Len = 0;

    /* Check if frame fits into target buffer */
    if(srcBuffLen_p <= targBuffLen_p)
    {
        if(fIsSlim_p)
        {
            /* Calculate frame positions and lengths for slim frames */
            if(srcBuffLen_p < 20)
                subFrame1Pos = SLIM_FRAME_SUB1_POS_CRC8;
            else
                subFrame1Pos = SLIM_FRAME_SUB1_POS_CRC16;

            subFrame2Targ = srcBuffLen_p - subFrame1Pos;
        }
        else
        {
            /* Calculate frame positions and lengths for normal frames */
            subFrame1Pos = (srcBuffLen_p>>1) + 1;
            subFrame2Targ = subFrame1Pos - 1;
        }

        /* Get length of sub frames */
        subFrame1Len = srcBuffLen_p - subFrame1Pos;
        subFrame2Len = srcBuffLen_p - subFrame1Len;

        /* Copy subframe1 to start of target buffer */
        MEMCOPY(pTargBuffer_p, &pSrcBuffer_p[subFrame1Pos], subFrame1Len);
        /* Add subframe2 to end of subframe1 */
        MEMCOPY(&pTargBuffer_p[subFrame2Targ], pSrcBuffer_p, subFrame2Len);

        fReturn = TRUE;
    }

    return fReturn;
}

/* \} */
