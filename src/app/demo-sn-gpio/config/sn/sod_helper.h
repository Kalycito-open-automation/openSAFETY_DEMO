/*| sodbuilder v0.1 | Header generated for demo-sn-gpio */

#ifndef SOD_HELPER_H
#define SOD_HELPER_H

#include <EPLScfg.h>
#include <EPLStarget.h>
#include <EPLStypes.h>

#include <SODapi.h>
#include <SPDOapi.h>

#define USEDCHANNELSIZE 8

typedef struct sUsedChannels
{
    UINT16 entries;
    BOOLEAN channel[ USEDCHANNELSIZE ];
}tUsedChannels;
/**
*  Structure for the SettingGroup "DefaultSettingsGroup"
*/
typedef struct sDefaultSettingsGroup
{
    UINT32 DefaultSetting01;
    UINT32 DefaultSetting02;
    UINT32 DefaultSetting03;
}tDefaultSettingsGroup;

/**
 *  Structure of all SettingGroups
 */
typedef struct sSettingGroups
{
    tDefaultSettingsGroup DefaultSettingsGroup;
}tSettingGroups;

tSettingGroups parseMfrParam(const UINT8 * stream);

extern BOOLEAN SAPL_SOD_ParameterSet_CLBK(BYTE_B_INSTNUM_ SOD_t_SERVICE e_srvc,
                                          const SOD_t_OBJECT *ps_obj,
                                          const void *pv_data,
                                          UINT32 org_dw_offset, UINT32 org_dw_size,
                                          SOD_t_ABORT_CODES *pe_abortCode);

extern BOOLEAN SHNF_SOD_ConsTimeBase_CLBK(BYTE_B_INSTNUM_ SOD_t_SERVICE e_srvc,
                                          const SOD_t_OBJECT *ps_obj,
                                          const void *pv_data,
                                          UINT32 org_dw_offset, UINT32 org_dw_size,
                                          SOD_t_ABORT_CODES *pe_abortCode);

#endif /*PARAM_SOD_H*/
