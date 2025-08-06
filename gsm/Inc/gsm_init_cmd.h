#ifndef __GSM_INIT_H
#define __GSM_INIT_H

#include "gsm_port.h"
#include "gsm_at_handler.h"
#define JIO_SIM_SET_APN    1

#define GSM_INIT_DEL_ALL_TIMEOUT        1000
// 
typedef struct
{
	char *cmd;
	uint8_t numRetries;
	uint16_t cmdTimeoutInMs;
	uint16_t retryWaitIntervalInIms;
	GsmCmdResponseCbFnPtr_t respCb;
    uint8_t flagCmdSuccess;
}gsmInitCmd_st;

#if JIO_SIM_SET_APN    
typedef enum
{
    AT_CMD = 0,
    ATF_CMD,
    E0_CMD,
    CMEE_CMD,
    IPR_CMD,
    CFUN_CMD,
    //CCID_CMD,
    GMR_CMD,	
    QINISTAT_CMD,
    ENABLE_GSM_REG_EVENT_CMD,
    ENABLE_GPRS_REG_EVENT_CMD,
    CEREG_CMD,
    //GSN_CMD,
    CCID_CMD,
//    GMR_CMD,
    TEST_CMD_1,
    TEST_CMD_2,
    TEST_CMD_3,
    TEST_CMD_4,
    URCCFG_CMD,
    URCCFG_CMD_READ,
    URC_CACHE_ENABLE,
    COPS_CMD,
    SET_SMS_TEXT_MODE,
    CSCS_CMD,
    STORE_CMD,
    CPIN_CMD,
//    QSPN_CMD,
    DELETE_ALL_SMS_CMD,
    NUM_INIT_CMD	         
}gsmInitCMD_et;


#else

typedef enum
{
    AT_CMD = 0,
    ATF_CMD,
    E0_CMD,
    CMEE_CMD,
    IPR_CMD,
    CFUN_CMD, 
    QINISTAT_CMD,
    ENABLE_GSM_REG_EVENT_CMD,
    ENABLE_GPRS_REG_EVENT_CMD,
    CEREG_CMD,
    GSN_CMD,
    CCID_CMD,
    GMR_CMD,
    URCCFG_CMD,
    URCCFG_CMD_READ,
    URC_CACHE_ENABLE,
    COPS_CMD,
    SET_SMS_TEXT_MODE,
    CSCS_CMD,
    STORE_CMD,
    CPIN_CMD,
//    QSPN_CMD,
    DELETE_ALL_SMS_CMD,
    NUM_INIT_CMD	         
}gsmInitCMD_et;
#endif

typedef enum
{
	GSM_INIT_CMD_WAIT,
	GSM_INIT_CMD_IN_PROGRESS,
	GSM_INIT_CMD_FAIL,
	GSM_INIT_CMD_SUCCESS
}gsmInitCmdState_et;

typedef enum 
{
    SYSTEM_STATE_START,
    SYSTEM_STATE_ATOK,
    SYSTEM_STATE_PHBOK,
    SYSTEM_STATE_SMSOK,
    SYSTEM_STATE_ALL_READY = 7
}sysInitState_et;

#define GSM_INIT_CMD_NUM_RETRY			3
#define GSM_INIT_CMD_DEFAULT_TIMEOUT	300
#define GSM_INIT_RETRY_WAIT_MS			100

// GSM init query retry and wait milliseconds
#define GSM_QUERY_INIT_NUM_RETRY        10
#define GSM_QUERY_INIT_TIMEOUT          300
#define GSM_QUERY_INIT_RETRY_WAIT_MS	5000

#define GSM_SIM_STAT_NUM_RETRY          6
#define GSM_SIM_STAT_TIMEOUT            1000
#define GSM_SIM_STAT_RETRY_WAIT_MS      5000


#define IMEI_BUFF_SIZE          25
#define CCID_BUFF_SIZE          25
#define OPERATOR_NAME_SIM_SIZE  30

/**
 *  @brief It is called before calling the init handler it sets the state and initialises all the init cmd variables
 *  @return none
 */
void GsmInitStart(void);
/**
 *  @brief All the init commands are sent and acknowledge is checked in this function
 *  @return will return the state of the init command
 */
gsmInitCmdState_et GsmInitHandler(void);
/**
*  @brief : This function will be called when sms ready urc is received
*  @return : none
 */
void GsmInitUpdateModuleReady(void);
void GsmInitUpdateSimReady(void);
void GsmInitUpdateSimNotReady(void);
void GsmInitFunctionalityReady(void);
void GsmInitFunctionalityNotReady(void);
uint8_t* getIMEI(void);
uint8_t* getCCID(void);

//uint8_t* GetIMEINumber(void);
void GetIMEINumber(uint8_t* imeiDataArray);
void GetCCIDNumber(uint8_t* ccidDataArray);


#endif