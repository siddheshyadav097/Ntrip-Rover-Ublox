#ifndef __GSM_IDLE_TASK_H
#define __GSM_IDLE_TASK_H
#include "gsm_port.h"
#include "gsm_statemachine.h"

// interval to poll the gprs status every particular interval
#define GSM_POLL_GPRS_ACT_INTERVAL_MS      (30000)
#define GSM_POLL_OPERATOR_INTERVAL_MS      (60000)
#define GSM_POLL_NW_INFO_INTERVAL_MS       (60000)
#define GSM_POLL_RECV_SMS_INTERVAL_MS      (60000)
#define GSM_POLL_DATE_TIME_INTERVAL_MS     (60000)
#define GSM_POLL_SIG_STRENGTH_INTERVAL_MS  (60000)
#define GSM_POLL_CELL_ID_INTERVAL_MS       (60000)

#define GPRS_STATUS_CMD_TIMEOUT             500
#define GSM_CELL_ID_CMD_TIMEOUT             1000
#define GSM_SIG_STREN_CMD_TIMEOUT           300
#define GSM_DATE_TIME_CMD_TIMEOUT           300
#define GSM_OPERATOR_NAME_CMD_TIMEOUT       300
#define GSM_QUERY_NW_INFO_CMD_TIMEOUT       300
#define GSM_QUERY_GPRS_ACT_CMD_TIMEOUT      150000

#define OPERATOR_SELECTION      30

typedef enum
{
    //GSM_POLL_SOC_STATUS,
//    GSM_POLL_CELL_ID,
//    GSM_POLL_GPRS_ACT,
    GSM_POLL_RECV_SMS,
    GSM_POLL_SIGNAL_STRENGTH,
    GSM_POLL_NW_TIME,
    GSM_POLL_GET_OPERATOR_NAME,
    GSM_POLL_NW_INFO,
    GSM_NUM_POLL_CMD
}gsmPollCmd_et;

typedef struct
{
	char *cmd;
	uint32_t cmdTimeoutInMs;
	GsmCmdResponseCbFnPtr_t respCb;
}gsmPollCmd_st;


typedef enum {                                                                  //gprs states for single and multiple connections
    GPRS_INITIAL = 0,
    GPRS_STARTED = 1,
    GPRS_CONFIG  = 2,
    GPRS_IND     = 3,
    GPRS_ACTIVE  = 4,                                                           //IP GPRSACT
    GPRS_STATUS  = 5,
//  GPRS_IP_PROCESSING=6,
//  TCP_SOC_PROCESSING = 6,
    TCP_CONNECT_OK     = 6,
//	SOCKET_CLOSE       = 8,
	GPRS_DEACTIVE      = 7,                                                     //PDP DEACT
	GPRS_STATUS_END    = 8
}gprsState_et;


void GetGsmTime(gsmTimePara_t* gsmTimeParaPtr);
gsmRespCb_et GsmDateTimeRespCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len);
void GsmIdleTaskStart(void);
void GsmIdleTaskHandler(void);

void GsmPollStatusSetCmd(gsmPollCmd_et cmd);
gsmCmdState_et GsmPollStatusHandler(void);

void SetPowerResetModemFlag(void);
void DeactGprsWhenIdle(void);
uint8_t* GetOperatorName(void);
uint8_t* GetNetworkInfoName(void);
void GetOperatorNameInit(uint8_t* opsDataArray);

#endif