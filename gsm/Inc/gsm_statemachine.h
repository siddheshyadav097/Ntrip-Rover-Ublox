#ifndef __GSM_STATEMACHINE_H
#define __GSM_STATEMACHINE_H

#include "gsm_port.h"
#include "gsm_reset_power.h"
#include "gsm_init_cmd.h"
#include "gsm_common.h"

#define GSM_NW_REG_CHECK_CMD_TIMEOUT_MS     300
#define GSM_NW_REG_CHECK_NUM_RETRY          5//15
#define GSM_NW_REG_CHECK_RETRY_INTERVAL_MS  2000//20000
#define SIM_NOT_PRESENT_RESTART_MS          120000    //2 minutes
#define MAX_GPRS_ACT_TIMEOUT                300000    //5 minutes
#define MAX_GPRS_DEACT_TIMEOUT              40000
#define GSM_QUERY_OPS_TIMEOUT_MS            5000//30000

#define GPRS_ACT_MAX_RETRY                  2

typedef enum{
    GSM_NO_INIT = 0,
    GSM_HW_POWER_RESET,
    GSM_SW_POWER_RESET,
    GSM_INIT,		            
	SIM_CARD_NOT_PRESENT,		
	GSM_MODEM_NOT_RESPONDING,	
	CHECK_GSM_NW_STATUS,		
    GSM_GET_SIGNAL_STRENGTH,
	 GSM_GET_OPS_NAME,
	GSM_NOT_REGISTERING,		
	GSM_MODEM_IDLE,		
    //QUERY_NETWORK_TIME,
	POLL_STATUS,			    
	CHECK_GPRS_REG_STATUS,
    GPRS_CHECK_SSL_CONFIG,
//	GPRS_NOT_REGISTERING,	
    GPRS_NOT_ACTIVATING,    
	GPRS_ACTIVATE,			    
	GPRS_DEACTIVATE,		    
    GSM_SOCKET_PROCESSING,
    GSM_HTTP_SOC_PROCESSING,
    GSM_SMS_PROCESSING,
    GSM_FTP_PROCESSING,
    GSM_FILE_PROCESSING
}gsmState_et;

typedef enum 
{
    NETWORK_STAT_NOT_REGISTERED = 0,                                            // Not register to network
    NETWORK_STAT_REGISTERED = 1,                                                    // The normal network state
    NETWORK_STAT_SEARCHING = 2,                                                     // Searching network
    NETWORK_STAT_REG_DENIED = 3,                                                    // The register request is denied
    NETWORK_STAT_UNKNOWN= 4,
    NETWORK_STAT_REGISTERED_ROAMING= 5                                            //Registered and Roaming state
}gsmNetworkState_et;


void GsmStateMachineInit(void);
gsmState_et GetGsmState(void);
void GsmSetState(gsmState_et state);
void GsmStateMachine(void);
void GsmUpdateGprsRegistration(uint8_t flagRegistered,gsmNetworkState_et nwState);
uint8_t GsmIsNetworkRegistered(void);
uint8_t GsmIsGprsRegistered(void);
void GsmUpdateNetworkRegistration(uint8_t flagRegistered, gsmNetworkState_et nwState);
gsmState_et GsmGetModemState(void);
uint8_t GsmStateIsIdle(void);
uint8_t GsmStateIsFileProcessing(void);

#endif
