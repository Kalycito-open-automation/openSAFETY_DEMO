/**
********************************************************************************
\file   constime.c

\brief  This module provides the consecutive timebase

This module provides the consecutive timebase. It acts as an interface to the
target specific module of the hardware timer.

\ingroup module_constime
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
#include <shnf/constime.h>

#include <sapl/sapl.h>
#include <apptarget/timer.h>

#include <SODapi.h>

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

/*----------------------------------------------------------------------------*/
/* local vars                                                                 */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* local function prototypes                                                  */
/*----------------------------------------------------------------------------*/

/*============================================================================*/
/*            P U B L I C   F U N C T I O N S                                 */
/*============================================================================*/

/*----------------------------------------------------------------------------*/
/**
\brief    Initialize the consecutive time

\retval TRUE    CT initialization successful
\retval FALSE   Error during CT initialization

\ingroup module_constime
*/
/*----------------------------------------------------------------------------*/
BOOLEAN constime_init(void)
{
    BOOLEAN fReturn = FALSE;

    if(timer_init() == 0)
    {
        fReturn = TRUE;
    }

    return fReturn;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Disable the consecutive time

\ingroup module_constime
*/
/*----------------------------------------------------------------------------*/
void constime_exit(void)
{
    timer_close();
}

/*----------------------------------------------------------------------------*/
/**
\brief    Get the current consecutive time

\return The current consecutive time

\ingroup module_constime
*/
/*----------------------------------------------------------------------------*/
UINT32 constime_getTime(void)
{
    return timer_getTickCount();
}


/*
 * This function is called after the write access to object
 * 0x1200, subindex 0x3. It is used to configure the hardware
 * timer with the new timebase.
 *
 * \param B_INSTNUM    instance number
 * \param e_srvc       type of service, see {SOD_t_SERVICE}
 *                     (not checked, only called with enum
 *                     value in CallBeforeReadClbk() or
 *                     CallBeforeWriteClbk() or
 *                     CallAfterWriteClbk())
 *                     valid range: SOD_t_SERVICE
 * \param ps_obj       pointer to a SOD entry, see
 *                     {SOD_t_OBJECT} (pointer not checked,
 *                     only called with reference to struct
 *                     in SOD_Read() or SOD_Write())
 *                     valid range: pointer to a SOD_t_OBJECT
 * \param pv_data      pointer to data to be written, in case
 *                     of SOD_k_SRV_BEFORE_WRITE, otherwise
 *                     NULL (pointer not checked)
 *                     valid range: pointer to data to be
 *                     written, in case of
 *                     SOD_k_SRV_BEFORE_WRITE, otherwise NULL
 * \param dw_offset    start offset in bytes of the segment
 *                     within the data block (not used)
 * \param dw_size      size in bytes of the segment (not used)
 * \param pe_abortCode abort code has to be set for the SSDO if
 *                     the return value is FALSE.
 *                     (pointer not checked, only called with
 *                     reference to variable)
 *                     valid range: pointer to the
 *                     SOD_t_ABORT_CODES
 *
 * \return - TRUE  - success
 *         - FALSE - failure
 */
BOOLEAN SHNF_SOD_ConsTimeBase_CLBK(BYTE_B_INSTNUM_ SOD_t_SERVICE e_srvc,
                                   const SOD_t_OBJECT *ps_obj,
                                   const void *pv_data,
                                   UINT32 dw_offset, UINT32 dw_size,
                                   SOD_t_ABORT_CODES *pe_abortCode)
{
    BOOLEAN fReturn = FALSE;
    tTimerBase timeBase;

    /* The parameter set is handled in the process function in the background.
     * Therefore these function parameters are not used!
     */
    UNUSED_PARAMETER(e_srvc);
    UNUSED_PARAMETER(dw_offset);
    UNUSED_PARAMETER(pv_data);
    UNUSED_PARAMETER(dw_size);

    /* Check used parameters sanity */
    if( ps_obj == NULL              ||
        ps_obj->pv_objData == NULL  ||
        pe_abortCode == NULL         )
    {
        errh_postFatalError(kErrSourceSapl, kErrorInvalidParameter, 0);

        if(pe_abortCode != NULL)
        {
            *pe_abortCode = SOD_ABT_GENERAL_ERROR;
        }
    }
    else
    {
        /* Store parameter set details in global structure */
        timeBase = (tTimerBase)(*(INT8*)ps_obj->pv_objData);
        if(timer_setBase(timeBase) == 0)
        {
            *pe_abortCode   = SOD_ABT_NO_ERROR;
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

/* \} */
