/**
********************************************************************************
\file   TSTinternal.c

\brief  Test internal module successful and failed initializations

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

#include <Driver/TSTinternalConfig.h>
#include <Stubs/STBdescList.h>
#include <Stubs/STBdummyHandler.h>
#include <Stubs/STBinitInternal.h>


#include <libappif/appif.h>

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
\brief Test error module

\ingroup module_unittests
*/
//------------------------------------------------------------------------------
void TST_internalInit(void)
{
    BOOL fReturn;
    tAppIfInitParam appifInit;

    APPIF_MEMSET(&appifInit, 0, sizeof(tAppIfInitParam));

    // Initialize with no parameters
    fReturn = appif_init(NULL);

    CU_ASSERT_FALSE( fReturn );

    // Initialize with wrong set parameters
    fReturn = appif_init(&appifInit);

    CU_ASSERT_FALSE( fReturn );

    // Init with wrong consumer id
    appifInit.idConsAck_m = kTbufCount;
    appifInit.idProdAck_m = kTbufAckRegisterProd;

    fReturn = appif_init(&appifInit);

    CU_ASSERT_FALSE( fReturn );

    // Init with wrong producer id
    appifInit.idConsAck_m = kTbufAckRegisterCons;
    appifInit.idProdAck_m = kTbufCount;

    fReturn = appif_init(&appifInit);

    CU_ASSERT_FALSE( fReturn );

    // Initialize with failing stream module
    appifInit.idConsAck_m = kTbufAckRegisterCons;
    appifInit.idProdAck_m = kTbufAckRegisterProd;

    fReturn = appif_init(&appifInit);

    CU_ASSERT_FALSE( fReturn );

    // Initialize internal module successfully
    stb_initBuffers();

    fReturn = stb_initInternalModule(stb_streamHandlerSuccess);

    CU_ASSERT_TRUE( fReturn );

    appif_exit();
}

void TST_initAckConsRegInvalidBase(void)
{
    BOOL fReturn = FALSE;

    stb_initDescriptorsChangeSelected(kTbufAckRegisterCons, NULL, sizeof(tTbufAckRegister));

    fReturn = stb_initInternalModule(stb_streamHandlerSuccess);

    CU_ASSERT_FALSE( fReturn );

    appif_exit();
}

void TST_initAckConsRegInvalidSize(void)
{
    BOOL fReturn = FALSE;

    tTbufAckRegister ackReg;

    stb_initDescriptorsChangeSelected(kTbufAckRegisterCons, (UINT8 *)&ackReg, 0);

    fReturn = stb_initInternalModule(stb_streamHandlerSuccess);

    CU_ASSERT_FALSE( fReturn );

    appif_exit();
}

void TST_initAckProdRegInvalidBase(void)
{
    BOOL fReturn = FALSE;

    stb_initDescriptorsChangeSelected(kTbufAckRegisterProd, NULL, sizeof(tTbufAckRegister));

    fReturn = stb_initInternalModule(stb_streamHandlerSuccess);

    CU_ASSERT_FALSE( fReturn );

    appif_exit();
}

void TST_initAckProdRegInvalidSize(void)
{
    BOOL fReturn = FALSE;

    tTbufAckRegister ackReg;

    stb_initDescriptorsChangeSelected(kTbufAckRegisterProd, (UINT8 *)&ackReg, 0);

    fReturn = stb_initInternalModule(stb_streamHandlerSuccess);

    CU_ASSERT_FALSE( fReturn );

    appif_exit();
}

void TST_internalProcessSync(void)
{
    BOOL fReturn;

    stb_initBuffers();

    // Init valid internal module
    fReturn = stb_initInternalModule(stb_streamHandlerSuccess);

    CU_ASSERT_TRUE_FATAL( fReturn );

    fReturn = appif_processSync();

    CU_ASSERT_TRUE( fReturn );

    // Init with invalid stream handler
    fReturn = stb_initInternalModule(stb_streamHandlerFail);

    CU_ASSERT_TRUE_FATAL( fReturn );

    fReturn = appif_processSync();

    CU_ASSERT_FALSE( fReturn );

    // Close module
    appif_exit();
}

void TST_internalProcessAsync(void)
{
    BOOL fReturn;

    stb_initBuffers();

    // Init valid internal module
    fReturn = stb_initInternalModule(stb_streamHandlerSuccess);

    CU_ASSERT_TRUE_FATAL( fReturn );

    fReturn = appif_processAsync(NULL);

#if (((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_SSDO)) != 0)
    CU_ASSERT_FALSE( fReturn );
#else
    CU_ASSERT_TRUE( fReturn );
#endif
}

//============================================================================//
//            P R I V A T E   F U N C T I O N S                               //
//============================================================================//
/// \name Private Functions
/// \{

/// \}