/**
********************************************************************************
\file   handshake-ma.c

\brief  Implements the handshake of uP-Master and uP-Slave

This module implements the handshake of both safe processors on the uP-Master
side. The master busy waits until the init arrives from the slave and sends
back it's current consecutive timebase value.

\ingroup module_hands
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
#include <sn/handshake.h>

#include <sn/upserial.h>

#include <common/platform.h>

/*============================================================================*/
/*            G L O B A L   D E F I N I T I O N S                             */
/*============================================================================*/

/*----------------------------------------------------------------------------*/
/* const defines                                                              */
/*----------------------------------------------------------------------------*/
#define WELCOME_RCV_TIMEOUT_MS  0xFFFFF     /**< Receive timeout of the welcome message */

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
 * \bried Handshake module instance type
 */
typedef struct
{
    volatile UINT8 welcomeMsg_m[WELCOME_MSG_LEN];      /**< Buffer the welcome message is stored */
    volatile UINT8 respMsg_m[RESPONSE_MSG_LEN];        /**< Buffer the response message is stored */
} tHandsInstance;

/*----------------------------------------------------------------------------*/
/* local vars                                                                 */
/*----------------------------------------------------------------------------*/
static tHandsInstance handsInstance_l SAFE_INIT_SEKTOR;

/*----------------------------------------------------------------------------*/
/* local function prototypes                                                  */
/*----------------------------------------------------------------------------*/
static BOOLEAN receiveWelcome(void);
static BOOLEAN sendResponse(void);
static BOOLEAN verifyWelcomeMessage(volatile UINT8 * pWelcomeMsg_p, UINT32 msglen_p);
static void fillResponseMsg(volatile UINT8 ** ppRespMsg_p, UINT8 * pRespLen_p);

/*============================================================================*/
/*            P U B L I C   F U N C T I O N S                                 */
/*============================================================================*/

/*----------------------------------------------------------------------------*/
/**
\brief    Carry out the handshake between both processors

\retval TRUE        Handshake was successful
\retval FALSE       Error on handshake

\ingroup module_hands
*/
/*----------------------------------------------------------------------------*/
BOOLEAN hands_perfHandshake(void)
{
    BOOLEAN fReturn = FALSE;

    /* Reset global variables */
    MEMSET(&handsInstance_l, 0, sizeof(tHandsInstance));

    upserial_deRegisterCb();

    /* Enable reception of welcome message */
    if(receiveWelcome())
    {
        /* Wait for some time */
        platform_msleep(100);

        /* Send a response back to uP-Slave */
        if(sendResponse())
        {
            fReturn = TRUE;
        }
    }

    return fReturn;
}

/*============================================================================*/
/*            P R I V A T E   F U N C T I O N S                               */
/*============================================================================*/
/* \name Private Functions */
/* \{ */

/*----------------------------------------------------------------------------*/
/**
\brief    Receive the welcome message

\retval TRUE        Successfully received and verified the welcome message
\retval FALSE       Error on receiving the welcome message

\ingroup module_hands
*/
/*----------------------------------------------------------------------------*/
static BOOLEAN receiveWelcome(void)
{
    BOOLEAN fReturn = FALSE;
    volatile UINT8 * pWelcomeMsg = &handsInstance_l.welcomeMsg_m[0];

    DEBUG_TRACE(DEBUG_LVL_ALWAYS, "\nWait for welcome message -> ");

    /* Enable receive channel */
    if(upserial_receiveBlock(pWelcomeMsg, WELCOME_MSG_LEN, WELCOME_RCV_TIMEOUT_MS))
    {
        /* Check if the received data is correct */
        if(verifyWelcomeMessage(pWelcomeMsg, WELCOME_MSG_LEN))
        {
            DEBUG_TRACE(DEBUG_LVL_ALWAYS, "SUCCESS!\n");

            fReturn = TRUE;
        }
        else
        {
            /* Welcome message is invalid */
            errh_postFatalError(kErrSourcePeriph, kHandSWelcomeMsgInvalid, 0);
        }
    }
    else
    {
        errh_postFatalError(kErrSourcePeriph, kErrorSerialReceiveFailed, 0);
    }

    return fReturn;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Send a response to uP-Slave

\retval TRUE        Response sent to uP-Slave
\retval FALSE       Error on sending

\ingroup module_hands
*/
/*----------------------------------------------------------------------------*/
static BOOLEAN sendResponse(void)
{
    BOOLEAN fReturn = FALSE;
    volatile UINT8 * pRespMsg = NULL;
    UINT8 respSize = 0;

    /* Create the response message */
    fillResponseMsg(&pRespMsg, &respSize);

    /* Forward the data to the serial module */
    if(upserial_transmitBlock(pRespMsg, respSize))
    {
        fReturn = TRUE;
    }
    else
    {
        errh_postFatalError(kErrSourcePeriph, kErrorSerialTransmitFailed, 0);
    }

    return fReturn;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Verify the welcome message correctness

\param[in] pWelcomeMsg_p    Pointer to the message
\param[in] msglen_p         Size of the message

\retval TRUE    The welcome message is correct
\retval FALSE   Invalid message received

\ingroup module_hands
*/
/*----------------------------------------------------------------------------*/
static BOOLEAN verifyWelcomeMessage(volatile UINT8 * pWelcomeMsg_p, UINT32 msglen_p)
{
    BOOLEAN fMsgCorrect = FALSE;
    UINT32 msgContent = (UINT32)WELCOME_MSG_CONTENT;

    /* Verify the correctness of the welcome message */
    if(MEMCOMP(pWelcomeMsg_p, &msgContent, msglen_p) == 0)
    {
        fMsgCorrect = TRUE;
    }

    return fMsgCorrect;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Fill the response message buffer

\param[out] ppRespMsg_p      Pointer to the response message
\param[out] msglen_p         Pointer to the resulting size of the message

\ingroup module_hands
*/
/*----------------------------------------------------------------------------*/
static void fillResponseMsg(volatile UINT8 ** ppRespMsg_p, UINT8 * pRespLen_p)
{
    volatile UINT8 * pRespMsg = &handsInstance_l.respMsg_m[0];
    UINT64 currTime = constime_getTimeBase();

    MEMCOPY(pRespMsg, &handsInstance_l.welcomeMsg_m[0], sizeof(UINT32));
    MEMCOPY(&pRespMsg[WELCOME_MSG_LEN], &currTime, sizeof(UINT64));

    /* Set pointer to the result buffer */
    *ppRespMsg_p = pRespMsg;
    *pRespLen_p = (UINT32)RESPONSE_MSG_LEN;
}

/* \} */
