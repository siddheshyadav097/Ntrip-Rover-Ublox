#ifndef __GSM_SOCKET_API_H
#define __GSM_SOCKET_API_H

#include "gsm_port.h"
#include "gsm_at_handler.h"
#include "packet_api.h"

#define GSM_SOCK_OPEN_TIMEOUT           15000//(75000) //actual timeout for this command is 75sec  -- 15sec    
#define	GSM_SOCK_SEND_DATA_TIMEOUT	    500     //300msec
#define GSM_CONNECT_MODE            	0       //0 - Non-Transperant connect mode/1-Transperant Mode
#define SSL_SOC_OPEN_TO             	90
#define GSM_SOCK_MAX_READ_DATA_LEN    	1500   //1024
#define	GSM_SOCK_READ_DATA_TIMEOUT	    1000   //5000         //500msec
#define GSM_SOCK_CLOSE_TIMEOUT			2000   //300
//#define GSM_SOCK_RECONNECT_TIMEOUT_MS   (10000)
#define GSM_POLL_READ_BUFF_TO           3000// 60000

#define MAX_GSM_SOCK_CONNECT_TIMEOUT    180000   //max timeout to check for socket connection - 3 minutes

#define GSM_SOCK_CHECK_NUM_RETRY          3

#define GSM_POWER_RESET_ON_CONN_MS              1200000 //20MINUTES

#define SERVICE_TYPE                    "TCP"

#define MAX_LEN_TO_BE_READ              1500
#define MAX_SOC_SERVICE_INDEX           12



typedef enum
{
	GSM_SOCK_NOT_READY = 0, 
	GSM_SOCK_CHECK_OPEN,          //sockets idle state
	GSM_SOCK_OPEN,            	// open the socket here
	GSM_SOCK_OPENED,			// socket is opened and is idle 
	GSM_SOCK_WRITE,				// write socket data is in progress
    GSM_SOCK_READ,				// read socket data is in progress
    GSM_SOCK_CLOSE              // socket closed
}gsmSockState_et;


typedef enum
{
	GSM_SOCK_OPEN_WAIT_FOR_START,
	GSM_SOCK_OPEN_START,
	GSM_SOCK_OPEN_IN_PROG,
	GSM_SOCK_OPEN_SUCCESS,
	GSM_SOCK_OPEN_FAIL,
}gsmSockOpenState_et;

typedef enum
{
    GSM_SOCK_WRITE_WAIT_FOR_START,
	GSM_SOCK_WRITE_START,
    GSM_SOCK_WRITE_IN_PROG,
    GSM_SOCK_WRITE_SUCCESS,
    GSM_SOCK_WRITE_FAIL
}gsmSockWriteState_et;

typedef enum
{
	GSM_SOCK_READ_WAIT_FOR_START,
	GSM_SOCK_READ_START,
    GSM_SOCK_READ_IN_PROG,
    GSM_SOCK_READ_SUCCESS,
    GSM_SOCK_READ_FAIL
}gsmSockReadState_et;

typedef enum
{
	GSM_SOCK_CLOSE_WAIT_FOR_START,
	GSM_SOCK_CLOSE_START,
	GSM_SOCK_CLOSE_IN_PROG,
	GSM_SOCK_CLOSE_SUCCESS,
	GSM_SOCK_CLOSE_FAIL
}gsmSockCloseState_et;

typedef enum
{
    GSM_SOCK_WRITE_BUSY,
    GSM_SOCK_WRITE_STARTED,
}gsmSockWriteRet_et;

typedef enum
{
    GSM_SOCK_IP_ADDR,
    GSM_SOCK_DOMAIN_NAME
}gsmSockAddress_et;

typedef enum
{
  GSM_SOC_OPEN =0,
  GSM_SOC_SEND_DATA,
  GSM_SOC_READ_DATA,
  GSM_SOC_CLOSE,
  GSM_QUERY_STATE,
  GSM_SOCK_NUM_CMDS
}gsmSocAtCmds_et;

typedef enum
{
    GSM_SOCK_UNSECURE = 0,          // WITHOUT TLS plain socket
    GSM_SOCK_SECURE             // WITH TLS or SSL
}gsmSockSecureType_et;

typedef enum
{
   SSL_SOCKET_OPEN_SUCCESS = 0,
   SSL_SOCKET_OPEN_ERROR = -1,
   SSL_SOCKET_OCCUPIED = -2
}gsmSSLSocketReply_et;


typedef struct
{
    uint8_t sockId;
    uint8_t flagOpenCmd;
    gsmSockState_et state;
    uint8_t *hostAddress;    //this will be the domain name or Ip
    uint8_t* hostPath;
    uint8_t* serverReqType;
    uint16_t portNum;
	gsmSockOpenState_et openState;
    gsmSockWriteState_et writeState;
	gsmSockCloseState_et closeState;
    uint8_t *writeBuf;
    uint16_t writeLen;
    gsmSockReadState_et readState;
    ringBuffer_st readBuf;
    gsmSockSecureType_et security;
//    uint32_t reconnectTimeout;
    uint16_t reconnectCnt;
}gsmSocket_st;

#define GSM_SOCK_NOT_AVAILABLE      255
#define GSM_SOCK_MAX_AVAILABLE      2
//#define MAX_SIZE_OF_READ_BUFF       1500

void GetServerConfigurations(void);


/**
 *  @brief  :Initialise all the variables of socket instance and 
 *  @param  :[in] address      - ip address or domain name of the server
 *  @param  :[in] portNum      - port number
 *  @param  :[in] readBuf      - pointer of read buffer 
 *  @param  :[in] readBuf      - size of read buffer
 *  @param  :[in] sockAddrType - GSM_SOCK_IP_ADDR or GSM_SOCK_DOMAIN_NAME
 *  @param  :[in] security     - GSM_SOCK_UNSECURE or GSM_SOCK_SECURE
 *  @return :   returns the socket ID all the other functions of socket api will use this socket number
 *              returns GSM_SOCK_NOT_AVAILABLE if all sockets are used
 *            
 */
uint8_t GsmSocketGet(uint8_t* hostNamePtr ,uint8_t* pathPtr, uint16_t portNum,uint8_t* serverReqType, uint8_t *readBuf, uint16_t readBuffLen,gsmSockSecureType_et security);

/**
 *  @brief  : socket open command is initiated
 *  @param  :[in] socketId
 *  @return : no ret val
 */
void GsmSocketOpen(uint8_t socketId);

/**
 *  @brief  : socket close command is initiated will happen only when the socket is open
 *  @param  :[in] socketId
 *  @return : no ret val
 */
void GsmSocketClose(uint8_t socketId);

uint8_t GsmSockCloseFlagStatus(void);


void GsmResetSockCloseFlag(void);


/**
 *  @brief  : get the state of the socket
 *  @param  :[in] socketId 
 *  @return :state of socket
*           typedef enum
            {
                GSM_SOCK_INIT,
                GSM_SOCK_WAIT_OPEN,
                GSM_SOCK_OPENED,
                GSM_SOCK_CLOSE
            }gsmSockState_et;
 */
gsmSockState_et GsmSocketGetState(uint8_t socketId);


/**
 *  @brief  : Socket Write Start Command will check whether socket is opened and 
 *  @param  :[in] sockId: socket number should be less than the max socket number
 *  @return : will return socket write failure if 
 */
gsmSockWriteRet_et GsmSocketWrite(uint8_t sockId, uint8_t *buff, uint16_t writeLen);

/**
 *  @brief  :Brief
 *  @param  :[in] sockId socket Id
 *  @return : returns the state of writing 
 *              typedef enum
                {
                    GSM_SOCK_WRITE_START,
                    GSM_SOCK_WRITE_IN_PROG,
                    GSM_SOCK_WRITE_SUCCESS,
                    GSM_SOCK_WRITE_FAIL
                }gsmSockWriteState_et;
 */
gsmSockWriteState_et GsmSockGetWriteState(uint8_t sockId);

gsmSockCloseState_et GsmSockGetCloseState(uint8_t sockId);
                
uint16_t GsmSocketGetAvlData(uint8_t sockId);

/**
 *  @brief  : will read the data received from the socket upto readLen 
 *  @param  :[in] sockId  socket Id
 *  @param  :[in] readBuff received data is copied in this buffer
 *  @param  :[in] readLen  length of data to be read
 *  @return : will return the actual read data it can be smaller than the read Len
 */
uint16_t GsmSocketRead(uint8_t sockId, uint8_t *readBuff, uint16_t readLen);

void GsmSocketStateReset(void);
void GsmSocketHandler(void);
uint8_t GsmSocketIsAnySecure(void);
uint8_t GsmSocketIsAnyUnsecure(void);

void GsmUpdateIpPort(uint8_t sockId, uint8_t* hostNamePtr ,uint8_t* pathPtr, uint16_t portNum,uint8_t* serverReqType);
void UrcUnsecuredSocketReadCb(uint8_t *lineBuff, uint16_t len);
void GsmSocketCloseStart(uint8_t sockIndex);
void UrcUnsecuredS0ClosedCb(uint8_t *lineBuff, uint16_t len);
void UrcUnsecuredS1ClosedCb(uint8_t *lineBuff, uint16_t len);
//void UrcUnsecuredS2ClosedCb(uint8_t *lineBuff, uint16_t len);
//void UrcUnsecuredS3ClosedCb(uint8_t *lineBuff, uint16_t len);
//void UrcUnsecuredS4ClosedCb(uint8_t *lineBuff, uint16_t len);

void UrcSecuredSocketCb(uint8_t *lineBuff, uint16_t len);

uint8_t GsmSocketIsOpened(uint8_t socketId);
uint8_t GsmGetSockUrcStatus(void);
void GsmCloseHttpSocket(void);
void GsmClearSockUrcStatus(void);

void GsmReadHttpSocket(void);
uint8_t GsmIsReadSockUrcRcvd(void);
void GsmClearReadSockUrc(void);
void GsmSocketSetState(gsmSockState_et state);
void  ResetAckReceivedTick(void);
uint16_t NtripSocketRead(uint8_t sockId, uint8_t *readBuff, uint16_t nreadLen);
void GsmCloseNtripSocket(void);
uint8_t GsmGetNtripSockUrcStatus(void);
void GsmClearNtripSockUrcStatus(void);
uint8_t GsmNtripSockCloseFlagStatus(void);
void GsmResetNtripSockCloseFlag(void);
#endif