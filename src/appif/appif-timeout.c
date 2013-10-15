/**
********************************************************************************
\file   appif-timeout.c

\brief  Module for internal timeout generation

This module generates a timeout for asynchronous transmissions by simply counting
the synchronous interrupts. If a limit is reached the timeout is generated.

\ingroup module_timeout
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
#include <appif/appif-timeoutint.h>
#include <appif/appif-timeout.h>

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

#define TIMEOUT_MAX_INSTANCES       5       ///< Maximum number of timeout module instances

//------------------------------------------------------------------------------
// local types
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// local vars
//------------------------------------------------------------------------------

static struct  eTimeoutInstance       timeoutInstance_l[TIMEOUT_MAX_INSTANCES];

//------------------------------------------------------------------------------
// local function prototypes
//------------------------------------------------------------------------------


//============================================================================//
//            P U B L I C   F U N C T I O N S                                 //
//============================================================================//

//------------------------------------------------------------------------------
/**
\brief    Initialize the timeout module

\return kAppIfSuccessful

\ingroup module_timeout
*/
//------------------------------------------------------------------------------
void timeout_init(void)
{
    APPIF_MEMSET(&timeoutInstance_l, 0 , sizeof(struct eTimeoutInstance) * TIMEOUT_MAX_INSTANCES);
}

//------------------------------------------------------------------------------
/**
\brief    Create an instance of the timeout module

\param[in]  cycleLimit_p    Cycle time limit counter

\return tTimeoutInstance
\retval Address              Pointer to the instance of the channel
\retval Null                 Unable to allocate instance

\ingroup module_timeout
*/
//------------------------------------------------------------------------------
tTimeoutInstance timeout_create(UINT16 cycleLimit_p)
{
    tTimeoutInstance pInstance = NULL;
    UINT8  timeoutId = 0xFF, i;

    // Search for free instance
    for(i=0; i < TIMEOUT_MAX_INSTANCES; i++)
    {
        if(timeoutInstance_l[i].fInstUsed_m == FALSE)
        {
            // Free instance found!
            timeoutId = i;
            break;
        }
    }

    if(timeoutId != 0xFF)
    {
        // Set maximum cycle count
        timeoutInstance_l[timeoutId].cycleLimit_m = cycleLimit_p;

        // Set valid instance id
        timeoutInstance_l[timeoutId].fInstUsed_m = TRUE;
        pInstance = &timeoutInstance_l[timeoutId];
    }

    return pInstance;
}

//------------------------------------------------------------------------------
/**
\brief    Destroy the timer instance

\param[in]  pInstance_p     Timeout module instance

\ingroup module_timeout
*/
//------------------------------------------------------------------------------
void timeout_destroy(tTimeoutInstance pInstance_p)
{
    // Destroy timeout instance!
    APPIF_MEMSET(pInstance_p, 0 , sizeof(struct eTimeoutInstance));
}



//------------------------------------------------------------------------------
/**
\brief    Check transmission validity

\param[in]  pInstance_p     Timeout module instance

\return tTimerStatus
\retval kTimerStateInvalid    Error occurred
\retval kTimerStateRunning    Timer is running
\retval kTimerStateStopped    Timer is stopped
\retval kTimerStateExpired    Timer is expired

\ingroup module_timeout
*/
//------------------------------------------------------------------------------
tTimerStatus timeout_checkExpire(tTimeoutInstance pInstance_p)
{
    tTimerStatus timerState = kTimerStateInvalid;

    if( pInstance_p->fTimeoutEn_m != FALSE )
    {
        timerState = kTimerStateRunning;

        if(pInstance_p->timeoutCycleCount_m > pInstance_p->cycleLimit_m)
        {
            timeout_stopTimer(pInstance_p);

            timerState = kTimerStateExpired;
        }
    }
    else
    {
        timerState = kTimerStateStopped;
    }

    return timerState;
}

//------------------------------------------------------------------------------
/**
\brief    Increment the local cycle counter

\param[in]  pInstance_p     Timeout module instance

\ingroup module_timeout
*/
//------------------------------------------------------------------------------
void timeout_incrementCounter(tTimeoutInstance pInstance_p)
{
    if(pInstance_p->fTimeoutEn_m != FALSE)
    {
        pInstance_p->timeoutCycleCount_m++;
    }
}

//------------------------------------------------------------------------------
/**
\brief    Start the timer for this instance

\param[in]  pInstance_p     Timeout module instance

\ingroup module_timeout
*/
//------------------------------------------------------------------------------
void timeout_startTimer(tTimeoutInstance pInstance_p)
{
    pInstance_p->fTimeoutEn_m = TRUE;
    pInstance_p->timeoutCycleCount_m = 0;
}

//------------------------------------------------------------------------------
/**
\brief    Stop the timer for this instance

\param[in]  pInstance_p     Timeout module instance

\ingroup module_timeout
*/
//------------------------------------------------------------------------------
void timeout_stopTimer(tTimeoutInstance pInstance_p)
{
    pInstance_p->fTimeoutEn_m = FALSE;
    pInstance_p->timeoutCycleCount_m = 0;
}

//------------------------------------------------------------------------------
/**
\brief    Check if the timer is running

\param[in]  pInstance_p     Timeout module instance

\return tTimerStatus
\retval kTimerStateRunning       Timer is running
\retval kTimerStateStopped       Timer is stopped

\ingroup module_timeout
*/
//------------------------------------------------------------------------------
tTimerStatus timeout_isRunning(tTimeoutInstance pInstance_p)
{
    tTimerStatus timerState = kTimerStateInvalid;

    if(pInstance_p->fTimeoutEn_m != FALSE)
    {
        timerState = kTimerStateRunning;
    }
    else
    {
        timerState = kTimerStateStopped;
    }

    return timerState;
}

//============================================================================//
//            P R I V A T E   F U N C T I O N S                               //
//============================================================================//
/// \name Private Functions
/// \{

/// \}

