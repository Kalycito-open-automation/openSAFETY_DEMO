/**
********************************************************************************
\file   handshake-sl.c

\brief  Implements the handshake of uP-Master and uP-Slave

This module implements the handshake of both safe processors on the uP-Slave
side. The slave sends the welcome message to the uP-Master and waits for the
response.

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
#define BOOTUP_TIMEOUT_MS           (UINT32)0x2000      /**< Wait time until a timeout occurs */


/*----------------------------------------------------------------------------*/
/* local types                                                                */
/*----------------------------------------------------------------------------*/

/**
 * \brief Response message receive state
 */
typedef enum
{
    kRcvStateInvalid    = 0x0,      /**< Invalid state reached */
    kRcvStateSuccessful = 0x1,      /**< Receive was successful */
    kRcvStateError      = 0x2,      /**< Error on receive */
    kRcvStateRetry      = 0x3,      /**< Timeout occurred -> Retry */
} tReceiveState;

/**
 * \bried Handshake module instance type
 */
typedef struct
{
    volatile UINT8 welcomeMsg_m[WELCOME_MSG_LEN];        /**< Buffer the welcome message is stored */
    volatile UINT8 responseMsg_m[RESPONSE_MSG_LEN];      /**< Buffer the response message is stored */
} tHandsInstance;

/*----------------------------------------------------------------------------*/
/* local vars                                                                 */
/*----------------------------------------------------------------------------*/
static tHandsInstance handsInstance_l SAFE_INIT_SEKTOR;

/*----------------------------------------------------------------------------*/
/* local function prototypes                                                  */
/*----------------------------------------------------------------------------*/
static BOOLEAN sendWelcome(void);
static tReceiveState receiveResponse(void);
static BOOLEAN verifyResponseMessage(volatile UINT8 * pRespMsg_p, UINT32 respLen_p);
static void fillWelcomeMsg(volatile UINT8 ** ppWelcMsg_p, UINT8 * pWelcLen_p);

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
    tReceiveState rcvState;

    /* Reset global variables */
    MEMSET(&handsInstance_l, 0, sizeof(tHandsInstance));

    upserial_deRegisterCb();

    /* Wait for uP-Master */
    platform_msleep(100);

    do
    {
        /* Send welcome message to uP-Master */
        if(sendWelcome())
        {
            /* Wait for response */
            rcvState = receiveResponse();
        }
        else
        {
            /* On send error -> Terminate! */
            break;
        }
    } while(rcvState == kRcvStateRetry);

    /* Set return value */
    if(rcvState == kRcvStateSuccessful)
    {
        fReturn = TRUE;
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
\brief    Send the welcome message

\retval TRUE        Successfully sent the welcome message
\retval FALSE       Error on sending

\ingroup module_hands
*/
/*----------------------------------------------------------------------------*/
static BOOLEAN sendWelcome(void)
{
    BOOLEAN fReturn = FALSE;
    volatile UINT8 * pWelcomeMsg = NULL;
    UINT8 welcomeSize = 0;

    /* Create the welcome message */
    fillWelcomeMsg(&pWelcomeMsg, &welcomeSize);

    DEBUG_TRACE(DEBUG_LVL_ALWAYS, "\nSend welcome message -> ");

    /* Forward the data to the serial module */
    if(upserial_transmitBlock(pWelcomeMsg, welcomeSize))
    {
        DEBUG_TRACE(DEBUG_LVL_ALWAYS, "SUCCESS!\n");

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
\brief    Wait for the response from the uP-Master

\return The receive state of the response message

\ingroup module_hands
*/
/*----------------------------------------------------------------------------*/
static tReceiveState receiveResponse(void)
{
    tReceiveState rcvState = kRcvStateError;
    volatile UINT8 * pResponseMsg = &handsInstance_l.responseMsg_m[0];
    UINT32 respLen = (UINT32)RESPONSE_MSG_LEN;

    if(upserial_receiveBlock(pResponseMsg, respLen, BOOTUP_TIMEOUT_MS))
    {
        /* Check if the received data is correct */
        if(verifyResponseMessage(pResponseMsg, respLen))
        {
            rcvState = kRcvStateSuccessful;
        }
        else
        {
            /* Response message is invalid */
            errh_postFatalError(kErrSourcePeriph, kHandSResponseMsgInvalid, 0);
        }
    }
    else
    {
        rcvState = kRcvStateRetry;
    }

    return rcvState;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Verify the response message correctness

\param[in] pRespMsg_p           Pointer to the received response message
\param[in] respLen_p            Size of the response message

\retval TRUE    The response message is correct
\retval FALSE   Invalid response received

\ingroup module_hands
*/
/*----------------------------------------------------------------------------*/
static BOOLEAN verifyResponseMessage(volatile UINT8 * pRespMsg_p, UINT32 respLen_p)
{
    BOOLEAN fMsgCorrect = FALSE;
    UINT64 currTime = 0;


    if(respLen_p <= (UINT32)RESPONSE_MSG_LEN)
    {
        /* Verify the correctness of the response message header */
        if(MEMCOMP(pRespMsg_p, &handsInstance_l.welcomeMsg_m[0], WELCOME_MSG_LEN) == 0)
        {
            /* Overtake the new consecutive timebase value */
            MEMCOPY(&currTime, &pRespMsg_p[WELCOME_MSG_LEN], sizeof(UINT64));

            /* Forward the new time value to the consecutive time module */
            constime_setTimebase(currTime);

            fMsgCorrect = TRUE;
        }
    }

    return fMsgCorrect;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Fill the welcome message buffer

\param[out] ppWelcMsg_p      Pointer to the welcome message
\param[out] pWelcLen_p       Pointer to the resulting size of the message

\ingroup module_hands
*/
/*----------------------------------------------------------------------------*/
static void fillWelcomeMsg(volatile UINT8 ** ppWelcMsg_p, UINT8 * pWelcLen_p)
{
    volatile UINT8 * pWelcomeMsg = &handsInstance_l.welcomeMsg_m[0];
    UINT32 welcomeMsg = WELCOME_MSG_CONTENT;

    MEMCOPY(pWelcomeMsg, &welcomeMsg, WELCOME_MSG_LEN);

    /* Set pointer to the result buffer */
    *ppWelcMsg_p = pWelcomeMsg;
    *pWelcLen_p = (UINT32)WELCOME_MSG_LEN;
}

/* \} */
