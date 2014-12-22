/**
********************************************************************************
\file   statehandler.c

\brief  This module handles the current SN state

This module consists of the current CN state which. It provides access
to this state in the whole firmware.

\ingroup module_stateh
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
#include <shnf/statehandler.h>

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
 * \brief The state handler instance type
 */
typedef struct
{
    BOOLEAN fEnterPreop_m;                 /**< TRUE if the SN should enter pre-operational state */
    BOOLEAN fEnterOperational_m;           /**< TRUE if the SN should enter operational state */
    BOOLEAN fShutdown_m;                   /**< TRUE if the SN should perform a shutdown */
    tSnState currSnState_m;                /**< Stores the current SN state */
} tStateHInstance;

/*----------------------------------------------------------------------------*/
/* local vars                                                                 */
/*----------------------------------------------------------------------------*/
static tStateHInstance statehInstance_l SAFE_INIT_SEKTOR;

/*----------------------------------------------------------------------------*/
/* local function prototypes                                                  */
/*----------------------------------------------------------------------------*/

/*============================================================================*/
/*            P U B L I C   F U N C T I O N S                                 */
/*============================================================================*/

/*----------------------------------------------------------------------------*/
/**
\brief    Initialize the state handler module

\param[in] snInitState_p    The initial state of the SN

\return TRUE on success; FALSE on error

\ingroup module_stateh
*/
/*----------------------------------------------------------------------------*/
BOOLEAN stateh_init(tSnState snInitState_p)
{
    BOOLEAN fReturn = FALSE;

    if(snInitState_p > kSnStateInvalid && snInitState_p < kSnStateCount)
    {
        MEMSET(&statehInstance_l, 0, sizeof(tStateHInstance));

        /* Set the SN initial state */
        statehInstance_l.currSnState_m = snInitState_p;

        fReturn = TRUE;
    }

    return fReturn;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Close the state handler module

\ingroup module_stateh
*/
/*----------------------------------------------------------------------------*/
void stateh_exit(void)
{
    MEMSET(&statehInstance_l, 0, sizeof(tStateHInstance));
}

/*----------------------------------------------------------------------------*/
/**
\brief    Set the SN state variable to a new value

\param[in] newState_p       The new state of the SN

\ingroup module_stateh
*/
/*----------------------------------------------------------------------------*/
void stateh_setSnState(tSnState newState_p)
{
    statehInstance_l.currSnState_m = newState_p;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Get the current value of the SN state variables

\return   The current state of the SN

\ingroup module_stateh
*/
/*----------------------------------------------------------------------------*/
tSnState stateh_getSnState(void)
{
    return statehInstance_l.currSnState_m;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Set the enter operational flag to a new value

\param[in] newVal_p       The new value of the enter op flag

\ingroup module_stateh
*/
/*----------------------------------------------------------------------------*/
void stateh_setEnterOpFlag(BOOLEAN newVal_p)
{
    statehInstance_l.fEnterOperational_m = newVal_p;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Get the new value of the enter operational flag

\return   The current value of the enter operational flag

\ingroup module_stateh
*/
/*----------------------------------------------------------------------------*/
BOOLEAN stateh_getEnterOpFlag(void)
{
    return statehInstance_l.fEnterOperational_m;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Set the enter pre-operational flag to a new value

\param[in] newVal_p       The new value of the enter preop flag

\ingroup module_stateh
*/
/*----------------------------------------------------------------------------*/
void stateh_setEnterPreOpFlag(BOOLEAN newVal_p)
{
    statehInstance_l.fEnterPreop_m = newVal_p;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Get the new value of the enter pre-operational flag

\return   The current value of the enter pre-operational flag

\ingroup module_stateh
*/
/*----------------------------------------------------------------------------*/
BOOLEAN stateh_getEnterPreOpFlag(void)
{
    return statehInstance_l.fEnterPreop_m;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Set the enter shutdown flag to a new value

\param[in] newVal_p       The new value of the enter shutdown flag

\ingroup module_stateh
*/
/*----------------------------------------------------------------------------*/
void stateh_setShutdownFlag(BOOLEAN newVal_p)
{
    statehInstance_l.fShutdown_m = newVal_p;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Get the new value of the enter shutdown flag

\return   The current value of the enter shutdown flag

\ingroup module_stateh
*/
/*----------------------------------------------------------------------------*/
BOOLEAN stateh_getShutdownFlag(void)
{
    return statehInstance_l.fShutdown_m;
}

/*============================================================================*/
/*            P R I V A T E   F U N C T I O N S                               */
/*============================================================================*/
/* \name Private Functions */
/* \{ */

/* \} */
