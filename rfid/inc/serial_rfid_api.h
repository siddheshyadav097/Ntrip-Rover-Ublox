#ifndef __SERIAL_RFID_API_H
#define __SERIAL_RFID_API_H

#include "stm32g0xx_hal.h"
#include "serial_rfid_port.h"
#include "rfid_data_handler.h"

#define  RF_TAG_DETECT_ON_TIME      500//2000//500
#define  RF_TAG_DETECT_OFF_TIME     500//1000

#define  MAX_EPC_ID_RETAIN_TIME     60000
#define  READERS_DEFAULT_ADDRESS    0x00
#define  MAX_EPC_IDS_AVAILABLE      30  //max epc ids which can be stored in the ram
#define  MAX_EPC_ID_LENGTH          30  //in hex
#define  MAX_EPC_ID_LENGTH_ASCII    15  //in hexAscii
#define  SCAN_RF_TAG_TIME           100 //every 100 msec scan for new tag 


//Rfid Reader Volatge Thresholds Value
//#define VEH_24V_UPPER_TH      25.5     
//#define VEH_24V_LOWER_TH      24         
//
//#define VEH_12V_UPPER_TH      13
//#define VEH_12V_LOWER_TH      12   

typedef struct
{
	uint8_t *cmd;
        uint16_t cmdLen;
	uint8_t  numRetries;
	uint16_t cmdTimeoutInMs;
	uint16_t retryWaitIntervalInIms;
    uint8_t flagCmdSuccess;
}rfidReaderCmd_st;


typedef struct 
{
   uint8_t  epcId[MAX_EPC_ID_LENGTH_ASCII];
   uint8_t  epcIdLength;
   uint8_t  epcTagArrayIndex;
   uint32_t epcTimeStamp;
   uint8_t  epcSentFlag;
}epcId_st;

typedef struct
{
   uint8_t RfepcId[MAX_EPC_ID_LENGTH];
   uint8_t RfepcIdLen;
}RfEpcData_st;

typedef struct
{
   uint8_t  readersAdd;
   uint8_t   rfPower;
   uint16_t rfScanTime;
   uint8_t  readerWorkMode;
   uint8_t   rfMaxFrequency;
   uint8_t   rfMinFrequency;

}rfidConfigStruct_st;

typedef enum
{
	READER_CMD_WAIT,
	READER_CMD_IN_PROGRESS,
	READER_CMD_FAIL,
	READER_CMD_SUCCESS
}rfidHandlerCmdState_et;

typedef enum
{
    RFID_CMD_SEND,
    RFID_WAIT_REPLY,
    RFID_CMD_SUCCESS,
    RFID_CMD_FAILURE,
    RFID_CMD_RETRY_WAIT
}rfidCmdState_et;

typedef enum
{
     RFID_POWER_RESET = 0,
     RFID_SET_SYSTEM_RESET,
     RFID_GET_ANTI_COLL_MODE,
     RFID_GET_C_AI_PARAM,
//     RFIS_GET_RDR_INFO,
     RFID_GET_RDR_INFO_SN,
     RFID_GET_RDR_POWER,
     RFID_SET_RDR_POWER,
     RFID_GET_REGION,
     RFID_SET_REGION,
     RFID_GET_FHLBT_PARAM,
     RFID_START_AUTO_READ,
     RFID_READER_WAIT,
     RFID_STOP_AUTO_READ,
}rfidStateMachine_et;


typedef enum
{
  SET_SYSTEM_RESET = 0,
  GET_ANTI_COLL_MODE,
  GET_C_AI_PARAM,
//  GET_RDR_INFO,
  GET_RDR_INFO_SN,
  GET_RDR_POWER,
  SET_RDR_POWER,
  GET_REGION,
  SET_REGION,
  GET_FHLBT_PARAM,
  START_AUTO_READ,
  STOP_AUTO_READ
  
}rfidCMD_et;

typedef enum
{
    READER_OFF,
    READER_ON
}rdrPwrState_et;

typedef enum
{
   LED_IDLE_STATE,
   TOGGLE_STATE
}rfidLedState_et;

rfidConfigStruct_st* RfidGetReaderConfigrations(void);
void RfidPowerStateCheckHandler(void);
void RfidReaderSetState(rfidStateMachine_et state);
rfidStateMachine_et GetRfidReaderSendState(void);
uint8_t getRfidReadTagStatus(void);
void RfidGetReaderPwrTh(void);
void RfidSetCurrentThresholds(void);
void RfidInit(void);
uint8_t RfidGetAvailableTagsCnt(void);
uint8_t RfidGetEPCData(uint8_t getDataCntr,RfEpcData_st *RfDataPtr);
void RfidEpcDataClearHandler(void);
void RfidClearEpcStruct(void);
rfidHandlerCmdState_et RfidCommandHandler(void);
void RfidStateMachineHandler(void);
void RfidReaderHandler(void);
void RfidBytesToHex(const uint8_t *bytes, uint32_t size, char *hex);


#endif