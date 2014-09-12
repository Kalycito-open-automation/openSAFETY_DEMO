/**
********************************************************************************
\file   cc.c

\brief  Configuration channel common module

Common module of the configuration channel module. Initializes the channels
object list.

\ingroup module-cc
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

#include <appif/cc.h>
#include <libappifcommon/ccobject.h>

#include <config/ccobjectlist.h>

#include <oplk/oplk.h>

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
\brief    Initialize the list of configuration channel objects

\return tAppIfStatus
\retval kAppIfSuccessful                On success
\retval kAppIfMainInitCcObjectFailed    Init of CC object failed

\ingroup module_appif
*/
//------------------------------------------------------------------------------
tAppIfStatus appif_initCcObjects(void)
{
    tAppIfStatus     ret = kAppIfSuccessful;
    tOplkError       oplkret = kErrorOk;
    tConfChanObject  object;
    UINT32           objSize;
    UINT8            i;
    UINT64           paylDest = 0;
    tCcObject        initObjList[CONF_CHAN_NUM_OBJECTS] = CCOBJECT_LIST_INIT_VECTOR;

    APPIF_MEMSET(&object, 0, sizeof(tConfChanObject));

    for(i=0; i < CONF_CHAN_NUM_OBJECTS; i++)
    {
        // For every object in list
        objSize = initObjList[i].objSize;

        // Read object details from local obdict
        oplkret = oplk_readLocalObject(initObjList[i].objIdx, initObjList[i].objSubIdx,
                &paylDest, &objSize);
        if(oplkret != kErrorOk)
        {
            ret = kAppIfConfChanInitCcObjectFailed;
            break;
        }

        // Check size of object!
        if(objSize != initObjList[i].objSize)
        {
            ret = kAppIfConfChanInitCcObjectFailed;
            break;
        }

        // Assemble object for list
        object.objIdx_m = initObjList[i].objIdx;
        object.objSubIdx_m = initObjList[i].objSubIdx;
        object.objSize_m = initObjList[i].objSize;
        APPIF_MEMCPY(&object.objPayloadLow_m, &paylDest, objSize);

        // Now initialize the object in the local list
        if(ccobject_initObject(i, &object) == FALSE)
        {
            ret = kAppIfConfChanInitCcObjectFailed;
            break;
        }
    }

    return ret;
}


//============================================================================//
//            P R I V A T E   F U N C T I O N S                               //
//============================================================================//
/// \name Private Functions
/// \{

/// \}


