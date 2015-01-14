/*| sodbuilder v0.1 | Header generated for demo-sn-gpio */

#ifndef SOD_H
#define SOD_H

#include <stdint.h>

#include <sn/global.h>
#include <sod_helper.h>

#include <powerlink.h>

#include <SPDOapi.h>

/**
 * @def SOD_k_NO_CALLBACK
 * @brief This define is used if no SOD callback function is available for the SOD object.
 */
#define SOD_k_NO_CALLBACK ((SOD_t_CLBK)(0))

/** SAPL_k_MAX_PARAM_SET_LEN:
    Maximum length of the parameter set.
*/
#define SAPL_k_MAX_PARAM_SET_LEN       361UL

#ifndef CFG_SAPL_SN_UDID

#warning "No UDID passed to the SN build! Using default value in CFG_SAPL_SN_UDID!"

/**
 * \brief Source address of the SN
 */
#define SAPL_k_SN_SADR     0x01U

/**
 * \brief The default UDID of the SN
 */
#define CFG_SAPL_SN_UDID     {MAC_VENDOR_ID,0x01U,0x14U,SAPL_k_SN_SADR}

#endif /* CFG_SAPL_SN_UDID*/


/*************************************************************************/

/** This define is the maximum number of CRCs any instance within the SL needs to protect its SOD. */
#if (! defined EPS_NO_SOD_CRCS)
    #define EPS_NO_SOD_CRCS 1
#endif

/**
 * \brief This is the structure of the CRC Object located in the SOD idx 1018 sidx 6
 */
typedef struct sSodCrcObj
{
    uint32_t ulTimestamp;                    /**< timestamp of the parameters */
    uint32_t aulSodCrc[EPS_NO_SOD_CRCS];     /**< crcs over the SOD */
} tParamChksum;


/*************************************************************************/


/**
 *  \brief Structure of SOD index 1001
 */
typedef struct sSOD_ErrRegister
{
    uint8_t     ucErrorRegister;         /**< error register */
} tSOD_ErrRegister;

/**
 * \brief Structure of SOD index 100C [Life Guarding]
 */
typedef struct sSOD_LifeGuard
{
    uint32_t    ulGuardTime;            /**< guard time interval */
    uint8_t     ucLifeTimeFactor;
} tSOD_LifeGuard;

/**
 *  \brief Structure of SOD index 100D [Number of Retries for Reset Guarding]
 */
typedef struct sSOD_NumRetriesRG
{
    uint32_t    ulRefreshPreOp;        /**< pre-operational signal interval */
} tSOD_NumRetriesRG;

/**
 *  \brief Structure of SOD index 100E [Refresh interval of Reset Guarding]
 */
typedef struct sSOD_RefreshIntRG
{
    uint8_t     ucNoRetries;          /**< number of retries */
} tSOD_RefreshIntRG;

/**
 * This structure contains the SOD variables which are hosted by EPS.
 * Within the SOD of the module those variables are used as actual values
 * and as default values.
 */
typedef struct sSOD_DevVendInfo
{
    uint8_t  aucUdid[EPLS_k_UDID_LEN];              /**< unique device id       0x1019 */
    uint32_t ulVendorId;                            /**< vendor id              0x1018 0x01 */
    uint32_t ulProductCode;                         /**< product code           0x1018 0x02 */
    uint32_t ulRevisionNumber;                      /**< revision number        0x1018 0x03 */
    uint32_t ulSerialNumber;                        /**< serial number          0x1018 0x04 */
    uint32_t ulFirmwareCrc;                         /**< firmware checksum      0x1018 0x05 */
    tParamChksum aParamCrcs;                        /**< Parameter CRC object   0x1018 0x06 */
    uint32_t aulParamTimestamp[EPS_NO_SOD_CRCS];    /**< Parameter timestamp    0x1018 0x07 */
} tSOD_DevVendInfo;

/**
 *  \brief Structure of the SOD index 1200 [Common Communication Parameter]
 */
typedef struct sSOD_CommonComParam
{
    uint16_t    usSdn;                          /**< safety domain of the SN */
    uint16_t    usSadrOfScm;                    /**< SADR of the SCM */
    int8_t      cCtb;                           /**< consecutive time base */
    uint8_t     aucUdidScm[EPLS_k_UDID_LEN];    /**< UDID of SCM */
} tSOD_CommonComParam;

/**
 *  \brief Structure of the SOD index 1400 [Rx SPDO Communication Parameter]
 */
typedef struct sSOD_RxComParam
{
    uint16_t    usSadr;                /**< SADR of the RxSPDO */
    uint32_t    ulSct;                 /**< SCT timer of the RxSPDO */
    uint8_t     ucNoConsecTReq;        /**< number of consecutive time requests */
    uint32_t    ulTimeDelayTReq;       /**< time delay between sets of TReq */
    uint32_t    ulTimeDelaySync;       /**< time delay after succ. sync */
    uint16_t    usMinTSyncPropDelay;   /**< minimum TSync propagation delay */
    uint16_t    usMaxTSyncPropDelay;   /**< maximum TSync propagation delay */
    uint16_t    usMinSpdoPropDelay;    /**< minimum SPDO propagation delay */
    uint16_t    usMaxSpdoPropDelay;    /**< maximum SPDO propagation delay */
    uint16_t    usBestCaseTresDelay;   /**< best case TRes delay */
    uint32_t    ulTReqCycle;           /**< time request cycle */
    uint16_t    usTxSpdoNo;            /**< number of the TxSPDO where the TReq is to be sent in */
} tSOD_RxComParam;

/**
 *  \brief Structure of the SOD index 1800 [Rx SPDO Mapping Parameter]
 */
typedef struct sSOD_RxMapParam
{
    uint8_t     ucNoMappingEntries;    /**< number of active mapping entries */
    uint32_t    aulMappingEntry[SPDO_cfg_MAX_RX_SPDO_MAPP_ENTRIES];    /**< mapping entries */
} tSOD_RxMapParam;

/**
 *  \brief Structure of the SOD index 1C00 [Tx SPDO Communication Parameter]
 */
typedef struct sSOD_TxComParam
{
    uint16_t    usSadr;               /**< SADR of the TxSPDO */
    uint16_t    usRefreshPrescale;    /**< refresh prescale of the TxSPDO */
    uint8_t     ucNoTRes;             /**< number of consecutive TRes */
} tSOD_TxComParam;

/**
 *  \brief Structure of the SOD index C000 [Tx SPDO Mapping Parameter]
 */
typedef struct sSOD_TxMapParam
{
    uint8_t     ucNoMappingEntries;                                    /**< number of active mapping entries */
    uint32_t    aulMappingEntry[SPDO_cfg_MAX_TX_SPDO_MAPP_ENTRIES];    /**< mapping entries */
} tSOD_TxMapParam;

/**
 *  \brief This structure contains
 */
typedef struct sSodObjectData
{
    tSOD_ErrRegister      errRegister;                              /**< Object 1001/00 error register */
    tSOD_LifeGuard        lifeGuard;                                /**< Object 100C life guard */
    tSOD_NumRetriesRG     numRetriesRG;                             /**< Object 100D Pre-Operational signal */
    tSOD_RefreshIntRG     refreshIntRG;                             /**< Object 100E Refresh interrupt */
    tSOD_DevVendInfo      devVendInfo;                              /**< Object 100E Refresh interrupt */
    tSOD_CommonComParam   commonComParam;
    tSOD_RxComParam       aRxComParam[SPDO_cfg_MAX_NO_RX_SPDO];
    tSOD_RxMapParam       aRxMapParam[SPDO_cfg_MAX_NO_RX_SPDO];
    tSOD_TxComParam       aTxComParam[SPDO_cfg_MAX_NO_TX_SPDO];
    tSOD_TxMapParam       aTxMapParam[SPDO_cfg_MAX_NO_TX_SPDO];
} tSodObjectData;

#endif /* SOD_H */
