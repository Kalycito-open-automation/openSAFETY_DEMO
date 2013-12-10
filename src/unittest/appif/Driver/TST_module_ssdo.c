#include <cunit/CUnit.h>

#include <libappif/appif.h>

#define SUCCESS		0
#define FAIL		1

//------------------------------------------------------------------------------
// local types
//------------------------------------------------------------------------------

typedef struct {
    tTbufMemLayout     tbufMemLayout_m;                        ///< Local copy of the triple buffer memory
    tSsdoInstance      instSsdoChan_m;    ///< Instance of the SSDO channel
} tMainInstance;

//------------------------------------------------------------------------------
// local vars
//------------------------------------------------------------------------------

static tMainInstance mainInstance_l;            ///< Instance of main module

//------------------------------------------------------------------------------
// local function prototypes
//------------------------------------------------------------------------------
static BOOL appif_initLibrary(void);
static BOOL appif_initSsdoModule(void);
static BOOL appif_ssdoCbRcvPaylChan0(UINT8* pPayload_p, UINT16 size_p);
static void appif_genDescList(tBuffDescriptor* pBuffDescList_p, UINT8 buffCount_p);
BOOL SysComp_SPICommand(tHandlerParam* pHandlParam_p);

//============================================================================//
//            T E S T   I N I T   F U N C T I O N                             //
//============================================================================//

// Initialize the test suite
int TST_ssdo_init(void)
{
	BOOL fReturn = FAIL;

	APPIF_MEMSET(&mainInstance_l, 0, sizeof(mainInstance_l));

	if(appif_initLibrary() != FALSE)
	{
		if(appif_initSsdoModule() != FALSE)
		{
			fReturn = SUCCESS;
		}
	}

	return fReturn;
}

//============================================================================//
//            T E S T   F U N C T I O N S                                     //
//============================================================================//

void TST_ssdoPostPayloadBusy(void)
{
	tSsdoTxStatus txState = kSsdoTxStatusError;
    UINT8 ssdoPayload[20] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 };

	CU_ASSERT_EQUAL ( ssdo_postPayload(mainInstance_l.instSsdoChan_m, &ssdoPayload[0], sizeof(ssdoPayload) ), kSsdoTxStatusSuccessful );
	/* Allocate API */
	CU_ASSERT_EQUAL ( ssdo_postPayload(mainInstance_l.instSsdoChan_m, &ssdoPayload[0], sizeof(ssdoPayload) ), kSsdoTxStatusBusy );
}

void TST_ssdoAnOtherTest(void)
{
	CU_ASSERT_EQUAL ( 0, 0 );
}

//============================================================================//
//            T E S T   C L O S E   F U N C T I O N                           //
//============================================================================//
int TST_ssdo_clean(void)
{
	return 0;
}

//============================================================================//
//            S T A T I C   F U N C T I O N S                                 //
//============================================================================//

static BOOL appif_initLibrary(void)
{
	BOOL                fReturn = FALSE;
	tAppIfInitParam     initParam;
    tBuffDescriptor     buffDescList[kTbufCount];

    APPIF_MEMSET(&buffDescList, 0, sizeof(buffDescList));

    // Generate buffer descriptor list
    appif_genDescList(&buffDescList[0], kTbufCount);

    initParam.pBuffDescList_m = &buffDescList[0];
    initParam.pfnStreamHandler_m = SysComp_SPICommand;
    initParam.pfnErrorHandler_m = NULL;
    initParam.idConsAck_m = kTbufAckRegisterCons;
    initParam.idProdAck_m = kTbufAckRegisterProd;
    initParam.idFirstProdBuffer_m = TBUF_NUM_CON + 1;   // Add one buffer for the consumer ACK register

    if(appif_init(&initParam) != FALSE)
    {
		fReturn = TRUE;
    }

	return fReturn;
}

static BOOL appif_initSsdoModule(void)
{
	BOOL fReturn = FALSE;
	tSsdoInitParam    ssdoInitParam;

	ssdoInitParam.buffIdRx_m = kTbufNumSsdoReceive0;
    ssdoInitParam.buffIdTx_m = kTbufNumSsdoTransmit0;
    ssdoInitParam.pfnRxHandler_m = appif_ssdoCbRcvPaylChan0;

    mainInstance_l.instSsdoChan_m = ssdo_create(0, &ssdoInitParam);
    if(mainInstance_l.instSsdoChan_m != NULL)
    {
        fReturn = TRUE;
    }

	return fReturn;
}

static BOOL appif_ssdoCbRcvPaylChan0(UINT8* pPayload_p, UINT16 size_p)
{
	return TRUE;
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

BOOL SysComp_SPICommand(tHandlerParam* pHandlParam_p)
{
	return TRUE;
}