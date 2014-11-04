/**
********************************************************************************
\file   timer.c

\brief  Target specific functions of the system timer

This module implements the hardware near target specific functions of the
system timer for Altera Nios2.

\ingroup module_timer
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
#include <apptarget/timer.h>

#include <system.h>
#include <io.h>

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
#define COUNTER_BASE  APP_0_COUNTER_0_BASE

#define COUNTER_TIME_REG           0
#define COUNTER_TICKCNT_REG        4

//------------------------------------------------------------------------------
// local types
//------------------------------------------------------------------------------

#define TIMER_TICKS_1US         50
#define TIMER_TICKS_10US        500
#define TIMER_TICKS_100US       5000
#define TIMER_TICKS_1MS         50000

//------------------------------------------------------------------------------
// local vars
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// local function prototypes
//------------------------------------------------------------------------------

//============================================================================//
//            P U B L I C   F U N C T I O N S                                 //
//============================================================================//

//------------------------------------------------------------------------------
/**
\brief    Initialize the timer module

\return 0 on success; 1 on error

\ingroup module_timer
*/
//------------------------------------------------------------------------------
UINT8 timer_init(void)
{
    IOWR_32DIRECT(COUNTER_BASE, COUNTER_TICKCNT_REG, 0);

    /* Set default timerbase to ipcore */
    IOWR_32DIRECT(COUNTER_BASE, COUNTER_TICKCNT_REG, TIMER_TICKS_100US);

    return 0;
}

//------------------------------------------------------------------------------
/**
\brief    Close the timer module

\ingroup module_timer
*/
//------------------------------------------------------------------------------
void timer_close(void)
{
    /* Disable the counter by writing zero to the tickcnt register*/
    IOWR_32DIRECT(COUNTER_BASE, COUNTER_TICKCNT_REG, 0);
}

//------------------------------------------------------------------------------
/**
\brief    Get current system tick

This function returns the current system tick determined by the system timer.

\return Returns the system tick in milliseconds

\ingroup module_timer
*/
//------------------------------------------------------------------------------
UINT32 timer_getTickCount(void)
{
    UINT32 time;

    time = IORD_32DIRECT(COUNTER_BASE, COUNTER_TIME_REG);

    return time;
}

//------------------------------------------------------------------------------
/**
\brief    Update the base of the timer

\param base_p The new base of the timer

\return 0 on success; 1 on error

\ingroup module_timer
*/
//------------------------------------------------------------------------------
UINT8 timer_setBase(tTimerBase base_p)
{
    UINT8 ret = 1;
    UINT32 timerBase = kTimerBase1us;

    /* De-activate timer core */
    IOWR_32DIRECT(COUNTER_BASE, COUNTER_TICKCNT_REG, 0);

    switch(base_p)
    {
        case kTimerBase1us:
            timerBase = TIMER_TICKS_1US;
            ret = 0;
            break;
        case kTimerBase10us:
            timerBase = TIMER_TICKS_10US;
            ret = 0;
            break;
        case kTimerBase100us:
            timerBase = TIMER_TICKS_100US;
            ret = 0;
            break;
        case kTimerBase1ms:
            timerBase = TIMER_TICKS_1MS;
            ret = 0;
            break;
        default:
            break;
    }

    /* Update the base register of the timer core */
    IOWR_32DIRECT(COUNTER_BASE, COUNTER_TICKCNT_REG, timerBase);

    return ret;
}


//============================================================================//
//            P R I V A T E   F U N C T I O N S                               //
//============================================================================//
/// \name Private Functions
/// \{

/// \}
