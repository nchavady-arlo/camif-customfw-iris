/******************************************************************************
  * @attention
  *
  * COPYRIGHT 2019 STMicroelectronics, all rights reserved
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied,
  * AND SPECIFICALLY DISCLAIMING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
******************************************************************************/


/*! \file
 *
 *  \author 
 *
 *  \brief Demo application
 *
 *  This demo shows how to poll for several types of NFC cards/devices and how 
 *  to exchange data with these devices, using the RFAL library.
 *
 *  This demo does not fully implement the activities according to the standards,
 *  it performs the required to communicate with a card/device and retrieve 
 *  its UID. Also blocking methods are used for data exchange which may lead to
 *  long periods of blocking CPU/MCU.
 * 
 */
 
/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */
#include "demo.h"
#include "utils.h"
#include "logger.h"
#include "rfal_nfc.h"
#include "rfal_t2t.h"

#if RFAL_SUPPORT_CE && RFAL_FEATURE_LISTEN_MODE
#include "demo_ce.h"
#endif /* RFAL_FEATURE_LISTEN_MODE */

/*
******************************************************************************
* GLOBAL DEFINES
******************************************************************************
*/

/* Definition of possible states the demo state machine could have */
#define DEMO_ST_NOTINIT               0     /*!< Demo State:  Not initialized | Stopped */
#define DEMO_ST_START_DISCOVERY       1     /*!< Demo State:  Start Discovery           */
#define DEMO_ST_DISCOVERY             2     /*!< Demo State:  Discovery                 */
                                                                                        
#define DEMO_NFCV_BLOCK_LEN           4     /*!< NFCV Block len                         */
                                                                                        
#define DEMO_NFCV_USE_SELECT_MODE     false /*!< NFCV demonstrate select mode           */
#define DEMO_NFCV_WRITE_TAG           false /*!< NFCV demonstrate Write Single Block    */
    
/* Definition of various Listen Mode constants */
#if defined(DEMO_LISTEN_MODE_TARGET) 
#define DEMO_LM_SEL_RES       0x40U         /*!<NFC-A SEL_RES configured for the NFC-DEP protocol    */
#define DEMO_LM_NFCID2_BYTE1  0x01U         /*!<NFC-F SENSF_RES configured for the NFC-DEP protocol  */ 
#define DEMO_LM_SC_BYTE1      0xFFU         /*!<NFC-F System Code byte 1                             */ 
#define DEMO_LM_SC_BYTE2      0xFFU         /*!<NFC-F System Code byte 2                             */ 
#define DEMO_LM_PAD0          0xFFU         /*!<NFC-F PAD0                                           */ 
#else
#define DEMO_LM_SEL_RES       0x20U         /*!<NFC-A SEL_RES configured for Type 4A Tag Platform    */
#define DEMO_LM_NFCID2_BYTE1  0x02U         /*!<NFC-F SENSF_RES configured for Type 3 Tag Platform   */ 
#define DEMO_LM_SC_BYTE1      0x12U         /*!<NFC-F System Code byte 1                             */ 
#define DEMO_LM_SC_BYTE2      0xFCU         /*!<NFC-F System Code byte 2                             */ 
#define DEMO_LM_PAD0          0x00U         /*!<NFC-F PAD0                                           */ 
#endif

/*
 ******************************************************************************
 * GLOBAL MACROS
 ******************************************************************************
 */

/*
 ******************************************************************************
 * LOCAL VARIABLES
 ******************************************************************************
 */

/* P2P communication data */
static uint8_t NFCID3[] = {0x01, 0xFE, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
static uint8_t GB[] = {0x46, 0x66, 0x6d, 0x01, 0x01, 0x11, 0x02, 0x02, 0x07, 0x80, 0x03, 0x02, 0x00, 0x03, 0x04, 0x01, 0x32, 0x07, 0x01, 0x03};

/* APDUs communication data */
#if RFAL_FEATURE_ISO_DEP_POLL
static uint8_t ndefSelectApp[] = { 0x00, 0xA4, 0x04, 0x00, 0x07, 0xD2, 0x76, 0x00, 0x00, 0x85, 0x01, 0x01, 0x00 };
static uint8_t ccSelectFile[] = { 0x00, 0xA4, 0x00, 0x0C, 0x02, 0xE1, 0x03};
static uint8_t readBinary[] = { 0x00, 0xB0, 0x00, 0x00, 0x0F };

/* For a Payment application a Select PPSE would be needed: 
   ppseSelectApp[] = { 0x00, 0xA4, 0x04, 0x00, 0x0E, 0x32, 0x50, 0x41, 0x59, 0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31, 0x00 } */
#endif /* RFAL_FEATURE_ISO_DEP_POLL */

#if RFAL_FEATURE_NFC_DEP
/* P2P communication data */
static uint8_t ndefLLCPSYMM[] = {0x00, 0x00};
static uint8_t ndefInit[] = {0x05, 0x20, 0x06, 0x0F, 0x75, 0x72, 0x6E, 0x3A, 0x6E, 0x66, 0x63, 0x3A, 0x73, 0x6E, 0x3A, 0x73, 0x6E, 0x65, 0x70, 0x02, 0x02, 0x07, 0x80, 0x05, 0x01, 0x02};
static uint8_t ndefUriSTcom[] = {0x13, 0x20, 0x00, 0x10, 0x02, 0x00, 0x00, 0x00, 0x19, 0xc1, 0x01, 0x00, 0x00, 0x00, 0x12, 0x55, 0x00, 0x68, 0x74, 0x74, 0x70, 0x3a, 0x2f, 0x2f, 0x77, 0x77, 0x77, 0x2e, 0x73, 0x74, 0x2e, 0x63, 0x6f, 0x6d};
#endif /* RFAL_FEATURE_NFC_DEP */

#if RFAL_SUPPORT_CE && RFAL_FEATURE_LISTEN_MODE
#if RFAL_SUPPORT_MODE_LISTEN_NFCA
/* NFC-A CE config */
/* 4-byte UIDs with first byte 0x08 would need random number for the subsequent 3 bytes.
 * 4-byte UIDs with first byte 0x*F are Fixed number, not unique, use for this demo
 * 7-byte UIDs need a manufacturer ID and need to assure uniqueness of the rest.*/
static uint8_t ceNFCA_NFCID[]     = {0x5F, 'S', 'T', 'M'};    /* =_STM, 5F 53 54 4D NFCID1 / UID (4 bytes) */
static uint8_t ceNFCA_SENS_RES[]  = {0x02, 0x00};             /* SENS_RES / ATQA for 4-byte UID            */
static uint8_t ceNFCA_SEL_RES     = DEMO_LM_SEL_RES;          /* SEL_RES / SAK                             */
#endif /* RFAL_SUPPORT_MODE_LISTEN_NFCA */

static uint8_t ceNFCF_nfcid2[]     = {DEMO_LM_NFCID2_BYTE1, 0xFE, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66};

#if RFAL_SUPPORT_MODE_LISTEN_NFCF
  /* NFC-F CE config */
static uint8_t ceNFCF_SC[]         = {DEMO_LM_SC_BYTE1, DEMO_LM_SC_BYTE2};
static uint8_t ceNFCF_SENSF_RES[]  = {0x01,                                                       /* SENSF_RES                                */
                                  0x02, 0xFE, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66,                 /* NFCID2                                   */
                                  DEMO_LM_PAD0, DEMO_LM_PAD0, 0x00, 0x00, 0x00, 0x7F, 0x7F, 0x00, /* PAD0, PAD1, MRTIcheck, MRTIupdate, PAD2  */
                                  0x00, 0x00 };                                                   /* RD                                       */
#endif /* RFAL_SUPPORT_MODE_LISTEN_NFCF */
#endif /* RFAL_SUPPORT_CE && RFAL_FEATURE_LISTEN_MODE */
                                 

/*
 ******************************************************************************
 * LOCAL VARIABLES
 ******************************************************************************
 */

static rfalNfcDiscoverParam discParam;
static uint8_t              state = DEMO_ST_NOTINIT;
static bool                 multiSel;

/*
******************************************************************************
* LOCAL FUNCTION PROTOTYPES
******************************************************************************
*/

static void demoP2P( rfalNfcDevice *nfcDev );
static void demoAPDU( void );
static void demoNfcv( rfalNfcvListenDevice *nfcvDev );
static void demoNfcf( rfalNfcfListenDevice *nfcfDev );
static void demoT2t( void );
static void demoCE( rfalNfcDevice *nfcDev );
static void demoNotif( rfalNfcState st );
ReturnCode  demoTransceiveBlocking( uint8_t *txBuf, uint16_t txBufSize, uint8_t **rxBuf, uint16_t **rcvLen, uint32_t fwt );


/*!
 *****************************************************************************
 * \brief Returns the RFAL NFC Discovery Parameter: Tech2Find
 *****************************************************************************
 */
uint16_t demoGetDiscoverTechs2Find( void )
{
    return discParam.techs2Find;
}

/*!
 *****************************************************************************
 * \brief Returns the RFAL NFC Discovery Parameter: Total Duration
 *****************************************************************************
 */
uint16_t demoGetDiscoverTotalDuration( void )
{
    return discParam.totalDuration;
}

/*!
 *****************************************************************************
 * \brief Demo Notification
 *
 *  This function receives the event notifications from RFAL
 *****************************************************************************
 */
static void demoNotif( rfalNfcState st )
{
    uint8_t       devCnt;
    rfalNfcDevice *dev;
    
    if( st == RFAL_NFC_STATE_WAKEUP_MODE )
    {
        platformLog("Wake Up mode started \r\n");
    }
    else if( st == RFAL_NFC_STATE_POLL_TECHDETECT )
    {
        if( discParam.wakeupEnabled )
        {
            platformLog("Wake Up mode terminated. Polling for devices \r\n");
        }
    }
    else if( st == RFAL_NFC_STATE_POLL_SELECT )
    {
        /* Check if in case of multiple devices, selection is already attempted */
        if( (!multiSel) )
        {
            multiSel = true;
            /* Multiple devices were found, activate first of them */
            rfalNfcGetDevicesFound( &dev, &devCnt );
            rfalNfcSelect( 0 );
            
            platformLog("Multiple Tags detected: %d \r\n", devCnt);
        }
        else
        {
            rfalNfcDeactivate( RFAL_NFC_DEACTIVATE_DISCOVERY );
        }
    }
    else if( st == RFAL_NFC_STATE_START_DISCOVERY )
    {
        /* Clear multiple device selection flag */
        multiSel = false;
    }
}

/*!
 *****************************************************************************
 * \brief Demo Ini
 *
 *  This function initializes the required layers for the demo
 *
 * \return true  : Initialization ok
 * \return false : Initialization failed
 *****************************************************************************
 */
bool demoIni( void )
{
    ReturnCode err;

    err = rfalNfcInitialize();
	
    if( err == ERR_NONE )
    {
        rfalNfcDefaultDiscParams( &discParam );

        discParam.devLimit      = 1U;
        
        ST_MEMCPY( &discParam.nfcid3, NFCID3, sizeof(NFCID3) );
        ST_MEMCPY( &discParam.GB, GB, sizeof(GB) );
        discParam.GBLen         = sizeof(GB);
        discParam.p2pNfcaPrio   = true;

        discParam.notifyCb             = demoNotif;
        discParam.totalDuration        = 1000U;
        discParam.techs2Find           = RFAL_NFC_TECH_NONE;          /* For the demo, enable the NFC Technologies based on RFAL Feature switches */


#if RFAL_FEATURE_NFCA
        discParam.techs2Find          |= RFAL_NFC_POLL_TECH_A;
#endif /* RFAL_FEATURE_NFCA */

#if RFAL_FEATURE_NFCB
        discParam.techs2Find          |= RFAL_NFC_POLL_TECH_B;
#endif /* RFAL_FEATURE_NFCB */

#if RFAL_FEATURE_NFCF
        discParam.techs2Find          |= RFAL_NFC_POLL_TECH_F;
#endif /* RFAL_FEATURE_NFCF */

#if RFAL_FEATURE_NFCV
        discParam.techs2Find          |= RFAL_NFC_POLL_TECH_V;
#endif /* RFAL_FEATURE_NFCV */

#if RFAL_FEATURE_ST25TB
        discParam.techs2Find          |= RFAL_NFC_POLL_TECH_ST25TB;
#endif /* RFAL_FEATURE_ST25TB */

#ifdef ST25R95
        discParam.isoDepFS           = RFAL_ISODEP_FSXI_128;          /* ST25R95 cannot support 256 bytes of data block */
#endif /* ST25R95 */

#if RFAL_SUPPORT_MODE_POLL_ACTIVE_P2P && RFAL_FEATURE_NFC_DEP
        discParam.techs2Find |= RFAL_NFC_POLL_TECH_AP2P;
#endif /* RFAL_SUPPORT_MODE_POLL_ACTIVE_P2P && RFAL_FEATURE_NFC_DEP */

#if RFAL_SUPPORT_MODE_LISTEN_ACTIVE_P2P && RFAL_FEATURE_NFC_DEP && RFAL_FEATURE_LISTEN_MODE
        discParam.techs2Find |= RFAL_NFC_LISTEN_TECH_AP2P;
#endif /* RFAL_SUPPORT_MODE_LISTEN_ACTIVE_P2P && RFAL_FEATURE_NFC_DEP && RFAL_FEATURE_LISTEN_MODE */


#if DEMO_CARD_EMULATION_ONLY
        discParam.totalDuration        = 60000U;              /* 60 seconds */
        discParam.techs2Find           = RFAL_NFC_TECH_NONE;  /* Overwrite any previous poller modes */
#endif /* DEMO_CARD_EMULATION_ONLY */

#if RFAL_SUPPORT_CE && RFAL_FEATURE_LISTEN_MODE
        demoCeInit( ceNFCF_nfcid2 );
    
#if RFAL_SUPPORT_MODE_LISTEN_NFCA
        /* Set configuration for NFC-A CE */
        ST_MEMCPY( discParam.lmConfigPA.SENS_RES, ceNFCA_SENS_RES, RFAL_LM_SENS_RES_LEN );     /* Set SENS_RES / ATQA */
        ST_MEMCPY( discParam.lmConfigPA.nfcid, ceNFCA_NFCID, RFAL_LM_NFCID_LEN_04 );           /* Set NFCID / UID */
        discParam.lmConfigPA.nfcidLen = RFAL_LM_NFCID_LEN_04;                                  /* Set NFCID length to 7 bytes */
        discParam.lmConfigPA.SEL_RES  = ceNFCA_SEL_RES;                                        /* Set SEL_RES / SAK */

        discParam.techs2Find |= RFAL_NFC_LISTEN_TECH_A;
#endif /* RFAL_SUPPORT_MODE_LISTEN_NFCA */

#if RFAL_SUPPORT_MODE_LISTEN_NFCF
        /* Set configuration for NFC-F CE */
        ST_MEMCPY( discParam.lmConfigPF.SC, ceNFCF_SC, RFAL_LM_SENSF_SC_LEN );                 /* Set System Code */
        ST_MEMCPY( &ceNFCF_SENSF_RES[RFAL_NFCF_CMD_LEN], ceNFCF_nfcid2, RFAL_NFCID2_LEN );     /* Load NFCID2 on SENSF_RES */
        ST_MEMCPY( discParam.lmConfigPF.SENSF_RES, ceNFCF_SENSF_RES, RFAL_LM_SENSF_RES_LEN );  /* Set SENSF_RES / Poll Response */

        discParam.techs2Find |= RFAL_NFC_LISTEN_TECH_F;
#endif /* RFAL_SUPPORT_MODE_LISTEN_NFCF */
#endif /* RFAL_SUPPORT_CE && RFAL_FEATURE_LISTEN_MODE */

        err = rfalNfcDiscover( &discParam );
        if( err != ERR_NONE )
        {
            return false;
        }
        state = DEMO_ST_START_DISCOVERY;
        return true;
    }
	printf("fail rfalNfcInitialize\n");
    return false;
}

/*!
 *****************************************************************************
 * \brief Demo Cycle
 *
 *  This function executes the demo state machine. 
 *  It must be called periodically
 *****************************************************************************
 */
void demoCycle( void )
{
    static rfalNfcDevice *nfcDevice;
    
    rfalNfcWorker();                                    /* Run RFAL worker periodically */

#if defined(PLATFORM_USER_BUTTON_PORT) && defined(PLATFORM_USER_BUTTON_PIN)
	
/* Check if button is supported (even if the pins are defined). MCUs may have the pin but not connected | shorted to GND */
#ifdef platformIsUserButtonSupported
    if( platformIsUserButtonSupported() )
#endif  /*platformIsUserButtonSupported()*/
		
        /*******************************************************************************/
        /* Check if USER button is pressed */
        if( platformGpioIsLow(PLATFORM_USER_BUTTON_PORT, PLATFORM_USER_BUTTON_PIN))
        {
            discParam.wakeupEnabled = !discParam.wakeupEnabled;    /* enable/disable wakeup */
            state = DEMO_ST_START_DISCOVERY;                       /* restart loop          */
            platformLog("Toggling Wake Up mode %s\r\n", discParam.wakeupEnabled ? "ON": "OFF");

            /* Re-start discovery with new Wake-up configuration */
            rfalNfcDeactivate( RFAL_NFC_DEACTIVATE_IDLE );
            rfalNfcDiscover( &discParam );

            /* Debounce button */
            while( platformGpioIsLow(PLATFORM_USER_BUTTON_PORT, PLATFORM_USER_BUTTON_PIN) );
        }
#endif /* PLATFORM_USER_BUTTON_PIN */
    
    switch( state )
    {
        /*******************************************************************************/
        case DEMO_ST_START_DISCOVERY:
		
          multiSel = false;
          state    = DEMO_ST_DISCOVERY;
          break;

        /*******************************************************************************/
        case DEMO_ST_DISCOVERY:

            if( rfalNfcIsDevActivated( rfalNfcGetState() ) )
            {
                rfalNfcGetActiveDevice( &nfcDevice );
                
                switch( nfcDevice->type )
                {
                    /*******************************************************************************/
                    case RFAL_NFC_LISTEN_TYPE_NFCA:
                        //platformLedOn(PLATFORM_LED_A_PORT, PLATFORM_LED_A_PIN);
                        switch( nfcDevice->dev.nfca.type )
                        {
                            case RFAL_NFCA_T1T:
                                platformLog("ISO14443A/Topaz (NFC-A T1T) TAG found. UID: %s\r\n", hex2Str( nfcDevice->nfcid, nfcDevice->nfcidLen ) );
                                break;
                            
                            case RFAL_NFCA_T4T:
                                platformLog("NFCA Passive ISO-DEP device found. UID: %s\r\n", hex2Str( nfcDevice->nfcid, nfcDevice->nfcidLen ) );
                            
                                demoAPDU();
                                break;
                            
                            case RFAL_NFCA_T4T_NFCDEP:
                            case RFAL_NFCA_NFCDEP:
                                platformLog("NFCA Passive P2P device found. NFCID: %s\r\n", hex2Str( nfcDevice->nfcid, nfcDevice->nfcidLen ) );
                                
                                demoP2P( nfcDevice );
                                break;
                                
                            default:
                                platformLog("ISO14443A/NFC-A card found. UID: %s\r\n", hex2Str( nfcDevice->nfcid, nfcDevice->nfcidLen ) );
                                
                                demoT2t();
                                break;
                        }
                        break;
                    
                    /*******************************************************************************/
                    case RFAL_NFC_LISTEN_TYPE_NFCB:
                        platformLog("Type : B\n");

                        //platformLedOn(PLATFORM_LED_B_PORT, PLATFORM_LED_B_PIN);
                        platformLog("ISO14443B/NFC-B card found. UID: %s\r\n", hex2Str( nfcDevice->nfcid, nfcDevice->nfcidLen ) );
                    
                        if( rfalNfcbIsIsoDepSupported( &nfcDevice->dev.nfcb ) )
                        {
                            demoAPDU();
                        }
                        break;
                        
                    /*******************************************************************************/
                    case RFAL_NFC_LISTEN_TYPE_NFCF:
                        platformLog("Type : F\n");
                        //platformLedOn(PLATFORM_LED_F_PORT, PLATFORM_LED_F_PIN);
                        if( rfalNfcfIsNfcDepSupported( &nfcDevice->dev.nfcf ) )
                        {
                            platformLog("NFCF Passive P2P device found. NFCID: %s\r\n", hex2Str( nfcDevice->nfcid, nfcDevice->nfcidLen ) );
                            demoP2P( nfcDevice );
                        }
                        else
                        {
                            platformLog("Felica/NFC-F card found. UID: %s\r\n", hex2Str( nfcDevice->nfcid, nfcDevice->nfcidLen ));
                            
                            demoNfcf( &nfcDevice->dev.nfcf );
                        }
                        
                        break;
                    
                    /*******************************************************************************/
                    case RFAL_NFC_LISTEN_TYPE_NFCV:
                        {
                            uint8_t devUID[RFAL_NFCV_UID_LEN];
                            
                            //platformLedOn(PLATFORM_LED_V_PORT, PLATFORM_LED_V_PIN);
                            
                            ST_MEMCPY( devUID, nfcDevice->nfcid, nfcDevice->nfcidLen );   /* Copy the UID into local var */
                            REVERSE_BYTES( devUID, RFAL_NFCV_UID_LEN );                 /* Reverse the UID for display purposes */
                            platformLog("ISO15693/NFC-V card found. UID: %s\r\n", hex2Str(devUID, RFAL_NFCV_UID_LEN));
                            
                            demoNfcv( &nfcDevice->dev.nfcv );
                        }
                        break;
                        
                    /*******************************************************************************/
                    case RFAL_NFC_LISTEN_TYPE_ST25TB:
	                    platformLog("Type : ST25TB\n");
                        
                        //platformLedOn(PLATFORM_LED_B_PORT, PLATFORM_LED_B_PIN);
                        platformLog("ST25TB card found. UID: %s\r\n", hex2Str( nfcDevice->nfcid, nfcDevice->nfcidLen ));
                        break;
                    
                    /*******************************************************************************/
                    case RFAL_NFC_LISTEN_TYPE_AP2P:
                    case RFAL_NFC_POLL_TYPE_AP2P:
                        
                        //platformLedOn(PLATFORM_LED_AP2P_PORT, PLATFORM_LED_AP2P_PIN);
                        platformLog("NFC Active P2P device found. NFCID3: %s\r\n", hex2Str(nfcDevice->nfcid, nfcDevice->nfcidLen));
                    
                        demoP2P( nfcDevice );
                        break;
                    
                    /*******************************************************************************/
                    case RFAL_NFC_POLL_TYPE_NFCA:
                    case RFAL_NFC_POLL_TYPE_NFCF:
                        
                        //platformLedOn( ((nfcDevice->type == RFAL_NFC_POLL_TYPE_NFCA) ? PLATFORM_LED_A_PORT : PLATFORM_LED_F_PORT), 
                        //              ((nfcDevice->type == RFAL_NFC_POLL_TYPE_NFCA) ? PLATFORM_LED_A_PIN  : PLATFORM_LED_F_PIN)  );
                        platformLog("Activated in CE %s mode.\r\n", (nfcDevice->type == RFAL_NFC_POLL_TYPE_NFCA) ? "NFC-A" : "NFC-F");

                        if( nfcDevice->rfInterface == RFAL_NFC_INTERFACE_NFCDEP )
                        {
                            demoP2P( nfcDevice );
                        }
                        else
                        {
                            demoCE( nfcDevice );
                        }
                        break;
                    
                    /*******************************************************************************/
                    default:
						printf("none : %d\n",nfcDevice->type);
                        break;
                }
                
                rfalNfcDeactivate( RFAL_NFC_DEACTIVATE_DISCOVERY );
                state = DEMO_ST_START_DISCOVERY;
            }
            break;

        /*******************************************************************************/
        case DEMO_ST_NOTINIT:
        default:
            break;
    }
}

/*!
 *****************************************************************************
 * \brief Demo Stop
 *
 *  This function stops the demo operation
 *
 *****************************************************************************
 */
void demoStop( void )
{
    rfalNfcDeactivate( RFAL_NFC_DEACTIVATE_IDLE );
    state = DEMO_ST_NOTINIT;
}



static void demoCE( rfalNfcDevice *nfcDev )
{
#if RFAL_SUPPORT_CE && RFAL_FEATURE_LISTEN_MODE
    
    ReturnCode err;
    uint8_t *rxData;
    uint16_t *rcvLen;
    uint8_t  txBuf[150];
    uint16_t txLen;

    do
    {
        rfalNfcWorker();
        
        switch( rfalNfcGetState() )
        {
            case RFAL_NFC_STATE_ACTIVATED:
                err = demoTransceiveBlocking( NULL, 0, &rxData, &rcvLen, 0);
                break;
            
            case RFAL_NFC_STATE_DATAEXCHANGE:
            case RFAL_NFC_STATE_DATAEXCHANGE_DONE:
                
                txLen = ( (nfcDev->type == RFAL_NFC_POLL_TYPE_NFCA) ? demoCeT4T( rxData, *rcvLen, txBuf, sizeof(txBuf) ) : rfalConvBytesToBits( demoCeT3T( rxData, rfalConvBitsToBytes(*rcvLen), txBuf, sizeof(txBuf) ) ) );
                err   = demoTransceiveBlocking( txBuf, txLen, &rxData, &rcvLen, RFAL_FWT_NONE );
                break;
            
            case RFAL_NFC_STATE_START_DISCOVERY:
                return;
            
            case RFAL_NFC_STATE_LISTEN_SLEEP:
            default:
                break;
        }
    }
    while( (err == ERR_NONE) || (err == ERR_SLEEP_REQ) );

#else
    NO_WARNING(nfcDev);
#endif /* RFAL_SUPPORT_CE && RFAL_FEATURE_LISTEN_MODE */
}

/*!
 *****************************************************************************
 * \brief Demo NFC-F
 *
 * Example how to exchange read and write blocks on a NFC-F tag
 * 
 *****************************************************************************
 */
static void demoNfcf( rfalNfcfListenDevice *nfcfDev )
{
#if RFAL_FEATURE_NFCF
    
    ReturnCode                 err;
    uint8_t                    buf[ (RFAL_NFCF_NFCID2_LEN + RFAL_NFCF_CMD_LEN + (3*RFAL_NFCF_BLOCK_LEN)) ];
    uint16_t                   rcvLen;
    rfalNfcfServ               srv = RFAL_NFCF_SERVICECODE_RDWR;
    rfalNfcfBlockListElem      bl[3];
    rfalNfcfServBlockListParam servBlock;
    //uint8_t                    wrData[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };
    
    servBlock.numServ   = 1;                            /* Only one Service to be used           */
    servBlock.servList  = &srv;                         /* Service Code: NDEF is Read/Writeable  */
    servBlock.numBlock  = 1;                            /* Only one block to be used             */
    servBlock.blockList = bl;
    bl[0].conf     = RFAL_NFCF_BLOCKLISTELEM_LEN_BIT;   /* Two-byte Block List Element           */
    bl[0].blockNum = 0x0001;                            /* Block: NDEF Data                      */
    
    err = rfalNfcfPollerCheck( nfcfDev->sensfRes.NFCID2, &servBlock, buf, sizeof(buf), &rcvLen);
    platformLog(" Check Block: %s Data:  %s \r\n", (err != ERR_NONE) ? "FAIL": "OK", (err != ERR_NONE) ? "" : hex2Str( &buf[1], RFAL_NFCF_BLOCK_LEN) );
    
    #if 0  /* Writing example */
        err = rfalNfcfPollerUpdate( nfcfDev->sensfRes.NFCID2, &servBlock, buf , sizeof(buf), wrData, buf, sizeof(buf) );
        platformLog(" Update Block: %s Data: %s \r\n", (err != ERR_NONE) ? "FAIL": "OK", (err != ERR_NONE) ? "" : hex2Str( wrData, RFAL_NFCF_BLOCK_LEN) );
        err = rfalNfcfPollerCheck( nfcfDev->sensfRes.NFCID2, &servBlock, buf, sizeof(buf), &rcvLen);
        platformLog(" Check Block:  %s Data: %s \r\n", (err != ERR_NONE) ? "FAIL": "OK", (err != ERR_NONE) ? "" : hex2Str( &buf[1], RFAL_NFCF_BLOCK_LEN) );
    #endif
    
#endif /* RFAL_FEATURE_NFCF */
}

/*!
 *****************************************************************************
 * \brief Demo NFC-V Exchange
 *
 * Example how to exchange read and write blocks on a NFC-V tag
 * 
 *****************************************************************************
 */
static void demoNfcv( rfalNfcvListenDevice *nfcvDev )
{
#if RFAL_FEATURE_NFCV
    
    ReturnCode            err;
    uint16_t              rcvLen;
    uint8_t               blockNum = 1;
    uint8_t               rxBuf[ 1 + DEMO_NFCV_BLOCK_LEN + RFAL_CRC_LEN ];                        /* Flags + Block Data + CRC */
    uint8_t               *uid; 
    uint8_t               reqFlag;
#if DEMO_NFCV_WRITE_TAG
    uint8_t               wrData[DEMO_NFCV_BLOCK_LEN] = { 0x11, 0x22, 0x33, 0x99 };             /* Write block example */
#endif /* DEMO_NFCV_WRITE_TAG */
              

    uid     = nfcvDev->InvRes.UID;
    reqFlag = RFAL_NFCV_REQ_FLAG_DEFAULT;
    
    #if DEMO_NFCV_USE_SELECT_MODE
        /*
        * Activate selected state
        */
        err = rfalNfcvPollerSelect( reqFlag, nfcvDev->InvRes.UID );
        platformLog(" Select %s \r\n", (err != ERR_NONE) ? "FAIL (revert to addressed mode)": "OK" );
        if( err == ERR_NONE )
        {
            reqFlag = (RFAL_NFCV_REQ_FLAG_DEFAULT | RFAL_NFCV_REQ_FLAG_SELECT);
            uid     = NULL;
        }
    #endif /* DEMO_NFCV_USE_SELECT_MODE */

    /*
    * Read block using Read Single Block command
    * with addressed mode (uid != NULL) or selected mode (uid == NULL)
    */
    err = rfalNfcvPollerReadSingleBlock(reqFlag, uid, blockNum, rxBuf, sizeof(rxBuf), &rcvLen);
    platformLog(" Read Block: %s %s\r\n", (err != ERR_NONE) ? "FAIL": "OK Data:", (err != ERR_NONE) ? "" : hex2Str( &rxBuf[1], DEMO_NFCV_BLOCK_LEN));
 
    #if DEMO_NFCV_WRITE_TAG /* Writing example */
        err = rfalNfcvPollerWriteSingleBlock(reqFlag, uid, blockNum, wrData, sizeof(wrData));
        platformLog(" Write Block: %s Data: %s\r\n", (err != ERR_NONE) ? "FAIL": "OK", hex2Str( wrData, DEMO_NFCV_BLOCK_LEN) );
        err = rfalNfcvPollerReadSingleBlock(reqFlag, uid, blockNum, rxBuf, sizeof(rxBuf), &rcvLen);
        platformLog(" Read Block: %s %s\r\n", (err != ERR_NONE) ? "FAIL": "OK Data:", (err != ERR_NONE) ? "" : hex2Str( &rxBuf[1], DEMO_NFCV_BLOCK_LEN));
    #endif /* DEMO_NFCV_WRITE_TAG */
        
#endif /* RFAL_FEATURE_NFCV */
}

/*!
 *****************************************************************************
 * \brief Demo T2T Exchange
 *
 * Example how to exchange read blocks on a T2T tag
 * 
 *****************************************************************************
 */
static void demoT2t( void )
{
#if RFAL_FEATURE_T2T
    ReturnCode            err;
    uint16_t              rcvLen;
    uint8_t               blockNum = 0;
    uint8_t               rxBuf[ RFAL_T2T_READ_DATA_LEN ];
    
    err = rfalT2TPollerRead(blockNum, rxBuf, sizeof(rxBuf), &rcvLen);
    platformLog(" Read Block: %s %s\r\n", (err != ERR_NONE) ? "FAIL": "OK Data:", (err != ERR_NONE) ? "" : hex2Str( rxBuf, RFAL_T2T_READ_DATA_LEN));
    
#endif
}
  
/*!
 *****************************************************************************
 * \brief Demo P2P Exchange
 *
 * Sends a NDEF URI record 'http://www.ST.com' via NFC-DEP (P2P) protocol.
 * 
 * This method sends a set of static predefined frames which tries to establish
 * a LLCP connection, followed by the NDEF record, and then keeps sending 
 * LLCP SYMM packets to maintain the connection.
 * 
 * 
 *****************************************************************************
 */
void demoP2P( rfalNfcDevice *nfcDev )
{
#if RFAL_FEATURE_NFC_DEP
    
    uint16_t   *rxLen;
    uint8_t    *rxData;
    ReturnCode err;
    
    /* In Listen mode retrieve the first request from Initiator */
    if( (nfcDev->type == RFAL_NFC_POLL_TYPE_AP2P) ||  (nfcDev->type == RFAL_NFC_POLL_TYPE_NFCA ) ||  (nfcDev->type == RFAL_NFC_POLL_TYPE_NFCF ) )
    {
        demoTransceiveBlocking( NULL, 0, &rxData, &rxLen, 0);   
        
        /* Initiator request is being ignored/discarded  */
    }

    platformLog(" Initialize device .. ");
    err = demoTransceiveBlocking( ndefInit, sizeof(ndefInit), &rxData, &rxLen, RFAL_FWT_NONE);
    if( err != ERR_NONE )
    {
        platformLog("failed.\r\n");
        return;
    }
    platformLog("succeeded.\r\n");

    platformLog(" Push NDEF Uri: www.st.com .. ");
    err = demoTransceiveBlocking( ndefUriSTcom, sizeof(ndefUriSTcom), &rxData, &rxLen, RFAL_FWT_NONE);
    if( err != ERR_NONE )
    {
        platformLog("failed.\r\n");
        return;
    }
    platformLog("succeeded.\r\n");


    platformLog(" Device present, maintaining connection ");
    while(err == ERR_NONE) 
    {
        err = demoTransceiveBlocking( ndefLLCPSYMM, sizeof(ndefLLCPSYMM), &rxData, &rxLen, RFAL_FWT_NONE);
        platformLog(".");
        platformDelay(50);
    }
    platformLog("\r\n Device removed.\r\n");
    
#endif /* RFAL_FEATURE_NFC_DEP */
}


/*!
 *****************************************************************************
 * \brief Demo APDUs Exchange
 *
 * Example how to exchange a set of predefined APDUs with PICC. The NDEF
 * application will be selected and then CC will be selected and read.
 * 
 *****************************************************************************
 */
void demoAPDU( void )
{
#if RFAL_FEATURE_ISO_DEP_POLL
    ReturnCode err;
    uint16_t   *rxLen;
    uint8_t    *rxData;

    /* Exchange APDU: NDEF Tag Application Select command */
    err = demoTransceiveBlocking( ndefSelectApp, sizeof(ndefSelectApp), &rxData, &rxLen, RFAL_FWT_NONE );
    platformLog(" Select NDEF Application: %s Data: %s\r\n", (err != ERR_NONE) ? "FAIL": "OK", (err != ERR_NONE) ? "" : hex2Str( rxData, *rxLen) );

    if( (err == ERR_NONE) && (rxData[0] == 0x90U) && (rxData[1] == 0x00U) )
    {
        /* Exchange APDU: Select Capability Container File */
        err = demoTransceiveBlocking( ccSelectFile, sizeof(ccSelectFile), &rxData, &rxLen, RFAL_FWT_NONE );
        platformLog(" Select CC: %s Data: %s\r\n", (err != ERR_NONE) ? "FAIL": "OK", (err != ERR_NONE) ? "" : hex2Str( rxData, *rxLen) );

        /* Exchange APDU: Read Capability Container File  */
        err = demoTransceiveBlocking( readBinary, sizeof(readBinary), &rxData, &rxLen, RFAL_FWT_NONE );
        platformLog(" Read CC: %s Data: %s\r\n", (err != ERR_NONE) ? "FAIL": "OK", (err != ERR_NONE) ? "" : hex2Str( rxData, *rxLen) );
    }
#endif /* RFAL_FEATURE_ISO_DEP_POLL */
}


/*!
 *****************************************************************************
 * \brief Demo Blocking Transceive 
 *
 * Helper function to send data in a blocking manner via the rfalNfc module 
 *  
 * \warning A protocol transceive handles long timeouts (several seconds), 
 * transmission errors and retransmissions which may lead to a long period of 
 * time where the MCU/CPU is blocked in this method.
 * This is a demo implementation, for a non-blocking usage example please 
 * refer to the Examples available with RFAL
 *
 * \param[in]  txBuf      : data to be transmitted
 * \param[in]  txBufSize  : size of the data to be transmited
 * \param[out] rxData     : location where the received data has been placed
 * \param[out] rcvLen     : number of data bytes received
 * \param[in]  fwt        : FWT to be used (only for RF frame interface, 
 *                                          otherwise use RFAL_FWT_NONE)
 *
 * 
 *  \return ERR_PARAM     : Invalid parameters
 *  \return ERR_TIMEOUT   : Timeout error
 *  \return ERR_FRAMING   : Framing error detected
 *  \return ERR_PROTO     : Protocol error detected
 *  \return ERR_NONE      : No error, activation successful
 * 
 *****************************************************************************
 */
ReturnCode demoTransceiveBlocking( uint8_t *txBuf, uint16_t txBufSize, uint8_t **rxData, uint16_t **rcvLen, uint32_t fwt )
{
    ReturnCode err;
    
    err = rfalNfcDataExchangeStart( txBuf, txBufSize, rxData, rcvLen, fwt );
    if( err == ERR_NONE )
    {
        do{
            rfalNfcWorker();
            err = rfalNfcDataExchangeGetStatus();
        }
        while( err == ERR_BUSY );
    }
    return err;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
