/**
********************************************************************************
\file   main.c

\brief  Example application of the application interface lite

This module demonstrates an exemplary use of the libAppIflite library. It
initializes the library and all needed modules and sends/receives exemplary
data from and to the PCP.

\ingroup module_main
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

#include <libappif/appif.h>         // Header for the appif library

#include <apptarget/platform.h>     // Platform specific functions
#include <apptarget/app-gpio.h>     // Application of this demo
#include <apptarget/benchmark.h>    // Debug header for performance measurements

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

typedef struct {
    tTbufMemLayout     tbufMemLayout_m;                        ///< Local copy of the triple buffer memory
    UINT8              fShutdown_m;                            ///< Indicates CN shutdown
    UINT8              fCcWriteObjTestEnable_m;                ///< Enable periodic writing of a cc object
} tMainInstance;

//------------------------------------------------------------------------------
// local vars
//------------------------------------------------------------------------------

static tMainInstance mainInstance_l;            ///< Instance of main module

//------------------------------------------------------------------------------
// local function prototypes
//------------------------------------------------------------------------------
static BOOL appif_initModules(void);
static void appif_exitModules(void);
static void appif_genDescList(tBuffDescriptor* pBuffDescList_p, UINT8 buffCount_p);
static BOOL appif_appCbSync(tAppIfTimeStamp* pTimeStamp_p );
static BOOL appif_workInputOutput(UINT32 rpdoRelTimeLow_p,
        tRpdoMappedObj* pRpdoImage_p,
        tTpdoMappedObj* pTpdoImage_p );

#if defined(__NIOS2__) && !defined(ALT_ENHANCED_INTERRUPT_API_PRESENT)
    static void appif_syncIntH(void* pArg_p, void* dwInt_p);
#else
    static void appif_syncIntH(void* pArg_p);
#endif

static void appif_errorHandler(tErrorInfo* pErrorInfo_p);

#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_CC)) != 0)
static void appif_ccWriteObject(void);
static void appif_ccReadObject(void);
#endif

//============================================================================//
//            P U B L I C   F U N C T I O N S                                 //
//============================================================================//

//------------------------------------------------------------------------------
/**
\brief    Entry function of CN API example

main() implements the main program function of the CN API example.
First all initialization is done, then the program runs in a loop where the
APs state machine will be updated and input/output ports will be processed.

\return int
\retval 0          On successful shutdown

\ingroup module_main
*/
//------------------------------------------------------------------------------
int main (void)
{
    tAppIfInitParam     initParam;
    tBuffDescriptor     buffDescList[kTbufCount];

    APPIF_MEMSET(&mainInstance_l, 0, sizeof(mainInstance_l));
    APPIF_MEMSET(&buffDescList, 0, sizeof(buffDescList));

    platform_init();

    // Generate buffer descriptor list
    appif_genDescList(&buffDescList[0], kTbufCount);

    // Enable test of configuration channel
    mainInstance_l.fCcWriteObjTestEnable_m = TRUE;

    // initialize and start the application interface internals
    DEBUG_TRACE(DEBUG_LVL_ALWAYS,"\n\nInitialize application interface internals...\n");

    initParam.pBuffDescList_m = &buffDescList[0];
    initParam.pfnStreamHandler_m = platform_spiCommand;
    initParam.pfnErrorHandler_m = appif_errorHandler;
    initParam.idConsAck_m = kTbufAckRegisterCons;
    initParam.idProdAck_m = kTbufAckRegisterProd;
    initParam.idFirstProdBuffer_m = TBUF_NUM_CON + 1;   // Add one buffer for the consumer ACK register

    if(appif_init(&initParam) == FALSE)
    {
        DEBUG_TRACE(DEBUG_LVL_ERROR," ... error!\n");
        goto Exit;
    }
    DEBUG_TRACE(DEBUG_LVL_ALWAYS," ... ok!\n");

    // initialize and start the application interface modules
    DEBUG_TRACE(DEBUG_LVL_ALWAYS,"\n\nInitialize application interface modules...\n");
    if(appif_initModules() == FALSE)
    {
        DEBUG_TRACE(DEBUG_LVL_ERROR," ... error!\n");
        goto Exit;
    }
    DEBUG_TRACE(DEBUG_LVL_ALWAYS,"... ok!\n");

    /* initialize PCP interrupt handler*/
    DEBUG_TRACE(DEBUG_LVL_ALWAYS,"\n\nInitialize synchronous interrupt...\n");
    if(platform_initSyncInterrupt(appif_syncIntH) == FALSE)
    {
        DEBUG_TRACE(DEBUG_LVL_ERROR," ... error!\n");
        goto Exit;
    }
    DEBUG_TRACE(DEBUG_LVL_ALWAYS,"... ok!\n");

    /* Start periodic main loop */
    DEBUG_TRACE(DEBUG_LVL_ALWAYS,"\n\nApplication interface is running...\n");

    /* main program loop */
    while (TRUE)
    {
        if(appif_processAsync(NULL) == FALSE)
        {
            DEBUG_TRACE(DEBUG_LVL_ERROR,"ERROR: Unable to process background task!\n");
            break;
        }

#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_CC)) != 0)
        // Transmit async dummy frame for testing
        if(mainInstance_l.fCcWriteObjTestEnable_m != FALSE)
        {
            appif_ccWriteObject();
        }
#endif

        if(mainInstance_l.fShutdown_m != FALSE)
        {
            break;
        }
    }

Exit:
    DEBUG_TRACE(DEBUG_LVL_ALWAYS,"\n\nShutdown application interface modules...\n");
    appif_exitModules();
    DEBUG_TRACE(DEBUG_LVL_ALWAYS,"... ok!\n");

    DEBUG_TRACE(DEBUG_LVL_ALWAYS,"\n\nShutdown application interface internals...\n");
    appif_exit();
    DEBUG_TRACE(DEBUG_LVL_ALWAYS,"... ok!\n");

    return 0;
}

//============================================================================//
//            P R I V A T E   F U N C T I O N S                               //
//============================================================================//
/// \name Private Functions
/// \{


//------------------------------------------------------------------------------
/**
\brief    Initialize application interface modules

\return BOOL
\retval TRUE    Library module initialization successful
\retval FALSE   Error during library module initialization

\ingroup module_main
*/
//------------------------------------------------------------------------------
static BOOL appif_initModules(void)
{
    BOOL fReturn = FALSE;
    tStatusInitParam   statusInitParam;
#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_PDO)) != 0)
    tPdoInitParam      pdoInitParam;
#endif
#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_CC)) != 0)
    tCcInitParam       ccInitParam;
#endif

    // Initialize the status module
    statusInitParam.pfnAppCbSync_m = appif_appCbSync;
    statusInitParam.buffOutId_m = kTbufNumStatusOut;
    statusInitParam.buffInId_m = kTbufNumStatusIn;

    if(status_init(&statusInitParam) == FALSE)
    {
        DEBUG_TRACE(DEBUG_LVL_ERROR,"ERROR: status_init() failed!\n");
        goto Exit;
    }

#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_PDO)) != 0)
    // Initialize the pdo module
    pdoInitParam.buffIdRpdo_m = kTbufNumRpdoImage;
    pdoInitParam.buffIdTpdo_m = kTbufNumTpdoImage;

    if(pdo_init(appif_workInputOutput, &pdoInitParam) == FALSE)
    {
        DEBUG_TRACE(DEBUG_LVL_ERROR,"ERROR: pdo_init() failed!\n");
        goto Exit;
    }
#endif

#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_CC)) != 0)
    // Initialize configuration channel (CC)
    ccInitParam.iccId_m = kTbufNumInputConfChan;
    ccInitParam.occId_m = kTbufNumOutputConfChan;
    ccInitParam.pfnCritSec_p = platform_enterCriticalSection;

    if(cc_init(&ccInitParam) == FALSE)
    {
        DEBUG_TRACE(DEBUG_LVL_ERROR,"ERROR: cc_init() failed!\n");
        goto Exit;
    }
#endif

    // All module successfully initialized! -> Set return to success!
    fReturn = TRUE;

Exit:
    return fReturn;
}

//------------------------------------------------------------------------------
/**
\brief    Destroy application interface modules

\ingroup module_main
*/
//------------------------------------------------------------------------------
static void appif_exitModules(void)
{
    // Destroy all library modules
    status_exit();

#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_PDO)) != 0)
    pdo_exit();
#endif

#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_CC)) != 0)
    cc_exit();
#endif

}

//------------------------------------------------------------------------------
/**
\brief    Generate a list of buffer descriptors for the useage inside the library

\param[out] pBuffDescList_p     Buffer descriptor list in library format
\param[in]  buffCount_p         Count of buffer descriptors

\ingroup module_main
*/
//------------------------------------------------------------------------------
static void appif_genDescList(tBuffDescriptor* pBuffDescList_p, UINT8 buffCount_p)
{
    UINT8 i;
    tBuffDescriptor* pBuffDec = pBuffDescList_p;
    tTbufDescriptor tbufDescList[kTbufCount] = TBUF_INIT_VEC;

    // Generate a descriptor list which can be used in the library
    for(i=0; i < buffCount_p; i++, pBuffDec++)
    {
        pBuffDec->pBuffBase_m = (UINT8 *)((UINT32)&mainInstance_l.tbufMemLayout_m + (UINT32)tbufDescList[i].buffOffset_m);
        pBuffDec->buffSize_m = tbufDescList[i].buffSize_m;
    }

}


//------------------------------------------------------------------------------
/**
\brief    Application synchronization to the POWERLINK cycle

 \param[in] pTimeStamp_p             Time information of the current interrupt.

 \return TRUE

\ingroup module_main
*/
//------------------------------------------------------------------------------
static BOOL appif_appCbSync(tAppIfTimeStamp* pTimeStamp_p )
{

    // Note: This is your place to do local synchronization. Synchronize
    //       your local clock here by using the provided timestamp.

    return TRUE;
}

//------------------------------------------------------------------------------
/**
\brief    Process synchronous data

\param[in] rpdoRelTimeLow_p     Relative time low value
\param[in] pRpdoImage_p         Pointer to the RPDO objects
\param[in] pTpdoImage_p         Pointer to the TPDO objects

 \return TRUE

\ingroup module_main
*/
//------------------------------------------------------------------------------
static BOOL appif_workInputOutput(UINT32 rpdoRelTimeLow_p,
        tRpdoMappedObj* pRpdoImage_p,
        tTpdoMappedObj* pTpdoImage_p )
{
    UINT8 i;
    UINT32 outPort = 0;
    UINT8 inPort;

    // Digital IN: read push- and joystick buttons
    inPort = app_readInputPort();


    pTpdoImage_p->digitalOutput0 = inPort;
    pTpdoImage_p->digitalOutput1 = inPort;
    pTpdoImage_p->digitalOutput2 = inPort;
    pTpdoImage_p->digitalOutput3 = inPort;

    // Digital OUT: set Leds and hex digits
    for (i = 0; i < 3; i++)
    {
        if (i == 0) //first 8 bit of DigOut
        {
            // configured as output -> overwrite invalid input values with RPDO mapped variables
            outPort = (outPort & ~(0xff << (i * 8))) | (pRpdoImage_p->digitalInput0 << (i * 8));
        }
        else if (i == 1) //second 8 bit of DigOut
        {
            outPort = (outPort & ~(0xff << (i * 8))) | (pRpdoImage_p->digitalInput1 << (i * 8));
        }
        else if (i == 2)  //third 8 bit of DigOut
        {
            // configured as input -> store in TPDO mapped variable
            outPort = (outPort & ~(0xff << (i * 8))) | (pRpdoImage_p->digitalInput3 << (i * 8));
        }
    }

    app_writeOutputPort(outPort);

    return TRUE;
}

//------------------------------------------------------------------------------
/**
\brief    Synchronous interrupt handler

\param[in] pArg_p       Interrupt handler arguments

 appif_syncIntH() implements the synchronous data interrupt. The PCP asserts
 the interrupt when periodic data is ready to transfer.

\ingroup module_main
*/
//------------------------------------------------------------------------------
#if defined(__NIOS2__) && !defined(ALT_ENHANCED_INTERRUPT_API_PRESENT)
static void appif_syncIntH(void* pArg_p, void* dwInt_p)
#else
static void appif_syncIntH(void* pArg_p)
#endif
{
    BENCHMARK_MOD_01_SET(0);

    // Call internal synchronous process function
    if(appif_processSync() == FALSE)
    {
        DEBUG_TRACE(DEBUG_LVL_ERROR,"ERROR: Unable to process sync task!\n");
        mainInstance_l.fShutdown_m = TRUE;
    }

    platform_ackSyncIrq();

    BENCHMARK_MOD_01_RESET(0);

}

//------------------------------------------------------------------------------
/**
\brief    Error handler callback function

\param pErrorInfo_p     Pointer to the first error in the list

\ingroup module_main
*/
//------------------------------------------------------------------------------
static void appif_errorHandler(tErrorInfo* pErrorInfo_p)
{
        // Print error message
        DEBUG_TRACE(DEBUG_LVL_ERROR,"ERROR: Module origin: 0x%0x, Error Code: 0x%0x\n",
                        pErrorInfo_p->srcModule_m,
                        pErrorInfo_p->errCode_m);
}

#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_CC)) != 0)
//------------------------------------------------------------------------------
/**
\brief    Write an object of the configuration channel


\ingroup module_main
*/
//------------------------------------------------------------------------------
static void appif_ccWriteObject(void)
{
    tCcWriteStatus retCc = kCcWriteStatusError;
    // Create test object
    static tConfChanObject  object = { 0x2000, 0x1, 2, 0, 0};

    retCc = cc_writeObject(&object);
    if(retCc != kCcWriteStatusSuccessful && retCc != kCcWriteStatusBusy)
    {
        DEBUG_TRACE(DEBUG_LVL_ERROR,"ERROR: Unable to write object data to CC module!\n");
    }
    else if( retCc == kCcWriteStatusSuccessful)
    {
        appif_ccReadObject();

        object.objSubIdx_m++;
        if(object.objSubIdx_m > 4)
        {
            object.objSubIdx_m = 1;
        }

        object.objPayloadLow_m++;
    }
}

//------------------------------------------------------------------------------
/**
\brief    Read an object from the configuration channel


\ingroup module_main
*/
//------------------------------------------------------------------------------
static void appif_ccReadObject(void)
{
    // Create pointer to target object
    static tConfChanObject*  pObject;
    static UINT16 objIdx = 0x2000;
    static UINT8 objSubIdx = 0x1;

    pObject = cc_readObject(objIdx, objSubIdx);
    if(pObject == NULL)
    {
        DEBUG_TRACE(DEBUG_LVL_ERROR,"ERROR: Unable to read object data from CC module!\n");
    }
    else
    {
//        DEBUG_TRACE(DEBUG_LVL_ALWAYS,"INFO: Idx: 0x%x Sub: 0x%x Data: %d \n",
//                pObject->objIdx_m,
//                pObject->objSubIdx_m,
//                pObject->objPayloadLow_m);

        objSubIdx++;
        if(objSubIdx > 4)
        {
            objSubIdx = 1;
        }
    }
}

#endif

/// \}
