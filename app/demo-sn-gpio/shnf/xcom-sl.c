/**
********************************************************************************
\file   demo-sn-gpio/shnf/xcom-sl.c

\defgroup module_sn_shnf_xcom_sl Cross communication module (Slave side)
\{

\brief  uP-Master <-> uP-Slave cross communication (uP-Slave version)

This module implements the cross communication between uP-Master and uP-Slave
in the uP-Slave version.

\ingroup group_app_sn_shnf
*******************************************************************************/

/*------------------------------------------------------------------------------
* License Agreement
*
* Copyright 2014 BERNECKER + RAINER, AUSTRIA, 5142 EGGELSBERG, B&R STRASSE 1
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
#include <shnf/xcom.h>

#include <shnf/xcomint.h>

#include <SHNF.h>
#include <SFS.h>

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
 * \brief Cross communication instance type
 */
typedef struct
{
    volatile tXComSlMaImage  transImg_m;         /**< uP-Slave -> uP-Master transmit image */
    volatile tXComMaSlImage  rcvImg_m;           /**< uP-Master <- uP-Slave receive image */
} tXComIntInstance;

/*----------------------------------------------------------------------------*/
/* local vars                                                                 */
/*----------------------------------------------------------------------------*/
static tXComIntInstance xcomIntInstance_l SAFE_INIT_SEKTOR;

/*----------------------------------------------------------------------------*/
/* local function prototypes                                                  */
/*----------------------------------------------------------------------------*/

/*============================================================================*/
/*            P U B L I C   F U N C T I O N S                                 */
/*============================================================================*/


/*----------------------------------------------------------------------------*/
/**
\brief    Initialize the xcom module

\param[out] pTransParam_p   The transmit parameters of the uP-Master

\return TRUE on success; FALSE on error
*/
/*----------------------------------------------------------------------------*/
BOOLEAN xcomint_init(tXComTransParams * pTransParam_p)
{
    BOOLEAN fReturn = FALSE;

    MEMSET(&xcomIntInstance_l, 0, sizeof(tXComIntInstance));

    if(pTransParam_p != NULL)
    {
        /* Set transmit image id initial value */
        xcomIntInstance_l.transImg_m.id_m = ID_VAL_SLMA_MSG;

        pTransParam_p->pTxImg_m = (UINT8 *)&xcomIntInstance_l.transImg_m;
        pTransParam_p->txImgSize_m = sizeof(tXComSlMaImage);
        pTransParam_p->pRxImg_m = (UINT8 *)&xcomIntInstance_l.rcvImg_m;
        pTransParam_p->rxImgSize_m = sizeof(tXComMaSlImage);

        fReturn = TRUE;
    }
    else
    {
        errh_postFatalError(kErrSourceXCom, kErrorInvalidParameter, 0);
    }

    return fReturn;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Close the xcom module
*/
/*----------------------------------------------------------------------------*/
void xcomint_exit(void)
{
    MEMSET(&xcomIntInstance_l, 0, sizeof(tXComIntInstance));
}

/*----------------------------------------------------------------------------*/
/**
\brief    Verify the ID value of the receive image

\return TRUE if the id value is correct; FALSE otherwise
*/
/*----------------------------------------------------------------------------*/
BOOLEAN xcomint_verifyIdValue(void)
{
    BOOLEAN fReturn = FALSE;

    if(xcomIntInstance_l.rcvImg_m.id_m == ID_VAL_MASL_MSG)
    {
        fReturn = TRUE;
    }

    return fReturn;
}


/*----------------------------------------------------------------------------*/
/**
\brief    Get the value of the message format field

\param[in] paylType_p      The direction of the message format to get
\param[out] pMsgForm_p     Pointer to the message format data
*/
/*----------------------------------------------------------------------------*/
BOOLEAN xcomint_getMsgFormat(tPaylType paylType_p, UINT8 * pMsgForm_p)
{
    BOOLEAN fReturn = FALSE;

    if(pMsgForm_p != NULL)
    {
        if(paylType_p == kPaylTypeReceive)
        {
            SFS_NET_CPY8(pMsgForm_p, &xcomIntInstance_l.rcvImg_m.msgFormat_m);
            fReturn = TRUE;
        }
        else if(paylType_p == kPaylTypeTransmit)
        {
            SFS_NET_CPY8(pMsgForm_p, &xcomIntInstance_l.transImg_m.msgFormat_m);
            fReturn = TRUE;
        }
    }

    return fReturn;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Set a value to the message format field

\param[in] paylType_p      The direction of the message format to get
\param[in] msgForm_p       The new value to set
*/
/*----------------------------------------------------------------------------*/
BOOLEAN xcomint_setMsgFormat(tPaylType paylType_p, UINT8 msgForm_p)
{
    BOOLEAN fReturn = FALSE;

    if(paylType_p == kPaylTypeReceive)
    {
        SFS_NET_CPY8(&xcomIntInstance_l.rcvImg_m.msgFormat_m, &msgForm_p);
        fReturn = TRUE;
    }
    else if(paylType_p == kPaylTypeTransmit)
    {
        SFS_NET_CPY8(&xcomIntInstance_l.transImg_m.msgFormat_m, &msgForm_p);
        fReturn = TRUE;
    }

    return fReturn;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Set the flow count to the transmit image

\param[in] flowCount_p      The new flow count value
*/
/*----------------------------------------------------------------------------*/
void xcomint_setFlowCount(UINT32 flowCount_p)
{
    SFS_NET_CPY32(&xcomIntInstance_l.transImg_m.flowCnt_m, &flowCount_p);
}

/*----------------------------------------------------------------------------*/
/**
\brief    Get the flow counter from the receive and transmit image

\param[in] paylType_p      The direction of the flow counter to get
\param[out] pFlowCount_p   The resulting value of the flow counter

\return TRUE on success; FALSE on error
*/
/*----------------------------------------------------------------------------*/
BOOLEAN xcomint_getFlowCnt(tPaylType paylType_p, UINT32 * pFlowCount_p)
{
    BOOLEAN fReturn = FALSE;

    if(pFlowCount_p != NULL)
    {
        if(paylType_p == kPaylTypeReceive)
        {
            SFS_NET_CPY32(pFlowCount_p, &xcomIntInstance_l.rcvImg_m.flowCnt_m);
            fReturn = TRUE;
        }
        else if(paylType_p == kPaylTypeTransmit)
        {
            SFS_NET_CPY32(pFlowCount_p, &xcomIntInstance_l.transImg_m.flowCnt_m);
            fReturn = TRUE;
        }
    }

    return fReturn;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Set the current timebase to the transmit image

\param[in] pCurrTime_p      Pointer to the value of the current timebase
*/
/*----------------------------------------------------------------------------*/
void xcomint_setTransTimebase(UINT64 * pCurrTime_p)
{
    MEMCOPY(&xcomIntInstance_l.transImg_m.currTime_m, pCurrTime_p, sizeof(UINT64));
}

/*----------------------------------------------------------------------------*/
/**
\brief    Get the timebase from the receive and transmit image

\param[in] paylType_p      The direction of the timebase to get
\param[out] pRetTime       The current value of the time field

\return TRUE on success; FALSE on error
*/
/*----------------------------------------------------------------------------*/
BOOLEAN xcomint_getTimeBase(tPaylType paylType_p, UINT64 * pRetTime)
{
    BOOLEAN fReturn = FALSE;

    if(paylType_p == kPaylTypeReceive)
    {
        MEMCOPY(pRetTime, &xcomIntInstance_l.rcvImg_m.currTime_m, sizeof(UINT64));

        fReturn = TRUE;
    }
    else if(paylType_p == kPaylTypeTransmit)
    {
        MEMCOPY(pRetTime, &xcomIntInstance_l.transImg_m.currTime_m, sizeof(UINT64));

        fReturn = TRUE;
    }

    return fReturn;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Set the SSDO/SNMT CRC to transmit image

\param[in] crcSub1_p   The new CRC value of subframe one
\param[in] crcSub2_p   The new CRC value of subframe two
*/
/*----------------------------------------------------------------------------*/
void xcomint_setSsdoSnmtCrc(UINT16 crcSub1_p, UINT16 crcSub2_p)
{
    SFS_NET_CPY16(&xcomIntInstance_l.transImg_m.ssdoSub1Crc_m, &crcSub1_p);
    SFS_NET_CPY16(&xcomIntInstance_l.transImg_m.ssdoSub2Crc_m, &crcSub2_p);
}


/*----------------------------------------------------------------------------*/
/**
\brief    Get the CRC values of the receive and transmit image

\param[in] paylType_p      The direction of the timebase to get
\param[out] pSub1Crc_p     Pointer the resulting CRC value of subframe one
\param[out] pSub2Crc_p     Pointer the resulting CRC value of subframe two

\return TRUE on success; FALSE on error
*/
/*----------------------------------------------------------------------------*/
BOOLEAN xcomint_getSsdoSnmtCrc(tPaylType paylType_p, UINT16 * pSub1Crc_p,
                                                     UINT16 * pSub2Crc_p)
{
    BOOLEAN fReturn = FALSE;

    if(paylType_p == kPaylTypeReceive)
    {
        SFS_NET_CPY16(pSub1Crc_p, &xcomIntInstance_l.rcvImg_m.ssdoSub1Crc_m);
        SFS_NET_CPY16(pSub2Crc_p, &xcomIntInstance_l.rcvImg_m.ssdoSub2Crc_m);
        fReturn = TRUE;
    }
    else if(paylType_p == kPaylTypeTransmit)
    {
        SFS_NET_CPY16(pSub1Crc_p, &xcomIntInstance_l.transImg_m.ssdoSub1Crc_m);
        SFS_NET_CPY16(pSub2Crc_p, &xcomIntInstance_l.transImg_m.ssdoSub2Crc_m);

        fReturn = TRUE;
    }

    return fReturn;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Set the SSDO/SNMT parameters of the transmit image

\param[in] pSub1Params_p   Pointer to the subframe1 transfer details
\param[in] pSub2Params_p   Pointer to the subframe2 transfer details
\param[in] pTargBuff_p     The base address of the target buffer
\param[in] targLen_p       The length of the target buffer

\return TRUE on success; FALSE on error
*/
/*----------------------------------------------------------------------------*/
BOOLEAN xcomint_postSsdoSnmtFrame(tSubFrameParams * pSub1Params_p,
                                  tSubFrameParams * pSub2Params_p,
                                  UINT8 * pTargBuff_p, UINT32 targLen_p)
{
    BOOLEAN fReturn = FALSE;

    UNUSED_PARAMETER(pTargBuff_p);
    UNUSED_PARAMETER(targLen_p);
    UNUSED_PARAMETER(pSub1Params_p);

    if(pSub2Params_p != NULL)
    {
        if(pSub2Params_p->pSubBase_m != NULL             &&
           pSub2Params_p->subLen_m > 0                   &&
           pSub2Params_p->subLen_m <= TSSDO_SNMT_SUB2_LEN )
        {
            /* Copy subframe two data to transmit buffer */
            MEMCOPY(&xcomIntInstance_l.transImg_m.ssdoSub2Payl_m,
                    pSub2Params_p->pSubBase_m, pSub2Params_p->subLen_m);

            fReturn = TRUE;
        }
        else
        {
            errh_postFatalError(kErrSourceXCom, kErrorInvalidParameter, 0);
        }
    }
    else
    {
        errh_postFatalError(kErrSourceXCom, kErrorInvalidParameter, 0);
    }

    return fReturn;
}


/*----------------------------------------------------------------------------*/
/**
\brief    Handle incoming SSDO/SNMT payload

\return TRUE on success; FALSE on error
*/
/*----------------------------------------------------------------------------*/
BOOLEAN xcomint_handleSsdoSnmtPayload(void)
{
    /* For uP-Slave only data is transmitted */

    return TRUE;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Set the SPDO crc to transmit image

\param[in] crcSub1_p   The new CRC value of subframe one
\param[in] crcSub2_p   The new CRC value of subframe two
*/
/*----------------------------------------------------------------------------*/
void xcomint_setSpdoCrc(UINT16 crcSub1_p, UINT16 crcSub2_p)
{
    SFS_NET_CPY16(&xcomIntInstance_l.transImg_m.spdoSub1Crc_m, &crcSub1_p);
    SFS_NET_CPY16(&xcomIntInstance_l.transImg_m.spdoSub2Crc_m, &crcSub2_p);
}


/*----------------------------------------------------------------------------*/
/**
\brief    Get the CRC values of the receive and transmit image

\param[in] paylType_p      The direction of the timebase to get
\param[out] pSub1Crc_p     Pointer the resulting CRC value of subframe one
\param[out] pSub2Crc_p     Pointer the resulting CRC value of subframe two

\return TRUE on success; FALSE on error
*/
/*----------------------------------------------------------------------------*/
BOOLEAN xcomint_getSpdoCrc(tPaylType paylType_p, UINT16 * pSub1Crc_p,
                                                 UINT16 * pSub2Crc_p)
{
    BOOLEAN fReturn = FALSE;

    if(paylType_p == kPaylTypeReceive)
    {
        SFS_NET_CPY16(pSub1Crc_p, &xcomIntInstance_l.rcvImg_m.spdoSub1Crc_m);
        SFS_NET_CPY16(pSub2Crc_p, &xcomIntInstance_l.rcvImg_m.spdoSub2Crc_m);
        fReturn = TRUE;
    }
    else if(paylType_p == kPaylTypeTransmit)
    {
        SFS_NET_CPY16(pSub1Crc_p, &xcomIntInstance_l.transImg_m.spdoSub1Crc_m);
        SFS_NET_CPY16(pSub2Crc_p, &xcomIntInstance_l.transImg_m.spdoSub2Crc_m);

        fReturn = TRUE;
    }

    return fReturn;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Post the SPDO frame to the transmit image

\param[in] pSub1Params_p   Pointer to the subframe1 transfer details
\param[in] pSub2Params_p   Pointer to the subframe2 transfer details
\param[in] pTargBuff_p     The base address of the target buffer
\param[in] targLen_p       The length of the target buffer

\return TRUE on success; FALSE on error
*/
/*----------------------------------------------------------------------------*/
BOOLEAN xcomint_postSpdoFrame(tSubFrameParams * pSub1Params_p,
                              tSubFrameParams * pSub2Params_p,
                              UINT8 * pTargBuff_p, UINT32 targLen_p)
{
    BOOLEAN fReturn = FALSE;

    UNUSED_PARAMETER(pTargBuff_p);
    UNUSED_PARAMETER(targLen_p);
    UNUSED_PARAMETER(pSub1Params_p);

    if(pSub2Params_p != NULL)
    {
        if(pSub2Params_p->pSubBase_m != NULL        &&
           pSub2Params_p->subLen_m > 0              &&
           pSub2Params_p->subLen_m <= TSPDO_SUB2_LEN )
        {
            /* Copy subframe two data to transmit buffer */
            MEMCOPY(&xcomIntInstance_l.transImg_m.spdoSub2Payl_m,
                    pSub2Params_p->pSubBase_m, pSub2Params_p->subLen_m);

            fReturn = TRUE;
        }
        else
        {
            errh_postFatalError(kErrSourceXCom, kErrorInvalidParameter, 0);
        }
    }
    else
    {
        errh_postFatalError(kErrSourceXCom, kErrorInvalidParameter, 0);
    }

    return fReturn;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Handle incoming SPDO payload

\return TRUE on success; FALSE on error
*/
/*----------------------------------------------------------------------------*/
BOOLEAN xcomint_handleSpdoPayload(void)
{
    /* For uP-Slave only data is transmitted */

    return TRUE;
}

/*============================================================================*/
/*            P R I V A T E   F U N C T I O N S                               */
/*============================================================================*/
/** \name Private Functions */
/** \{ */

/**
 * \}
 * \}
 */
