/**
********************************************************************************
\file       main.c

\brief      Main module of AppIf interface

The main module of the application interface provides the gateway from the
openPOWERLINK stack to the application interface. It forwards data to the
buffers which can be accessed by the application processor.

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

#include <appif/appif.h>
#include <appif/status.h>

#include <Epl.h>
#include <user/pdou.h>

#include <event.h>


//------------------------------------------------------------------------------
// defines
//------------------------------------------------------------------------------
#define NODEID      0x01         ///< Initial NodeId. Should be NOT 0xF0 (=MN) in case of CN

#define CYCLE_LEN   1000                                ///< length of the cycle [us]
#define MAC_ADDR    0x00, 0x12, 0x34, 0x56, 0x78, 0x9A  ///< MAC address of the CN
#define IP_ADDR     0xc0a86401                          ///< IP-Address 192.168.100.1 (Object: 0x1E40/0x02) (don't care the last byte!)
#define SUBNET_MASK 0xFFFFFF00                          ///< Subnet mask 255.255.255.0 (Object: 0x1E40/0x03)
#define DEF_GATEWAY 0xc0a864f0                          ///< Default gateway: 192.168.100.254 (Object: 0x1E40/0x05)
#define HOST_NAME   "openPOWERLINK CN"                  ///< The hostname of the CN (Object: 0x1F9A/0x00)

#define CONFIG_ISOCHR_TX_MAX_PAYLOAD    max(40, sizeof(tTpdoMappedObj));
#define CONFIG_ISOCHR_RX_MAX_PAYLOAD    1496
#define CONFIG_IDENT_DEVICE_TYPE        -1              ///< Device type of the node
#define CONFIG_IDENT_VENDOR_ID          0x00000000      ///< Vendor ID of the node
#define CONFIG_IDENT_PRODUCT_CODE       0x00            ///< Product code of the node
#define CONFIG_IDENT_REVISION           0x00010020      ///< Revision number of the node
#define CONFIG_IDENT_SERIAL_NUMBER      0x00000000      ///< Serial number of the node

#define MAC_ADDR_LAST_BYTE      5                       ///< Position of the last MAC address byte

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
 * \brief Main module instance structure
 */
typedef struct
{
    UINT8         aMacAddr[6];       ///< MAC address
    UINT8         sHostName[33];     ///< The hostname of the node
    UINT8         nodeId;            ///< Node ID
    UINT32        subnetMask;        ///< The subnet mask of the node
    UINT32        defGateway;        ///< The default gateway of the node

    UINT8         fShutdown;         ///< Initialize CN shutdown sequence
    UINT32        cycleTime;         ///< Cycle time of the CN (set in kEplNmtGsResetConfiguration)
    tNmtState     plkState;          ///< The current state of the CN
} tMainInstance;

//------------------------------------------------------------------------------
// local vars
//------------------------------------------------------------------------------

static tMainInstance mainInstance_l;

//------------------------------------------------------------------------------
// local function prototypes
//------------------------------------------------------------------------------
static tAppIfStatus appif_initPlk(tMainInstance* pInstance_p);
static tAppIfStatus appif_processPlk(tMainInstance* pInstance_p);
static void appif_switchOffPlk(void);
static void appif_enterCriticalSection(UINT8 fEnable_p);
static tEplKernel appif_userEventCb(tEplApiEventType EventType_p,
                                   tEplApiEventArg* pEventArg_p,
                                   void* pUserArg_p);

static tEplKernel PUBLIC appif_syncCb(tSocTimeStamp* socTimeStamp_p) SECTION_MAIN_APP_CB_SYNC;

//------------------------------------------------------------------------------
// private functions
//------------------------------------------------------------------------------


//============================================================================//
//            P U B L I C   F U N C T I O N S                                 //
//============================================================================//

//------------------------------------------------------------------------------
/**
\brief    Entry point of the program

Initializes the application interface and the openPOWERLINK stack.

\return int
\retval 0          Successful termination
\retval Other      On error

\ingroup module_main
*/
//------------------------------------------------------------------------------
int main (void)
{
    UINT8    nodeId;
    tAppIfStatus ret = kAppIfSuccessful;
    const UINT8 aMacAddr[] = {MAC_ADDR};
    const UINT8 sHostname[] = {HOST_NAME};

    // reset local instance of main
    APPIF_MEMSET(&mainInstance_l, 0 , sizeof(tMainInstance));

    //Set state of stack to invalid value
    mainInstance_l.plkState = 0xFFFF;

    PRINTF("----------------------------------------------------------\n");
    PRINTF("Starting openPOWERLINK with external application interface\n");
    PRINTF("----------------------------------------------------------\n");

    target_init();

    // Read node id from switches
    nodeId = target_getNodeid();
    if(nodeId == 0)
    {
        nodeId = NODEID;
    }

    PRINTF("Initialize application interface...\n");
    ret = appif_init(nodeId, appif_enterCriticalSection);
    if(ret != kAppIfSuccessful)
    {
        goto Exit;
    }

    initEvents(&appif_userEventCb);

    // Set POWERLINK stack initialization parameters
    mainInstance_l.cycleTime    = CYCLE_LEN;
    mainInstance_l.subnetMask   = SUBNET_MASK;
    mainInstance_l.defGateway   = DEF_GATEWAY;
    mainInstance_l.nodeId       = (nodeId != 0) ? nodeId : NODEID;
    mainInstance_l.fShutdown    = FALSE;

    // set mac address (last byte is set to node ID)
    EPL_MEMCPY(mainInstance_l.aMacAddr, aMacAddr, sizeof(aMacAddr));
    mainInstance_l.aMacAddr[MAC_ADDR_LAST_BYTE] = mainInstance_l.nodeId;

    // setup the hostname
    EPL_MEMCPY(mainInstance_l.sHostName, sHostname, sizeof(sHostname));

    // Initialize the POWERLINK stack
    ret = appif_initPlk(&mainInstance_l);
    if(ret != kAppIfSuccessful)
    {
        goto Exit;
    }

    PRINTF("Configure application interface...\n");
    // Configure the application interface modules
    ret = appif_configureModules();
    if(ret != kAppIfSuccessful)
    {
        goto ExitShutdown;
    }

    // Process POWERLINK background task
    ret = appif_processPlk(&mainInstance_l);
    if(ret != kAppIfSuccessful)
    {
        goto ExitShutdown;
    }

ExitShutdown:
    // Shutdown POWERLINK stack
    oplk_shutdown();

Exit:
    appif_exit();

    target_cleanup();

    return 0;
}

//============================================================================//
//            P R I V A T E   F U N C T I O N S                               //
//============================================================================//
/// \name Private Functions
/// \{


//------------------------------------------------------------------------------
/**
\brief  Initialize the openPOWERLINK stack

The function initializes the openPOWERLINK stack.

\param  pInstance_p             Pointer to demo instance

\return tAppIfStatus
\retval
*/
//------------------------------------------------------------------------------
static tAppIfStatus appif_initPlk(tMainInstance* pInstance_p)
{
    tAppIfStatus            ret = kAppIfSuccessful;
    tEplKernel              eplret = kEplSuccessful;
    static tEplApiInitParam initParam;

    PRINTF("Initializing openPOWERLINK stack...\n");

    EPL_MEMSET(&initParam, 0, sizeof(initParam));

    initParam.m_uiSizeOfStruct            = sizeof(initParam);
    initParam.m_uiNodeId                  = pInstance_p->nodeId;
    initParam.m_dwIpAddress               = (0xFFFFFF00 & IP_ADDR) | initParam.m_uiNodeId;

    EPL_MEMCPY(initParam.m_abMacAddress, pInstance_p->aMacAddr,
            sizeof(initParam.m_abMacAddress));

    initParam.m_fAsyncOnly                = FALSE;
    initParam.m_dwFeatureFlags            = -1;
    initParam.m_dwCycleLen                = pInstance_p->cycleTime;        // required for error detection
    initParam.m_uiIsochrTxMaxPayload      = CONFIG_ISOCHR_TX_MAX_PAYLOAD;  // const
    initParam.m_uiIsochrRxMaxPayload      = CONFIG_ISOCHR_RX_MAX_PAYLOAD;  // const
    initParam.m_dwPresMaxLatency          = 2000;                          // const; only required for IdentRes
    initParam.m_dwAsndMaxLatency          = 2000;                          // const; only required for IdentRes
    initParam.m_uiPreqActPayloadLimit     = 36;                            // required for initialization (+28 bytes)
    initParam.m_uiPresActPayloadLimit     = 40;                            // required for initialization of Pres frame (+28 bytes)
    initParam.m_uiMultiplCycleCnt         = 0;                             // required for error detection
    initParam.m_uiAsyncMtu                = 300;                           // required to set up max frame size
    initParam.m_uiPrescaler               = 2;                             // required for sync
    initParam.m_dwLossOfFrameTolerance    = 5000000;
    initParam.m_dwAsyncSlotTimeout        = 3000000;
    initParam.m_dwWaitSocPreq             = 0;
    initParam.m_uiSyncNodeId              = EPL_C_ADR_SYNC_ON_SOC;
    initParam.m_fSyncOnPrcNode            = FALSE;
    initParam.m_dwDeviceType              = CONFIG_IDENT_DEVICE_TYPE;      // NMT_DeviceType_U32
    initParam.m_dwVendorId                = CONFIG_IDENT_VENDOR_ID;        // NMT_IdentityObject_REC.VendorId_U32
    initParam.m_dwProductCode             = CONFIG_IDENT_PRODUCT_CODE;     // NMT_IdentityObject_REC.ProductCode_U32
    initParam.m_dwRevisionNumber          = CONFIG_IDENT_REVISION;         // NMT_IdentityObject_REC.RevisionNo_U32
    initParam.m_dwSerialNumber            = CONFIG_IDENT_SERIAL_NUMBER;    // NMT_IdentityObject_REC.SerialNo_U32
    initParam.m_dwApplicationSwDate       = 0;
    initParam.m_dwApplicationSwTime       = 0;
    initParam.m_dwSubnetMask              = pInstance_p->subnetMask;
    initParam.m_dwDefaultGateway          = pInstance_p->defGateway;

    strncpy((char *)initParam.m_sHostname, (char *)pInstance_p->sHostName,
            sizeof(initParam.m_sHostname));

    // set callback functions
    initParam.m_pfnCbEvent                = processEvents;
    initParam.m_pfnCbSync                 = appif_syncCb;

    // initialize POWERLINK stack
    eplret = oplk_init(&initParam);
    if(eplret != kEplSuccessful)
    {
        PRINTF("oplk_init() failed (Error:0x%x!\n", ret);
        ret = kAppIfMainPlkStackInitError;
    }

    return kEplSuccessful;
}


//------------------------------------------------------------------------------
/**
\brief    Process the POWERLINK stack

Carry out kNmtEventSwReset and periodically process the POWERLINK background
task.

\param[in] pInstance_p       Module instance

\return tAppIfStatus
\retval kAppIfSuccessful                  Successful exit
\retval kAppIfMainPlkStackStartError      Unable to start the stack
\retval kAppIfMainPlkStackProcessError    Unable to process the stack
\retval other                             On other error

\ingroup module_main
*/
//------------------------------------------------------------------------------
static tAppIfStatus appif_processPlk(tMainInstance* pInstance_p)
{
    tEplKernel eplret = kEplSuccessful;
    tAppIfStatus ret = kAppIfSuccessful;

    // start processing
    eplret = oplk_execNmtCommand(kNmtEventSwReset);
    if(eplret != kEplSuccessful)
    {
        ret = kAppIfMainPlkStackStartError;
        goto Exit;
    }

    while(1)
    {
        // do background tasks
        eplret = oplk_process();
        if(eplret != kEplSuccessful)
        {
            ret = kAppIfMainPlkStackProcessError;
            break;
        }

        // Handle background task of application interface
        ret = appif_handleAsync();
        if(ret != kAppIfSuccessful)
            break;

        // trigger switch off
        if(pInstance_p->fShutdown != FALSE)
        {
            appif_switchOffPlk();

            // reset shutdown flag to generate only one switch off command
            pInstance_p->fShutdown = FALSE;
        }

        // exit loop if NMT is in off state
        if(pInstance_p->plkState == kNmtGsOff)
            break;
    }

Exit:
    return ret;
}

/**
 ********************************************************************************
\brief
 *******************************************************************************/

//------------------------------------------------------------------------------
/**
\brief    Switch off the openPOWERLINK stack

\ingroup module_main
*/
//------------------------------------------------------------------------------
static void appif_switchOffPlk(void)
{
    tEplKernel ret;

    ret = oplk_execNmtCommand(kNmtEventSwitchOff);
    if(ret != kEplSuccessful)
    {
        DEBUG_TRACE(DEBUG_LVL_ERROR, "ERROR: Unable to carry out kNmtEventSwitchOff!\n");
    }
}

//------------------------------------------------------------------------------
/**
\brief    Enter the critical section

\param[in] fEnable_p       TRUE = Interrupts disabled
                           FALSE = Interrupts enabled

\ingroup module_main
*/
//------------------------------------------------------------------------------
static void appif_enterCriticalSection(UINT8 fEnable_p)
{
    EplTgtEnableGlobalInterrupt(fEnable_p);
}


//------------------------------------------------------------------------------
/**
\brief  openPOWERLINK event callback

The function implements the applications stack event handler.

\param  EventType_p         Type of event
\param  pEventArg_p         Pointer to union which describes the event in detail
\param  pUserArg_p          User specific argument

\return The function returns a tEplKernel error code.

\ingroup module_main
*/
//------------------------------------------------------------------------------
static tEplKernel appif_userEventCb(tEplApiEventType EventType_p,
                                   tEplApiEventArg* pEventArg_p,
                                   void* pUserArg_p)
{
    tEplKernel  eplret = kEplSuccessful;
    tAppIfStatus ret = kAppIfSuccessful;

    UNUSED_PARAMETER(pUserArg_p);

    switch(EventType_p)
    {
        case kEplApiEventNmtStateChange:
        {
            // Make POWERLINK stack state global
            mainInstance_l.plkState = pEventArg_p->m_NmtStateChange.newNmtState;

            switch(pEventArg_p->m_NmtStateChange.newNmtState)
            {
                case kNmtGsOff:
                {
                    // NMT state machine was shut down
                    eplret = kEplShutdown;

                    break;
                }
                case kNmtGsResetConfiguration:
                {
                    UINT size = sizeof(mainInstance_l.cycleTime);;

                    eplret = oplk_readLocalObject(0x1006, 0, &mainInstance_l.cycleTime, &size);
                    if (eplret != kEplSuccessful)
                    {
                        mainInstance_l.cycleTime = 0;
                    }
                    else
                    {
                        // Forward current cycle to status module
                        status_setCycleTime(mainInstance_l.cycleTime);
                    }

                    break;
                }
                case kNmtCsPreOperational1:
                {
                    // Reset the relative time state machine
                    ret = status_resetRelTime();
                    if(ret != kAppIfSuccessful)
                    {
                        eplret = kEplNoResource;
                    }
                    break;
                }
                case kNmtGsResetCommunication:
                {
                    UINT8   nodeId = 0xF0;
                    UINT32  nodeAssignment = EPL_NODEASSIGN_NODE_EXISTS;

                    eplret = oplk_writeLocalObject(0x1F81, nodeId, &nodeAssignment, sizeof(nodeAssignment));

                    break;
                }
                case kNmtCsOperational:
                {
                    status_enableSyncInt();

                    break;
                }
                default:
                    break;
            }
            break;
        }
        case kEplApiEventSdo:
        {
            // Sdo access finished -> tell appif interface
            ret = appif_sdoAccFinished(&pEventArg_p->m_Sdo);
            if(ret != kAppIfSuccessful)
            {
                eplret = kEplApiInvalidParam;
            }
            break;
        }
        default:
            break;
    }

    return eplret;
}

//------------------------------------------------------------------------------
/**
\brief    Sync event callback function

appif_syncCb() implements the event callback function called by event module
within kernel part (high priority). This function sets the outputs, reads the
inputs and runs the control loop.

\param  socTimeStamp_p          Timestamp of the current POWERLINK cycle

\return    tEplKernel
\retval    kEplSuccessful            no error
\retval    otherwise                 post error event to API layer

\ingroup module_main
*/
//------------------------------------------------------------------------------
static tEplKernel PUBLIC appif_syncCb(tSocTimeStamp* socTimeStamp_p)
{
    tEplKernel         eplret = kEplSuccessful;
    tAppIfStatus       ret = kAppIfSuccessful;
    tTimeInfo          time;

    eplret = pdou_copyRxPdoToPi();
    if(eplret != kEplSuccessful)
        goto Exit;

    appif_pdoProcFinished(tPdoDirRpdo);

    // CN is configured (cycle time is set)
    if (mainInstance_l.cycleTime != 0 &&
            mainInstance_l.plkState >= kNmtCsReadyToOperate)
    {
        time.relativeTimeLow_m = (UINT32)socTimeStamp_p->relTime;
        time.relativeTimeHigh_m = (UINT32)(socTimeStamp_p->relTime>>32);
        time.fTimeValid_m = socTimeStamp_p->fSocRelTimeValid;
        time.fCnIsOperational_m = (mainInstance_l.plkState == kNmtCsOperational) ? TRUE : FALSE;

        ret = status_process(&time);
        if(ret != kAppIfSuccessful)
        {
            eplret = kEplInvalidOperation;
            goto Exit;
        }

        // Handle synchronous task of application interface
        ret = appif_handleSync();
        if(ret != kAppIfSuccessful)
        {
            eplret = kEplInvalidOperation;
            goto Exit;
        }
    }

    eplret = pdou_copyTxPdoFromPi();
    if(eplret != kEplSuccessful)
        goto Exit;

    appif_pdoProcFinished(tPdoDirTpdo);

Exit:
    return eplret;
}

/// \}

