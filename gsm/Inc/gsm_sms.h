#ifndef __GSM_SMS_H__
#define __GSM_SMS_H__

#include "gsm_port.h"
#include "gsm_at_handler.h"


#define SEND_SMS_TIMEOUT          15000    //30sec - actual timeout is 120sec
#define READ_SMS_TIMEOUT          15000    
#define DELETE_SMS_TIMEOUT        1000     //500msec

typedef enum
{
  READ_SMS_TEXT_MODE,
  READ_SMS
}gsmSmsReadCMD_et;

typedef enum
{
    SMS_SEND_WAIT_FOR_START,
    SMS_SEND_START,
    SMS_SENDING_IN_PROGRESS,
    SMS_SENDING_SUCCESS,
    SMS_SENDING_FAIL,
}smsSendState_et;

typedef enum
{
    SMS_DELETE_WAIT_FOR_START,
    SMS_DELETE_START,
    SMS_DELETE_IN_PROGRESS,
    SMS_DELETE_SUCCESS,
    SMS_DELETE_FAIL,
}smsDeleteState_et;

typedef enum
{
    SMS_READ_WAIT_FOR_START,
    SMS_READ_START,
    SMS_READING_IN_PROGRESS,
    SMS_READING_SUCCESS,
    SMS_READING_FAIL,
}smsReadState_et;

typedef struct
{
	char *cmd;
	uint16_t cmdTimeoutInMs;
	GsmCmdResponseCbFnPtr_t respCb;
}smsAtCmd_st;

//typedef  struct

typedef  struct
{
   uint8_t rcvMobileNo[20];
   uint16_t rcvSmsLength;
   uint8_t rcvContentData[256];
}gsmReadSmsData_st;

typedef enum 
{
    SMS_DEL_INDEXED_MSG = 0, 			/* Single message by index */
    SMS_DEL_READ_MSG = 1,         		/* Already read messages */
    SMS_DEL_READ_SENT_MSG = 2,   		/* Read and sent messages */
    SMS_DEL_READ_SENT_UNSENT_MSG = 3, 	/* Read ,sent and unsent messages */
    SMS_DEL_ALL_MSG = 4					/* All messages in current storage */
} smsDeleteFlag_et;

typedef enum
{
    GSM_SMS_IDLE,
    GSM_SMS_READ,
    GSM_SMS_SEND,
    GSM_SMS_DELETE
}gsmSmsState_et;

typedef void (*GsmReceiveSmsCallbackFnPtr_t)(uint8_t* phNum, uint8_t* smsData, uint16_t smsLen);

void GsmSmsInit(GsmReceiveSmsCallbackFnPtr_t fnPtr);
void SendSMS(uint8_t* phno, char* send_buf, uint8_t length);
void GsmGetSenderMobileNo(uint8_t* cmgrLinePtr,uint16_t cmgrLineLen);
void GsmDeleteSMS(void);
void GsmSendSMS(void);
void GsmSmsHandler(void);
void UrcSmsCb(uint8_t *lineBuff, uint16_t len);
void GsmSmsRead(uint8_t smsIndex);
uint8_t GsmGetSendState(void);






















#endif