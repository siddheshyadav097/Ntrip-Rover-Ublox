#ifndef __GSM_AT_HANDLER_H
#define  __GSM_AT_HANDLER_H
#include "gsm_port.h"
#include "ring_buffer.h" 
#include "gsm_process_resp.h"
#include "gsm_urc_api.h"

#define MAX_GSM_RX_BUFFER_SIZE		1024

typedef enum
{
    GSM_CMD_IDLE, // this state waits for at command and also checks for URC response
    GSM_SEND_CMD,
    GSM_CHECK_CMD_RESPONSE
}gsmResponseState_et;

typedef enum
{
    GSM_SEND_CMD_SUCCESS,
    GSM_SEND_CMD_FAILURE    
}gsmSendCmdRet_et;

typedef enum
{
    AT_CMD_SEND,
    AT_WAIT_REPLY,
    AT_CMD_SUCCESS,
    AT_CMD_FAILURE,
    AT_CMD_RETRY_WAIT
}gsmCmdState_et;

typedef enum
{
	GSM_WAIT_FOR_LINE,
	GSM_WAIT_FOR_FIXED_LEN
}gsmProcessReceiveState_et;

typedef enum
{
    GSM_RESP_CB_WAIT,
    GSM_RESP_CB_OK_COMPLETE,
    GSM_RESP_CB_ERROR_COMPLETE
}gsmRespCb_et;

typedef gsmRespCb_et (*GsmCmdResponseCbFnPtr_t)(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len);
typedef void (*GsmReadFixedLenDataFnPtr_t)(uint8_t receiveData);

void GsmAtHandlerInit(void);

uint8_t GsmIsAtIdle(void);

gsmSendCmdRet_et GsmSendAtCmd(char *cmdBuf, uint16_t cmdLen, uint32_t timeout,GsmCmdResponseCbFnPtr_t gsmRespCbFnPtr);

void GsmResponseHandler(void);

void GsmReadFixedLengthData(GsmReadFixedLenDataFnPtr_t fnPtr ,uint16_t length);



#define MAX_SOCKETS 2

typedef struct {
    GsmReadFixedLenDataFnPtr_t cb;
    uint16_t totalLen;
    uint16_t received;
    uint8_t flagDataReceived;
    uint8_t inProgress;
} GsmSocketRx_t;

void GsmReadFixedLengthDataIndexed(uint8_t socketIndex, GsmReadFixedLenDataFnPtr_t fnPtr, uint16_t length);
















#endif