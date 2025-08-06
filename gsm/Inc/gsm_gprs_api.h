#ifndef __GSM_GPRS_API_H
#define __GSM_GPRS_API_H

#include "gsm_port.h"
#include "gsm_at_handler.h"
#include "mem_config_api.h"

#define PDP_CONTEXT_ID      0

#define CONTEXT_ID          1   //range is 1-16
#define CONTEXT_TYPE        1   //IPV4

#define GSM_APN_BUFF_SIZE	    100
#define USER_BUFF_SIZE	    20
#define PASS_BUFF_SIZE	    20

#define GPRS_INIT_CMD_DEFAULT_TIMEOUT       300
#define	READ_SOCKET_URC_TIMEOUT	            500  
#define GPRS_ACT_CMD_TIMEOUT                150000
#define GSM_GET_LOC_IP_TIMEOUT              2000

#define	GPRS_PDP_DEACT_CMD_TIMEOUT	        40000     //actual timeout is 40sec 

typedef enum
{
    GPRS_WAIT_FOR_ACTIVATE,
    GPRS_ACTIVATE_IN_PROGRESS,
    GPRS_ACTIVATION_SUCCESS,
    GPRS_ACTIVATION_FAIL,
}gprsActivateState_et;

typedef enum
{
//   ENABLE_TCPIP_MUX=0,
//   SET_PDP_CONTEXT,
   CONFIGURE_GPRS = 0,
//   START_TCPIP_TASK,
//   ENABLE_READ_SOCKET_DATA_EVENT,
//   ENABLE_DOMAIN_NAME,
   ACTIVATE_GPRS = 1,
//   GET_LOCAL_IP_FOR_CONNECTION,
   GPRS_NUM_INIT_CMDS
}gprsInitCmd_et; 

typedef struct
{
	char *cmd;
	uint32_t cmdTimeoutInMs;
	GsmCmdResponseCbFnPtr_t respCb;
}gprsInitCmd_st;

void GprsInit(void);
void GprsSetSim1Config(gprsConfig_st *config);
void GprsSetSim2Config(gprsConfig_st *config);

void GprsActivateStart(void);
gprsActivateState_et GprsActivationHandler(void);

typedef enum
{
    GPRS_DEACTIVATE_WAIT_FOR_START,
    GPRS_DEACTIVATE_IN_PROGRESS,
    GPRS_DEACTIVATION_SUCCESS,
    GPRS_DEACTIVATION_FAILURE,
}gprsDeactivateState_et;

uint8_t GsmGprsIsActive(void);
void GetGprsConfigsFromMem(void);

void getGPRSmemconfig(void);
void GprsDeactivateStart(void);
gprsDeactivateState_et GprsDeactivationHandler(void);
void GsmResetGprsActFlag(void);
void UrcGprsDeactiveCb(uint8_t *lineBuff, uint16_t len);
void GsmResetGprsActTime(void);
void UpdateSimApn(char* apnString,uint8_t apnLen);;

#endif