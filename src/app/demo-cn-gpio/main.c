/**
********************************************************************************
\file   main.c

\brief  Example application of the POWERLINK slim interface basic CN demo

This module demonstrates an exemplary use of the libpsi library as a simple
input/output GPIO demo. It initializes the library and all needed modules and
sends/receives exemplary data from and to the PCP.

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

/*----------------------------------------------------------------------------*/
/* includes                                                                   */
/*----------------------------------------------------------------------------*/

#include <libpsi/psi.h>         /* Header for the psi library */

#include <common/platform.h>      /* Interface header to the platform specific functions */
#include <common/pcpserial.h>     /* Interface header to the platform specific serial device */
#include <common/syncir.h>        /* Interface header to initialize the synchronous interrupt */

#include <common/benchmark.h>     /* Debug header for performance measurements */
#include <common/debug.h>
#include <common/tbufparams.h>
#include <powerlink.h>

#include <cn/app-gpio.h>          /* Interface header to the application of this demo */


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

typedef struct {
    volatile UINT8 tbufMemLayout_m[TBUF_IMAGE_SIZE];    /**< Local copy of the triple buffer memory */
    UINT8 fShutdown_m;                                  /**< Indicates CN shutdown */
    UINT8 fCcWriteObjTestEnable_m;                      /**< Enable periodic writing of a cc object */
} tMainInstance;

/*----------------------------------------------------------------------------*/
/* local vars                                                                 */
/*----------------------------------------------------------------------------*/

static tMainInstance mainInstance_l;            /**< Instance of main module */

/*----------------------------------------------------------------------------*/
/* local function prototypes                                                  */
/*----------------------------------------------------------------------------*/
static BOOL psi_initModules(void);
static void psi_exitModules(void);
static BOOL psi_appCbSync(tPsiTimeStamp* pTimeStamp_p );
static BOOL psi_workInputOutput(UINT32 rpdoRelTimeLow_p,
        tRpdoMappedObj* pRpdoImage_p,
        tTpdoMappedObj* pTpdoImage_p );

static void psi_syncIntH(void* pArg_p);
static void psi_serialTransferFinished(BOOL fError_p);

static void psi_errorHandler(tPsiErrorInfo* pErrorInfo_p);

#if(((PSI_MODULE_INTEGRATION) & (PSI_MODULE_CC)) != 0)
static void psi_ccWriteObject(void);
static void psi_ccReadObject(void);
#endif

/*============================================================================*/
/*            P U B L I C   F U N C T I O N S                                 */
/*============================================================================*/

/*----------------------------------------------------------------------------*/
/**
\brief    Entry function of CN API example

main() implements the main program function of the CN API example.
First all initialization is done, then the program runs in a loop where the
APs state machine will be updated and input/output ports will be processed.

\return int
\retval 0          On successful shutdown

\ingroup module_main
*/
/*----------------------------------------------------------------------------*/
int main (void)
{
    tPsiInitParam     initParam;
    tBuffDescriptor   buffDescList[kTbufCount];
    tHandlerParam     transferParam;
    UINT8 * pTbufMemBase = (UINT8 *)(&mainInstance_l.tbufMemLayout_m[0]);
    UINT8             macAddr[] = MAC_ADDR;

    PSI_MEMSET(&mainInstance_l, 0, sizeof(mainInstance_l));
    PSI_MEMSET(&buffDescList, 0, sizeof(buffDescList));
    PSI_MEMSET(&transferParam, 0, sizeof(tHandlerParam));

    /* Init the target platform */
    platform_init();

    DEBUG_TRACE(DEBUG_LVL_ALWAYS, "\n\n********************************************************************\n");
    DEBUG_TRACE(DEBUG_LVL_ALWAYS, "\n\topenPOWERLINK - GPIO Demo \n\n ");
    DEBUG_TRACE(DEBUG_LVL_ALWAYS, "\tSerial number: \t\t0x%X\n", CONFIG_IDENT_SERIAL_NUMBER );
    DEBUG_TRACE(DEBUG_LVL_ALWAYS, "\tVendor id: \t\t0x%X\n", CONFIG_IDENT_VENDOR_ID );
    DEBUG_TRACE(DEBUG_LVL_ALWAYS, "\tMAC address: \t\t0x%X,0x%X,0x%X,0x%X,0x%X,0x%X\n",
                                   macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5] );
    DEBUG_TRACE(DEBUG_LVL_ALWAYS, "********************************************************************\n");

    /* Generate buffer descriptor list */
    if(tbufp_genDescList(pTbufMemBase, kTbufCount, &buffDescList[0]) == FALSE)
    {
        DEBUG_TRACE(DEBUG_LVL_ERROR," ERROR: Unable to generate tbuf descriptor list!\n");
        goto Exit;
    }

    /* Enable test of configuration channel */
    mainInstance_l.fCcWriteObjTestEnable_m = TRUE;

    /* initialize and start the slim interface internals */
    DEBUG_TRACE(DEBUG_LVL_ALWAYS,"\n\nInitialize slim interface internals...\n");

    initParam.pBuffDescList_m = &buffDescList[0];
    initParam.pfnStreamHandler_m = pcpserial_transfer;
    initParam.pfnErrorHandler_m = psi_errorHandler;
    initParam.idConsAck_m = kTbufAckRegisterCons;
    initParam.idProdAck_m = kTbufAckRegisterProd;
    initParam.idFirstProdBuffer_m = TBUF_NUM_CON + 1;   /* Add one buffer for the consumer ACK register */

    if(psi_init(&initParam) == FALSE)
    {
        DEBUG_TRACE(DEBUG_LVL_ERROR," ... error!\n");
        goto Exit;
    }
    DEBUG_TRACE(DEBUG_LVL_ALWAYS," ... ok!\n");

    /* initialize and start the slim interface modules */
    DEBUG_TRACE(DEBUG_LVL_ALWAYS,"\n\nInitialize slim interface modules...\n");
    if(psi_initModules() == FALSE)
    {
        DEBUG_TRACE(DEBUG_LVL_ERROR," ... error!\n");
        goto Exit;
    }
    DEBUG_TRACE(DEBUG_LVL_ALWAYS,"... ok!\n");

    /* Setup consumer/producer transfer parameters with initialization fields */
    if(tbufp_genTransferParams(pTbufMemBase, &transferParam) == FALSE)
    {
        DEBUG_TRACE(DEBUG_LVL_ERROR,"ERROR: Unable to generate transfer parameters!\n");
        goto Exit;
    }

    /* Init the serial device */
    DEBUG_TRACE(DEBUG_LVL_ALWAYS,"\n\nInitialize serial device...\n");
    if(pcpserial_init(&transferParam, psi_serialTransferFinished) == FALSE)
    {
        DEBUG_TRACE(DEBUG_LVL_ERROR," ... error!\n");
        goto Exit;
    }
    DEBUG_TRACE(DEBUG_LVL_ALWAYS,"... ok!\n");


    /* initialize PCP interrupt handler */
    DEBUG_TRACE(DEBUG_LVL_ALWAYS,"\n\nInitialize synchronous interrupt...\n");
    if(syncir_init(psi_syncIntH) == FALSE)
    {
        DEBUG_TRACE(DEBUG_LVL_ERROR," ... error!\n");
        goto Exit;
    }
    DEBUG_TRACE(DEBUG_LVL_ALWAYS,"... ok!\n");

    /* Now enable the synchronous interrupt */
    syncir_enable();

    /* Start periodic main loop */
    DEBUG_TRACE(DEBUG_LVL_ALWAYS,"\n\nStart processing...\n");

    while (TRUE)
    {
        if(psi_processAsync() == FALSE)
        {
            DEBUG_TRACE(DEBUG_LVL_ERROR,"ERROR: Unable to process background task!\n");
            break;
        }

#if(((PSI_MODULE_INTEGRATION) & (PSI_MODULE_CC)) != 0)
        /* Transmit async dummy frame for testing */
        if(mainInstance_l.fCcWriteObjTestEnable_m != FALSE)
        {
            psi_ccWriteObject();
        }
#endif

        if(mainInstance_l.fShutdown_m != FALSE)
        {
            break;
        }
    }

Exit:
    DEBUG_TRACE(DEBUG_LVL_ALWAYS,"\n\nPerform system shutdown...\n");

    /* Shutdown platform specific parts */
    syncir_exit();
    pcpserial_exit();
    platform_exit();

    /* Shutdown slim interface */
    psi_exitModules();
    psi_exit();

    return 0;
}

/*============================================================================*/
/*            P R I V A T E   F U N C T I O N S                               */
/*============================================================================*/
/* \name Private Functions */
/* \{ */

/*----------------------------------------------------------------------------*/
/**
\brief    Initialize slim interface modules

\return BOOL
\retval TRUE    Library module initialization successful
\retval FALSE   Error during library module initialization

\ingroup module_main
*/
/*----------------------------------------------------------------------------*/
static BOOL psi_initModules(void)
{
    BOOL fReturn = FALSE;
    tStatusInitParam   statusInitParam;
#if(((PSI_MODULE_INTEGRATION) & (PSI_MODULE_PDO)) != 0)
    tPdoInitParam      pdoInitParam;
#endif
#if(((PSI_MODULE_INTEGRATION) & (PSI_MODULE_CC)) != 0)
    tCcInitParam       ccInitParam;
#endif

    /* Initialize the status module */
    statusInitParam.pfnAppCbSync_m = psi_appCbSync;
    statusInitParam.buffOutId_m = kTbufNumStatusOut;
    statusInitParam.buffInId_m = kTbufNumStatusIn;

    if(status_init(&statusInitParam) == FALSE)
    {
        DEBUG_TRACE(DEBUG_LVL_ERROR,"ERROR: status_init() failed!\n");
        goto Exit;
    }

#if(((PSI_MODULE_INTEGRATION) & (PSI_MODULE_PDO)) != 0)
    /* Initialize the pdo module */
    pdoInitParam.buffIdRpdo_m = kTbufNumRpdoImage;
    pdoInitParam.buffIdTpdo_m = kTbufNumTpdoImage;

    if(pdo_init(psi_workInputOutput, &pdoInitParam) == FALSE)
    {
        DEBUG_TRACE(DEBUG_LVL_ERROR,"ERROR: pdo_init() failed!\n");
        goto Exit;
    }
#endif

#if(((PSI_MODULE_INTEGRATION) & (PSI_MODULE_CC)) != 0)
    /* Initialize configuration channel (CC) */
    ccInitParam.iccId_m = kTbufNumInputConfChan;
    ccInitParam.occId_m = kTbufNumOutputConfChan;
    ccInitParam.pfnCritSec_p = syncir_enterCriticalSection;

    if(cc_init(&ccInitParam) == FALSE)
    {
        DEBUG_TRACE(DEBUG_LVL_ERROR,"ERROR: cc_init() failed!\n");
        goto Exit;
    }
#endif

    /* All module successfully initialized! -> Set return to success! */
    fReturn = TRUE;

Exit:
    return fReturn;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Destroy slim interface modules

\ingroup module_main
*/
/*----------------------------------------------------------------------------*/
static void psi_exitModules(void)
{
    /* Destroy all library modules */
    status_exit();

#if(((PSI_MODULE_INTEGRATION) & (PSI_MODULE_PDO)) != 0)
    pdo_exit();
#endif

#if(((PSI_MODULE_INTEGRATION) & (PSI_MODULE_CC)) != 0)
    cc_exit();
#endif

}

/*----------------------------------------------------------------------------*/
/**
\brief    Application synchronization to the POWERLINK cycle

 \param[in] pTimeStamp_p             Time information of the current interrupt.

 \return TRUE

\ingroup module_main
*/
/*----------------------------------------------------------------------------*/
static BOOL psi_appCbSync(tPsiTimeStamp* pTimeStamp_p )
{
    UNUSED_PARAMETER(pTimeStamp_p);

    /* Note: This is your place to do local synchronization. Synchronize */
    /*       your local clock here by using the provided timestamp. */

    return TRUE;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Process synchronous data

\param[in] rpdoRelTimeLow_p     Relative time low value
\param[in] pRpdoImage_p         Pointer to the RPDO objects
\param[in] pTpdoImage_p         Pointer to the TPDO objects

 \return TRUE

\ingroup module_main
*/
/*----------------------------------------------------------------------------*/
static BOOL psi_workInputOutput(UINT32 rpdoRelTimeLow_p,
        tRpdoMappedObj* pRpdoImage_p,
        tTpdoMappedObj* pTpdoImage_p )
{
    UINT8 i;
    UINT32 outPort = 0;
    UINT8 inPort;

    UNUSED_PARAMETER(rpdoRelTimeLow_p);

    /* Digital IN: read push- and joystick buttons */
    inPort = app_readInputPort();


    pTpdoImage_p->digitalOutput0 = inPort;
    pTpdoImage_p->digitalOutput1 = inPort;
    pTpdoImage_p->digitalOutput2 = inPort;
    pTpdoImage_p->digitalOutput3 = inPort;

    /* Digital OUT: set Leds and hex digits */
    for (i = 0; i < 3; i++)
    {
        if (i == 0) /* first 8 bit of DigOut */
        {
            /* configured as output -> overwrite invalid input values with RPDO mapped variables */
            outPort = (outPort & ~(0xff << (i * 8))) | (pRpdoImage_p->digitalInput0 << (i * 8));
        }
        else if (i == 1) /* second 8 bit of DigOut */
        {
            outPort = (outPort & ~(0xff << (i * 8))) | (pRpdoImage_p->digitalInput1 << (i * 8));
        }
        else if (i == 2)  /* third 8 bit of DigOut */
        {
            /* configured as input -> store in TPDO mapped variable */
            outPort = (outPort & ~(0xff << (i * 8))) | (pRpdoImage_p->digitalInput3 << (i * 8));
        }
    }

    app_writeOutputPort(outPort);

    return TRUE;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Synchronous interrupt handler

\param[in] pArg_p       Interrupt handler arguments

 psi_syncIntH() implements the synchronous data interrupt. The PCP asserts
 the interrupt when periodic data is ready to transfer.

\ingroup module_main
*/
/*----------------------------------------------------------------------------*/
static void psi_syncIntH(void* pArg_p)
{
    UNUSED_PARAMETER(pArg_p);

    BENCHMARK_MOD_01_SET(0);

    /* Call internal synchronous process function */
    if(psi_processSync() == FALSE)
    {
        DEBUG_TRACE(DEBUG_LVL_ERROR,"ERROR: Unable to process sync task!\n");
        mainInstance_l.fShutdown_m = TRUE;
    }

    syncir_acknowledge();

    BENCHMARK_MOD_01_RESET(0);

}


/*----------------------------------------------------------------------------*/
/**
\brief    Serial transfer finished callback function

This function is called after a serial transfer from the PCP to the application.

\param[in] fError_p       True if the transfer had an error

\ingroup module_hnf
*/
/*----------------------------------------------------------------------------*/
static void psi_serialTransferFinished(BOOL fError_p)
{
    if(fError_p == FALSE)
    {
        BENCHMARK_MOD_01_SET(2);

        /* Transfer finished -> Call all post action tasks */
        if(psi_processPostTransferActions() == FALSE)
        {
            DEBUG_TRACE(DEBUG_LVL_ERROR,"ERROR: Unable to process post actions!\n");
            mainInstance_l.fShutdown_m = TRUE;
        }

        BENCHMARK_MOD_01_RESET(2);
    }
    else
    {
        /* There was an error during the serial transfer */
        DEBUG_TRACE(DEBUG_LVL_ERROR,"ERROR: Serial transfer error!\n");
        mainInstance_l.fShutdown_m = TRUE;
    }
}

/*----------------------------------------------------------------------------*/
/**
\brief    Error handler callback function

\param pErrorInfo_p     Pointer to the first error in the list

\ingroup module_main
*/
/*----------------------------------------------------------------------------*/
static void psi_errorHandler(tPsiErrorInfo* pErrorInfo_p)
{
#ifdef NDEBUG
    UNUSED_PARAMETER(pErrorInfo_p);
#else
    /* Print error message */
    DEBUG_TRACE(DEBUG_LVL_ERROR,"ERROR: Module origin: 0x%0x, Error Code: 0x%0x\n",
                    pErrorInfo_p->srcModule_m,
                    pErrorInfo_p->errCode_m);
#endif /* #ifdef NDEBUG */
}

#if(((PSI_MODULE_INTEGRATION) & (PSI_MODULE_CC)) != 0)
/*----------------------------------------------------------------------------*/
/**
\brief    Write an object of the configuration channel


\ingroup module_main
*/
/*----------------------------------------------------------------------------*/
static void psi_ccWriteObject(void)
{
    tCcWriteStatus retCc = kCcWriteStatusError;
    /* Create test object */
    static tConfChanObject  object = { 0x2000, 0x1, 2, 0, 0};

    retCc = cc_writeObject(&object);
    if(retCc != kCcWriteStatusSuccessful && retCc != kCcWriteStatusBusy)
    {
        DEBUG_TRACE(DEBUG_LVL_ERROR,"ERROR: Unable to write object data to CC module!\n");
    }
    else if( retCc == kCcWriteStatusSuccessful)
    {
        psi_ccReadObject();

        object.objSubIdx_m++;
        if(object.objSubIdx_m > 4)
        {
            object.objSubIdx_m = 1;
        }

        object.objPayloadLow_m++;
    }
}

/*----------------------------------------------------------------------------*/
/**
\brief    Read an object from the configuration channel


\ingroup module_main
*/
/*----------------------------------------------------------------------------*/
static void psi_ccReadObject(void)
{
    /* Create pointer to target object */
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
/*        DEBUG_TRACE(DEBUG_LVL_ALWAYS,"INFO: Idx: 0x%x Sub: 0x%x Data: %d \n",
                  pObject->objIdx_m,
                  pObject->objSubIdx_m,
                  pObject->objPayloadLow_m); */

        objSubIdx++;
        if(objSubIdx > 4)
        {
            objSubIdx = 1;
        }
    }
}

#endif

/* \} */
