#ifndef __RFID_DATA_HANDLER_H
#define __RFID_DATA_HANDLER_H

#include "serial_rfid_port.h"
#include "serial_rfid_api.h"


#define PRESET_VALUE                0xFFFF
#define POLYNOMIAL                  0x8408

#define MAX_RFID_RX_BUFFER_SIZE		1024

#define MAX_GET_RF_RESP_TIMEOUT     300

#define RFID_PREAMBLE_BYTE         0xBB
#define RFID_ENDMARK_BYTE          0x7E

/*Command packets are user-to-reader RCP packets. 
Response and notification RCP packets are reader-to-user RCP packets*/
typedef enum
{
  MSG_TYPE_COMMAND      = 0x00,
  MSG_TYPE_RESPONSE     = 0x01,
  MSG_TYPE_NOTIFICATION = 0x02,
  MSG_TYPE_RESERVED     = 0x03
}rfidMsgType_et;


typedef enum
{
   RF_RESP_TIMEOUT              = 0x00,
   SET_SYSTEM_RESET_CMD         = 0x08,      
   GET_ANTI_COL_MODE_CMD        = 0x34,   
   GET_TYPE_CA_QUERY_CMD        = 0x0D,     
//   GET_READER_INFO_MODE_CMD     ,   
   GET_READER_INFO_SN_CMD       = 0x03,     
   GET_READER_POWER_CMD         = 0x15,       
   SET_READER_POWER_CMD         = 0x16,       
   GET_REGION_CMD               = 0x06,            
   SET_REGION_CMD               = 0x07,            
   GET_FHLBT_PARAM_CMD          = 0x13,
   START_AUTO_READ_CMD          = 0x36,
   STOP_AUTO_READ_CMD           = 0x37,
   START_AUTO_READ_NOTIFICATION = 0x22,


}rfidReaderCmdCode_et;

//Response and their matching return values
typedef struct
{
    uint8_t response;
    rfidReaderCmdCode_et  respCode;
}rfidCmdResp_st;

typedef enum
{
   RFID_WAIT_FOR_PREAMBLE,
   RFID_WAIT_FOR_HEADER,
   RFID_READ_COMMAND_PAYLOAD_DATA,
   RFID_READ_EPC_PAYLOAD_DATA,
   RFID_WAIT_FOR_ENDMARK_CRC,
   RFID_CHECK_CRC,
   RFID_DATA_RECV_TIMEOUT  
}rfidProcessRespState_et;


typedef enum
{
    RFID_CMD_IDLE, // this state waits for at command and also checks for URC response
    RFID_SEND_CMD,
    RFID_CHECK_CMD_RESPONSE
}rfidResponseState_et;

typedef enum
{
    RFID_SEND_CMD_SUCCESS,
    RFID_SEND_CMD_FAILURE    
}rfidSendCmdRet_et;

typedef enum
{
    RFID_RESP_CB_WAIT,
    RFID_RESP_CB_OK_COMPLETE,
    RFID_RESP_CB_ERROR_COMPLETE
}rfidRespCb_et;


void RfidCmdHandlerInit(void);
void uiCrc16Cal(unsigned char* pData,  uint16_t DataLen);
unsigned int CheckCRC(unsigned char* pData, uint16_t len);
uint8_t RfidIsCmdIdle(void);

typedef rfidRespCb_et (*readerCmdResponseCbFnPtr_t)(rfidReaderCmdCode_et cmdCode, uint8_t *lineBuff, uint16_t len);
rfidSendCmdRet_et SendDataToRfidPort(uint8_t *buf,uint16_t len,uint16_t timeout,readerCmdResponseCbFnPtr_t rfRespCbFnPtr);

void RfidResponseHandler(void);
void GetRfidResponseData(void);
void RfidDataHandlerReset(void);
uint16_t RfidGetPayloadLen(void);

#endif