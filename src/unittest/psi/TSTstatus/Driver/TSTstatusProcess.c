/**
********************************************************************************
\file   TSTstatusProcess.c

\brief  Test status module init and processing with correct stream init

\ingroup module_unittests
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
#include <cunit/CUnit.h>

#include <Driver/TSTstatusConfig.h>
#include <Stubs/STBdescList.h>
#include <Stubs/STBinitStream.h>
#include <Stubs/STBdummyHandler.h>

#include <libpsi/internal/status.h>
#include <libpsi/internal/stream.h>


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
\brief    Init with valid parameters

\return int
\retval 0       Init successful
\retval other   Init failed

\ingroup module_unittests
*/
//------------------------------------------------------------------------------
int TST_validInit(void)
{
    BOOL fReturn;

    stb_initBuffers();

    // Call init successfully
    fReturn = stb_initStreamModule();

    return (fReturn != FALSE) ? 0 : 1;
}

//------------------------------------------------------------------------------
/**
\brief Test status module initialization function

\ingroup module_unittests
*/
//------------------------------------------------------------------------------
void TST_statusInit(void)
{
    BOOL fReturn;
    tStatusInitParam InitParam;

    PSI_MEMSET(&InitParam, 0, sizeof(tStatusInitParam));

    // Test init with no parameter
    fReturn = status_init(NULL);

    CU_ASSERT_FALSE( fReturn );

    // Test init with sync handler not set
    fReturn = status_init(&InitParam);

    CU_ASSERT_FALSE( fReturn );

    // Test with invalid in buffer id
    InitParam.pfnProcSyncCb_m = stb_dummySyncHandlerSuccess;
    InitParam.buffInId_m = kTbufCount;

    fReturn = status_init(&InitParam);

    CU_ASSERT_FALSE( fReturn );

    // Test with invalid out buffer id
    InitParam.pfnProcSyncCb_m = stb_dummySyncHandlerSuccess;
    InitParam.buffInId_m = kTbufNumStatusIn;
    InitParam.buffOutId_m = kTbufCount;

    fReturn = status_init(&InitParam);

    CU_ASSERT_FALSE( fReturn );

    // Test initialization successful
    InitParam.pfnProcSyncCb_m = stb_dummySyncHandlerSuccess;
    InitParam.buffInId_m = kTbufNumStatusIn;
    InitParam.buffOutId_m = kTbufNumStatusOut;

    fReturn = status_init(&InitParam);

    CU_ASSERT_TRUE( fReturn );

    // Call exit dummy function
    status_exit();
}

//------------------------------------------------------------------------------
/**
\brief Process status module with valid initialization

\ingroup module_unittests
*/
//------------------------------------------------------------------------------
void TST_statusProcess(void)
{
    BOOL fReturn;

    fReturn = stream_processSync();

    CU_ASSERT_TRUE( fReturn );
}

//------------------------------------------------------------------------------
/**
\brief Process status module with failing sync handler

\ingroup module_unittests
*/
//------------------------------------------------------------------------------
void TST_statusProcessSyncFail(void)
{
    BOOL fReturn;
    tStatusInitParam InitParam;

    PSI_MEMSET(&InitParam, 0, sizeof(tStatusInitParam));

     // Test initialization with failing sync handler
    InitParam.pfnProcSyncCb_m = stb_dummySyncHandlerFail;
    InitParam.buffInId_m = kTbufNumStatusIn;
    InitParam.buffOutId_m = kTbufNumStatusOut;

    fReturn = status_init(&InitParam);

    CU_ASSERT_TRUE( fReturn );

    // Process with failing sync handler
    fReturn = stream_processSync();

    CU_ASSERT_FALSE( fReturn );
}

//============================================================================//
//            P R I V A T E   F U N C T I O N S                               //
//============================================================================//
/// \name Private Functions
/// \{

/// \}