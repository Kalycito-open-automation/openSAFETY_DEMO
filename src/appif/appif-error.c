/**
********************************************************************************
\file   appif-error.c

\brief  Library internal error handler

Application interface error handler module. Handles each occurred error and
forwards a trace to the user layer.

\ingroup module_error
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
#include <appif/appif-errorint.h>

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
* \brief Error handler user instance type
*/
typedef struct
{
    tErrorHandler   pfnErrorHandler_m;    ///< Error module error handler
} tErrorInstance;

//------------------------------------------------------------------------------
// local vars
//------------------------------------------------------------------------------

static tErrorInstance        errorInstance_l;

//------------------------------------------------------------------------------
// local function prototypes
//------------------------------------------------------------------------------


//============================================================================//
//            P U B L I C   F U N C T I O N S                                 //
//============================================================================//

//------------------------------------------------------------------------------
/**
\brief    Initialize the error handler module

\return BOOL
\retval TRUE         Module initialization successful
\retval FALSE        Error on module initialization

\ingroup module_error
*/
//------------------------------------------------------------------------------
BOOL error_init(tErrorHandler pfnErrorHandler_p)
{
    BOOL fReturn = FALSE;

    APPIF_MEMSET(&errorInstance_l, 0, sizeof(tErrorInstance));

    // Register error handler
    if(pfnErrorHandler_p != NULL)
    {
        errorInstance_l.pfnErrorHandler_m = pfnErrorHandler_p;
    }

    // Succeed even if error handler is not set
    fReturn = TRUE;

    return fReturn;
}

//------------------------------------------------------------------------------
/**
\brief    Destroy the error handler module

\ingroup module_error
*/
//------------------------------------------------------------------------------
void error_exit(void)
{

}

//------------------------------------------------------------------------------
/**
\brief    Set error to error handler module

\param  srcModule_p     Module source of the error
\param  errCode_p       Code of the error

\ingroup module_error
*/
//------------------------------------------------------------------------------
void error_setError(tAppIfModules srcModule_p, tAppIfStatus errCode_p)
{
    tErrorInfo errInfo;

    if(errorInstance_l.pfnErrorHandler_m != NULL)
    {
        errInfo.srcModule_m = srcModule_p;
        errInfo.errCode_m = errCode_p;

        // Call error handler to inform application
        errorInstance_l.pfnErrorHandler_m(&errInfo);
    }

}

//============================================================================//
//            P R I V A T E   F U N C T I O N S                               //
//============================================================================//
/// \name Private Functions
/// \{

/// \}

