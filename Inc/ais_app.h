#ifndef __AIS_APP_H
#define __AIS_APP_H

#include <stdint.h>
#include "fota_api.h"
#include "packet_api.h"
#include "led_api.h"

#define SOCK_RESP_WAIT_TIMEOUT          5000//10000
#define WAIT_FOR_MEMORY_CHECK		    30

#define MAX_SOCK_OPEN_TIMEOUT          80000
#define MAX_SOCK_CLOSED_TIMEOUT        5000
#define SOCK_ACK_URC_TIMEOUT           10000
#define SOCK_CLOSED_URC_TIMEOUT        2000

#define ENABLE_ACK_SMS                  1

#define SOCK_RESP_START_BYTE           '$'
#define SOCK_RESP_END_BYTE             '*'

#define CONFIGPASSWORD                  "1234"

#define MAX_SIZE_OF_READ_BUFF       4096

#define BUZZER_EN_GPIO_PIN      GPIO_PIN_12
#define BUZZER_EN_GPIO_PORT      GPIOB




#define RELAY_EN_GPIO_PIN      GPIO_PIN_11
#define RELAY_EN_GPIO_PORT      GPIOB
typedef enum
{
   BUZZER_IDLE_STATE,
   BUZZER_TOGGLE_STATE
}BuzzerState_et;

typedef enum 
{
	GSM_INITIALIZED,
	GSM_REGISTERED,
	GSM_NOT_REGISTERED,
	GPRS_REGISTERED,
	GPRS_DEREGISTERED,
	GPRS_ACTIVATED,
	GPRS_DEACTIVATED,
    GSM_NOT_INITIALIZED,
    PACKET_SENT_SUCCESS,
    PACKET_SENT_FAILED
}gsmStatus_et;

typedef enum
{
    SOCK_RESP_CHECK_START,
    SOCK_RESP_CHECK_END,
    SOCK_RESP_CHECK_DATA,
    SOCK_RESP_CHECK_CMD,
    SOCK_RESP_SUCCESS,
    SOCK_RESP_FAIL
}socketResponse_et;

typedef enum
{
    SOCK_ACK_CHECK_START,
    SOCK_ACK_CHECK_PACKET_NUM,
    SOCK_ACK_CHECK_WAIT,
    SOCK_ACK_CHECK_SUCCESS,
    SOCK_ACK_CHECK_FAIL
}socketAck_et;

typedef enum
{
    PVT_WAIT_FOR_PACKET,
    PVT_WAIT_FOR_SOCK_OPEN,
    PVT_IS_SOCK_OPENED,
    PVT_SOCK_OPEN_FAIL,    
    PVT_SOCK_OPEN_TIMEOUT,   
    PVT_PACKET_WRITE_START,
    PVT_GET_WRITE_STATUS,
    PVT_WAIT_FOR_ACK_URC,
    PVT_GET_SOCK_ACK,
    PVT_ACK_SUCCESS,
    PVT_SOCK_WRITE_FAIL,
    PVT_GET_ACK_FAIL,
    PVT_CLOSE_SOCKET,
    PVT_GPRS_DEACT_STATE
    
}pvtSendHandler_et;


typedef enum
{
    NTRIP_WAIT_FOR_PACKET,
    NTRIP_WAIT_FOR_SOCK_OPEN,
    NTRIP_IS_SOCK_OPENED,
    NTRIP_SOCK_OPEN_FAIL,    
    NTRIP_SOCK_OPEN_TIMEOUT,   
    NTRIP_PACKET_WRITE_START,
    NTRIP_GET_WRITE_STATUS,
    NTRIP_WAIT_FOR_ACK_URC,
    NTRIP_GET_SOCK_ACK,
    NTRIP_ACK_SUCCESS,
    NTRIP_SOCK_WRITE_FAIL,
    NTRIP_GET_ACK_FAIL,
    NTRIP_CLOSE_SOCKET,
    NTRIP_GPRS_DEACT_STATE
    
}ntripSendHandler_et;

typedef enum {
    HEADER_STATE_START,
    HEADER_STATE_I,
    HEADER_STATE_C,
    HEADER_STATE_Y,
    HEADER_STATE_SPACE1,
    HEADER_STATE_2,
    HEADER_STATE_0,
    HEADER_STATE_0_2,
    HEADER_STATE_SPACE2,
    HEADER_STATE_O,
    HEADER_STATE_K,
    HEADER_STATE_CR,
    HEADER_STATE_LF,
    HEADER_STATE_COMPLETE
} HeaderParseState;

typedef  struct
{
	uint8_t debugCmdBuff[512];
	uint16_t debugCmdLen;
}DebugCmd_st;

void PvtSendHandler(void);
//void GetSerialCommand(uint8_t* receiveData,uint16_t fuelDataLen);
void SocketRespSetState(socketResponse_et state);
socketResponse_et SocketResponseHandler(void);
pvtSendHandler_et GetPvtSendState(void);
uint8_t PvtSendIsWaitForPacket(void);
uint8_t PvtIsWaitForConnPacket(void);
uint8_t PvtSendPacket(uint8_t* packetBuff , uint16_t packetlen,uint32_t packetNumber);
uint8_t PvtSendConnPacket(uint8_t* packetBuff , uint16_t packetlen);
uint8_t AisSendDataPacket(uint8_t sockId , char* buffPtr , uint16_t bufflen);
uint8_t NtripSendDataPacket(uint8_t sockId , char* buffPtr , uint16_t bufflen);
void GetAllMemoryConfigurations(void);
void GetIntervalsConfigfrmMem(void);
void GetGprsConfigurations(void);
void AisAppInit(void);
void AisDebugInit(void);
void PvtSendSetState(pvtSendHandler_et state);
void GetAISSockets(void);

void SendFOTAProcessResponse(fotaStatus_et fotaState);
void  GetUnitIdFrmMemForAppFile(void);
void serialCmdHandler(void);
void writeBuzzerPinState(uint8_t pinState);

void writeRelayPinState(uint8_t pinState);
void BuzzerEnableInit(void);
void RelayEnableInit(void);
void BuzzerHandler();
void ToggleBuzzer();
void StartRelayHandler(void);


//ntrip
void NtripSendSetState(ntripSendHandler_et state);
void rtcm_header_Parse(void);
void rtcm_parser(void);


typedef enum {
    RTCM_STATE_WAIT_HEADER,    // Waiting for ICY 200 OK header
    RTCM_STATE_PROCESS_DATA    // Processing RTCM frames after header is received
} rtcm_state_t;


typedef enum {
    RTCM_FRAME_START,          // Looking for start byte
    RTCM_FRAME_LENGTH,         // Reading length bytes
    RTCM_FRAME_DATA,           // Reading data bytes
    RTCM_FRAME_CRC             // Reading CRC bytes
} rtcm_frame_state_t;

void rtcm_reset(void);
void NtripSendHandler(void);
void MakeNtripHttpHeader(void);
uint8_t NtripSendPacket(uint8_t* packetBuff , uint16_t packetlen);
uint8_t NtripSendIsWaitForPacket(void);
uint8_t NtripSendIsWaitForPacket(void);
ntripSendHandler_et GetntripSendState(void);
void rtcmbuffreset(void);
void encode_basic_auth_credentials(const char *username, const char *password, char *output_b64);
void base64_encode(const unsigned char *input, int length, char *output);


#endif

