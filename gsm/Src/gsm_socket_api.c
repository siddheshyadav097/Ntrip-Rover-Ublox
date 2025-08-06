#include "gsm_socket_api.h"
#include "gsm_statemachine.h"
#include "gsm_gprs_api.h"
#include "packet_api.h"
#include "mem_packet_api.h"
#include "memoryport.h"

#include <stdlib.h>
#include <string.h>

uint8_t gsmSocketIndex = 0;
static uint8_t gsmSocketUsed = 0;
static gsmCmdState_et gsmSockAtCmdState = AT_CMD_SEND;
gsmSocket_st gsmSocket[GSM_SOCK_MAX_AVAILABLE];
// used to prepare at commands
static char gsmSockTmpBuff[250];
// used in ssl urc callback data processing;
uint8_t gsmSockStrTmp[20];
char gsmSockReceiveDataLen[10];
uint16_t gsmSockReceiveLen = 0;
uint16_t receiveLen = 0;
uint16_t packetFrameId = 0;

uint32_t gsmSockReconnectTimeout = 0;
uint8_t  gsmGprsDeact = 0; 
uint8_t socRead10minTimeoutFlag = 0;
uint8_t connectId = 0, rcvConnectId = 0;
//if socket not connected for 10 minutes then power reset module time
uint32_t gsmPowerResetOnNoConnTick = 0;
uint32_t gsmNoAckReceivedFromServer = 0;
uint8_t readPDPContext = 0;
uint8_t readModemrole = 1;
uint8_t gsmSocAlreadyConnect = 0;

uint32_t sockDataCheckTo = 0;
uint32_t socUnknownStateTick = 0;

int basertcmsocket_id = 0;
long double basertcmdata_length = 0;

//unitInfo_st* unitidInfoPtr;
serverConfig_st* servConfigPtr;
//httpSendHandler_et httpSendState = WAIT_FOR_PACKET;
//uint8_t tempPackBuff[PAC_BUFSIZE];
//uint8_t packBuff[PAC_BUFSIZE];
//char httpPacketBuff[HTTP_PAC_BUFFSIZE];
uint16_t packetLen = 0, httpPacketLen = 0, headerLength = 0, bulkPacHeaderLen = 0;
uint16_t count = 0;
uint32_t bulkPacketLen = 0;
uint32_t startReadIndex = 0, endReadIndex = 0;

//socketResponse_et packACKState = SOCK_RESP_CHECK_START;
uint32_t pacSendingInProgressTick = 0;
uint32_t packAckReceiveTick = 0;
uint16_t maxPacketstoUpload = 0;
uint8_t httpReqCount = 0, failCount = 0, pacStoredFlag = 0;
uint8_t gsmPacAckTimeoutFlag = 0;

//uint8_t httpHeader[250] = {0};
//uint8_t host[100] = {0}, path[100] = {0};

extern packetSendState_et packeSendState;
extern packetSendType_et packetSendType;
extern uint32_t packetSendTick;
extern uint32_t getACKtimout;
extern uint32_t deleteCount;
extern uint8_t gprsActivated;
extern uint8_t gprsDbgRespSent;
extern uint8_t packetSentSuccess;

uint8_t flagCloseGsmSocket = 0;
uint8_t flagCloseGsmNtripSocket=0;
uint8_t flagGsmGprsDeactivate = 0;
uint8_t GsmSockCloseURC =0;
uint8_t GsmNtripSockCloseURC = 0;
uint8_t GsmReadSockURC = 0;


void GsmSocketOpenHandler(void);
void GsmSocketWriteHandler(void);
void GsmSocketReadHandler(void);
void GsmSocketCloseHandler(void);

const char* gsmSecuredSocCmdSet[GSM_SOCK_NUM_CMDS] = {       
            "AT+QSSLOPEN=",    //0
            "AT+QSSLSEND=",    //1
            "AT+QSSLRECV=",    //2
            "AT+QSSLCLOSE=",   //3
            "AT+QSSLSTATE\r"   //4
};

const char* gsmUnsecuredSocCmdSet[GSM_SOCK_NUM_CMDS] = {	       
            "AT+QIOPEN=1,",    //0          AT+QIOPEN=1,0,"TCP","vtv1.qdvts.com",1889,0,1   //access mode - Direct Push mode
            "AT+QISEND=",      //1
            "AT+QIRD=",        //2
            "AT+QICLOSE="      //3
//            "AT+QISTATE\r"   //4
};


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
uint8_t GsmSocketGet(uint8_t* hostNamePtr ,uint8_t* pathPtr, uint16_t portNum,uint8_t* serverReqType, uint8_t *readBuf, 
                     uint16_t readBuffLen,gsmSockSecureType_et security)
{
    if(gsmSocketUsed >= GSM_SOCK_MAX_AVAILABLE)
    {
        return GSM_SOCK_NOT_AVAILABLE;
    }
    
    // initialise all the variables of the struct
    gsmSocket[gsmSocketUsed].sockId         = gsmSocketUsed;
    gsmSocket[gsmSocketUsed].flagOpenCmd    = 0;
    gsmSocket[gsmSocketUsed].state          = GSM_SOCK_CHECK_OPEN;
    gsmSocket[gsmSocketUsed].hostAddress    = hostNamePtr;
    gsmSocket[gsmSocketUsed].hostPath       = pathPtr;
    gsmSocket[gsmSocketUsed].serverReqType  = serverReqType;
    gsmSocket[gsmSocketUsed].portNum        = portNum;
	  gsmSocket[gsmSocketUsed].openState      = GSM_SOCK_OPEN_WAIT_FOR_START;
    gsmSocket[gsmSocketUsed].writeState     = GSM_SOCK_WRITE_WAIT_FOR_START;
    gsmSocket[gsmSocketUsed].writeBuf       = NULL;
    gsmSocket[gsmSocketUsed].writeLen       = 0;
    gsmSocket[gsmSocketUsed].readState      = GSM_SOCK_READ_WAIT_FOR_START;
	  gsmSocket[gsmSocketUsed].closeState     = GSM_SOCK_CLOSE_WAIT_FOR_START;
    RingBufferInit(&gsmSocket[gsmSocketUsed].readBuf,readBuf,readBuffLen);
    gsmSocket[gsmSocketUsed].security       = security;
//  gsmSocket[gsmSocketUsed].reconnectTimeout = GetStartTime();
    gsmSocket[gsmSocketUsed].reconnectCnt     =  5;
    
    gsmSockReconnectTimeout = GetStartTime();
    
//    LOG_DBG(CH_SOCK,"Socket num init %d",gsmSocketUsed);
    
    // increment the socket used
    gsmSocketUsed++;
    
    
    // the socket id is the index of the socket structure 
    return (gsmSocketUsed-1);
}


void GsmSocketStateReset(void)
{
  uint8_t sockId = 0;
  
  for(sockId = 0; sockId <= (gsmSocketUsed - 1) ; sockId++ )
  {
      gsmSocket[sockId].state           = GSM_SOCK_CHECK_OPEN;
      gsmSocket[sockId].openState       = GSM_SOCK_OPEN_WAIT_FOR_START;
      gsmSocket[sockId].writeState      = GSM_SOCK_WRITE_WAIT_FOR_START;
      gsmSocket[sockId].writeBuf        = NULL;
      gsmSocket[sockId].writeLen        = 0;
      gsmSocket[sockId].readState       = GSM_SOCK_READ_WAIT_FOR_START;
      gsmSocket[sockId].closeState      = GSM_SOCK_CLOSE_WAIT_FOR_START;
//    gsmSocket[sockId].reconnectTimeout = GetStartTime();
      gsmSocket[sockId].reconnectCnt     =  5;
  }
//  sockDataCheckTo = GetStartTime();
}


/**
 *  @brief  :GsmGetCircularSocIndex()- get the socket index circulary to scan in a  GSmSocketStateHandler()
 *  @param  :[in] socId - Current socket id
 *  @return : void         
 */
void GsmSocketIndexNext()
{
   if(gsmSocketIndex < (gsmSocketUsed -1))
   {
      gsmSocketIndex++;                       //main loop socket index
   }
   else
   {
     gsmSocketIndex = 0;
   }
}

void GsmReset10MinTick(void)
{
    gsmPowerResetOnNoConnTick = GetStartTime();
}

void GsmUpdateIpPort(uint8_t sockId, uint8_t* hostNamePtr ,uint8_t* pathPtr, uint16_t portNum,uint8_t* serverReqType)
{
    
    gsmSocket[sockId].hostAddress    = hostNamePtr;
    gsmSocket[sockId].hostPath       = pathPtr;
    gsmSocket[sockId].portNum        = portNum;
    gsmSocket[sockId].serverReqType  = serverReqType;
	   GsmSocketSetState(GSM_SOCK_CLOSE);

}

void GsmClearReadSockUrc(void)
{
  GsmReadSockURC = 0;
}

uint8_t GsmIsReadSockUrcRcvd(void)
{
   return GsmReadSockURC;
}

void GsmReadHttpSocket(void)
{
  gsmSocket[0].readState = GSM_SOCK_READ_START;
}

/**
 *  @brief  :GsmGetCircularSocIndex()- get the socket index circulary to scan in a  GSmSocketStateHandler()
 *  @param  :[in] socId - Current socket id
 *  @return : void         
 */
//void GsmSocketIndexNext(void)
//{
//   if(gsmSocketIndex < (gsmSocketUsed -1))
//   {
//      gsmSocketIndex++;                       //main loop socket index
//   }
//   else
//   {
//     gsmSocketIndex = 0;
//   }
//}

/**
 *  @brief  : Check if any of the socket is unsecure socket.
 *  @param  :[in] socketId
 *  @return : no ret val
 */
uint8_t GsmSocketIsAnyUnsecure(void)
{ 
   uint8_t i;
   for(i = 0; i < gsmSocketUsed; i++)
   {
        if(gsmSocket[i].security == GSM_SOCK_UNSECURE)
        {
            break;
        }  
   }
   return gsmSocket[i].security;
}

/**
 *  @brief  : Check if any of the socket is secure socket.
 *  @param  :[in] socketId
 *  @return : no ret val
 */
uint8_t GsmSocketIsAnySecure(void)
{
    uint8_t i;
    for(i = 0; i < gsmSocketUsed; i++)
    {
        if(gsmSocket[i].security == GSM_SOCK_SECURE)
        {
            break;
        }  
    }
   return gsmSocket[i].security;
}

/**
 *  @brief  : socket open command is initiated
 *  @param  :[in] socketId
 *  @return : no ret val
 */
void GsmSocketOpen(uint8_t socketId)
{
	 gsmSocket[socketId].flagOpenCmd = 1;
    //gsmSocket[socketId].reconnectTimeout = GetStartTime();
}

/**
 *  @brief  : socket close command is initiated will happen only when the socket is open
 *  @param  :[in] socketId
 *  @return : no ret val
 */
void GsmSocketClose(uint8_t socketId)
{
	gsmSocket[socketId].flagOpenCmd = 0;
}





uint8_t GsmSockCloseFlagStatus(void)
{
  return flagCloseGsmSocket;
}

uint8_t GsmNtripSockCloseFlagStatus(void)
{
  return flagCloseGsmNtripSocket;
}





void GsmResetSockCloseFlag(void)
{
   flagCloseGsmSocket = 0;
}

void GsmResetNtripSockCloseFlag(void)
{
   flagCloseGsmNtripSocket = 0;
}

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
gsmSockState_et GsmSocketGetState(uint8_t socketId)
{
	return gsmSocket[socketId].state;
}

uint8_t GsmSocketIsOpened(uint8_t socketId)
{
	if(gsmSocket[socketId].state == GSM_SOCK_OPENED || gsmSocket[socketId].state == GSM_SOCK_WRITE ||
	gsmSocket[socketId].state == GSM_SOCK_READ)
	{
		return 1;
	}
	
	return 0;
}

/**
 *  @brief  : Socket Write Start Command will check whether socket is opened and 
 *  @param  :[in] sockId: socket number should be less than the max socket number
 *  @return : will return socket write failure if 
 *  
 */
gsmSockWriteRet_et GsmSocketWrite(uint8_t sockId, uint8_t *buff, uint16_t writeLen)
{
	if(gsmSocket[sockId].state == GSM_SOCK_OPENED)
	{
		// change the state of socket write
		gsmSocket[sockId].writeBuf = buff;
		gsmSocket[sockId].writeLen = writeLen;
		gsmSocket[sockId].writeState = GSM_SOCK_WRITE_START;
		return GSM_SOCK_WRITE_STARTED;
	}
	else
	{
		return GSM_SOCK_WRITE_BUSY;
	}
}

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
gsmSockWriteState_et GsmSockGetWriteState(uint8_t sockId)
{
	return gsmSocket[sockId].writeState;
}

uint16_t GsmSocketGetAvlData(uint8_t sockId)
{
    return RingBufferGetAvlLen(&gsmSocket[sockId].readBuf);
} 

void GsmSocketReadFixedLenCb(uint8_t receiveData)
{
	RingBufferFill(&gsmSocket[0].readBuf,receiveData);
//    if(GsmSocketGetAvlData(gsmSocket[gsmSocketIndex].sockId) >= 9)
//    {
//      getSocAck = GsmSocketRead(gsmSocket[gsmSocketIndex].sockId,gsmSockStrTmp,9, SOCKET_ACK_DATA);
//      if(getSocAck)
//      {
//        getSocAck = 0;
//      }
//    }
}

void NtripGsmSocketReadFixedLenCb(uint8_t receiveData)
{
	RingBufferFill(&gsmSocket[1].readBuf,receiveData);
//    if(GsmSocketGetAvlData(gsmSocket[gsmSocketIndex].sockId) >= 9)
//    {
//      getSocAck = GsmSocketRead(gsmSocket[gsmSocketIndex].sockId,gsmSockStrTmp,9, SOCKET_ACK_DATA);
//      if(getSocAck)
//      {
//        getSocAck = 0;
//      }
//    }
}

/**
 *  @brief  : will read the data received from the socket upto readLen 
 *  @param  :[in] sockId  socket Id
 *  @param  :[in] readBuff received data is copied in this buffer
 *  @param  :[in] readLen  length of data to be read
 *  @return : will return the actual read data it can be smaller than the read Len
 *              returns 0 if the socket is not connected or there is not data available
 */
uint16_t GsmSocketRead(uint8_t sockId, uint8_t *readBuff, uint16_t readLen)
{
  	 uint16_t len = 0;
		while(RingBufferDrain(&gsmSocket[sockId].readBuf,(uint8_t*)readBuff))
		{
			readBuff++;
			len++;
			if(len >= readLen)
			{
				break;
			}
		}
		return len;
	
//    uint16_t len = 0;
//    uint8_t recvData[20];
//    
//	while(RingBufferDrain(&gsmSocket[sockId].readBuf,readBuff))
//	{
//        recvData[len] = *readBuff;
//        readBuff++;
//        len++;
//        
//		if(len >= readLen)
//		{
//          if(GSMStrPrefixMatch((const char*)recvData, (const char*)expectedReply))
//          {
//            return 1;
//          }
//          else
//            return 0;
//		}
//	}
//	return 0;
}

uint16_t NtripSocketRead(uint8_t sockId, uint8_t *readBuff, uint16_t nreadLen)
{
  	 uint16_t ntriplen = 0;
		while(RingBufferDrain(&gsmSocket[sockId].readBuf,(uint8_t*)readBuff))
		{
			readBuff++;
			ntriplen++;
			if(ntriplen >= nreadLen)
			{
				break;
			}
		}
		return ntriplen;
	
//    uint16_t len = 0;
//    uint8_t recvData[20];
//    
//	while(RingBufferDrain(&gsmSocket[sockId].readBuf,readBuff))
//	{
//        recvData[len] = *readBuff;
//        readBuff++;
//        len++;
//        
//		if(len >= readLen)
//		{
//          if(GSMStrPrefixMatch((const char*)recvData, (const char*)expectedReply))
//          {
//            return 1;
//          }
//          else
//            return 0;
//		}
//	}
//	return 0;
}

uint8_t GsmSocketReadSingleChar(uint8_t sockId, uint8_t *readBuff, uint16_t readLen)
{
    uint16_t len = 0;
    
	while(RingBufferDrain(&gsmSocket[sockId].readBuf,readBuff))
	{
        len++;
        if(len >= readLen)
		{
          return 1;
        }
        else
          return 0;
    }
	return 0;
}

void GsmSocketSetState(gsmSockState_et state)
{
	if(gsmSocket[gsmSocketIndex].state == state)
	{
		return;
	}
	gsmSocket[gsmSocketIndex].state = state;
	
	switch(gsmSocket[gsmSocketIndex].state)
	{
		case GSM_SOCK_NOT_READY:
		
		break;
        
        case GSM_SOCK_CHECK_OPEN:
            GsmSetState(GSM_MODEM_IDLE);
        break;
		
		case GSM_SOCK_OPEN:
			gsmSocket[gsmSocketIndex].openState = GSM_SOCK_OPEN_IN_PROG;
			gsmSockAtCmdState = AT_CMD_SEND;
			GsmSetState(GSM_SOCKET_PROCESSING);
		break;

		case GSM_SOCK_OPENED:
            if(socRead10minTimeoutFlag)
            {
                GsmSetState(GSM_SW_POWER_RESET);
                socRead10minTimeoutFlag = 0;
            }
            else
            {
                GsmSetState(GSM_MODEM_IDLE);
            }
        break;
		
		case GSM_SOCK_WRITE:
			gsmSocket[gsmSocketIndex].writeState = GSM_SOCK_WRITE_IN_PROG;
			gsmSockAtCmdState = AT_CMD_SEND;
			GsmSetState(GSM_SOCKET_PROCESSING);
		break;
		
		case GSM_SOCK_READ:
			gsmSocket[gsmSocketIndex].readState = GSM_SOCK_READ_IN_PROG;
			gsmSockAtCmdState = AT_CMD_SEND;
			GsmSetState(GSM_SOCKET_PROCESSING);
		break;
		
		case GSM_SOCK_CLOSE:
			gsmSocket[gsmSocketIndex].closeState = GSM_SOCK_CLOSE_IN_PROG;
			gsmSockAtCmdState = AT_CMD_SEND;
			GsmSetState(GSM_SOCKET_PROCESSING);
		break;
	}
}

/**
 *  @brief : check whether gprs is activated then change all the socket state to socket open if it is not
 *  		ready
 *  @return none
 *  @details Details
 */
void GsmSocketCheckGprsAndUpdateState(void)
{
		uint8_t i;
	for(i = 0; i < gsmSocketUsed; i++)
	{
		if(GsmGprsIsActive())
		{
			if(gsmSocket[i].state == GSM_SOCK_NOT_READY)
			{
				gsmSocket[i].state = GSM_SOCK_CHECK_OPEN;
			}
		}
		else
		{
			if(gsmSocket[i].state == GSM_SOCK_OPENED)
			{
				gsmSocket[i].state = GSM_SOCK_NOT_READY;
			}
		}
	}
}

void  ResetAckReceivedTick(void)
{
  gsmNoAckReceivedFromServer = GetStartTime();
}

void GsmSocketHandler(void)
{
	// check whether gprs is activated 
    GsmSocketCheckGprsAndUpdateState();
	// handler to check whether socket to be opened
    switch(gsmSocket[gsmSocketIndex].state)
	{
	  case GSM_SOCK_NOT_READY:
			break;
		
		case GSM_SOCK_CHECK_OPEN:
         // check whether a open command is set for the socket then change the socket state to open socket
            if(gsmSocket[gsmSocketIndex].flagOpenCmd == 1)
            {
                if(GsmStateIsIdle())
                {
                    gsmSocket[gsmSocketIndex].reconnectCnt++; //increament the sock reconnect retry count
                    
									  
                    if(TimeSpent(gsmPowerResetOnNoConnTick,GSM_POWER_RESET_ON_CONN_MS))//no data activity for 20 minutes
                    {
                        LOG_ERR(CH_SOCK,"FAILED to open Socket for 20 minutes,Power reset Gsm Modem");
                        gsmPowerResetOnNoConnTick = GetStartTime();
                        GsmSetState(GSM_HW_POWER_RESET);
                    }
										if(TimeSpent(gsmNoAckReceivedFromServer,MAX_GSM_SOCK_CONNECT_TIMEOUT)) //for 3 minutes no ack is been received the hard reset the gsm mode
										{
										     LOG_ERR(CH_SOCK,"FAILED to receive ack for 3 minutes,Power reset Gsm Modem");
                        gsmNoAckReceivedFromServer = GetStartTime();
                        GsmSetState(GSM_HW_POWER_RESET);
										
										}
                    else if(TimeSpent(gsmSockReconnectTimeout,MAX_GSM_SOCK_CONNECT_TIMEOUT) && \
                      ((gsmSocket[gsmSocketIndex].reconnectCnt >= GSM_SOCK_CHECK_NUM_RETRY)))  //max 3 minutes timeout
                    {
                        if(!flagGsmGprsDeactivate)
                        {
                            flagGsmGprsDeactivate = 1;
                        }
                        else
                        {
                            flagGsmGprsDeactivate = 0;
                            gsmGprsDeact = 0;
                            gsmSocket[gsmSocketIndex].reconnectCnt = 0;
                            gsmSockReconnectTimeout = GetStartTime();
                            LOG_ERR(CH_SOCK,"FAILED to open Socket for 3 minutes, Deactivate GPRS");
                            GsmSetState(GPRS_DEACTIVATE);      //if unable to open the any of the socket for 5 minutes then deactivate the gprs
                        }
                    }  
                    else
                    {
                        GsmSocketSetState(GSM_SOCK_OPEN);   //If 3 minutes time is not spent just try to reopen the socket
                    }
                }
                else
                {
                        GsmSocketIndexNext();
                }
            }
            else
            {
                    GsmSocketIndexNext();
            }
         break;
		
		case GSM_SOCK_OPEN:
			GsmSocketOpenHandler();
			if(gsmSocket[gsmSocketIndex].openState == GSM_SOCK_OPEN_SUCCESS)
			{
				GsmSocketSetState(GSM_SOCK_OPENED);
                gsmSocket[gsmSocketIndex].reconnectCnt = 0;
                gsmGprsDeact = 0;
                gsmSockReconnectTimeout = GetStartTime(); 
                gsmPowerResetOnNoConnTick = GetStartTime(); 
			}
			else if(gsmSocket[gsmSocketIndex].openState == GSM_SOCK_OPEN_FAIL)
			{
				GsmSocketSetState(GSM_SOCK_CLOSE);
                if(!gsmGprsDeact)
                {
                   gsmGprsDeact = 1;
                   gsmSockReconnectTimeout = GetStartTime();    //5 minutes timeout to check the socket reconnection
                }
			}
		break;

		case GSM_SOCK_OPENED:
			if(GsmStateIsIdle())
			{
			
			  if(gsmSocket[gsmSocketIndex].closeState == GSM_SOCK_CLOSE_START)
				{
					GsmSocketSetState(GSM_SOCK_CLOSE);
				}
			
				else if(gsmSocket[gsmSocketIndex].writeState == GSM_SOCK_WRITE_START)
				{
					GsmSocketSetState(GSM_SOCK_WRITE);
				}
        else if(gsmSocket[gsmSocketIndex].readState == GSM_SOCK_READ_START)
				{
					GsmSocketSetState(GSM_SOCK_READ);
				}
//                else if(gsmSocket[gsmSocketIndex].closeState == GSM_SOCK_CLOSE_START)
//				{
//					GsmSocketSetState(GSM_SOCK_CLOSE);
//				}
				else
				{
                    GsmSocketIndexNext();
				}
			}
		break;
		
		case GSM_SOCK_WRITE:
			GsmSocketWriteHandler();
			if(gsmSocket[gsmSocketIndex].writeState == GSM_SOCK_WRITE_SUCCESS)
			{
				GsmSocketSetState(GSM_SOCK_OPENED);
				
			}
			else if(gsmSocket[gsmSocketIndex].writeState == GSM_SOCK_WRITE_FAIL)
			{
				// close the gsm socket
				GsmSocketSetState(GSM_SOCK_CLOSE);
			}
		break;
		
		case GSM_SOCK_READ:
			GsmSocketReadHandler();
			if(gsmSocket[gsmSocketIndex].readState == GSM_SOCK_READ_SUCCESS)
			{
				GsmSocketSetState(GSM_SOCK_OPENED);
			}
			else if(gsmSocket[gsmSocketIndex].readState == GSM_SOCK_READ_FAIL)
			{
                GsmSocketSetState(GSM_SOCK_OPENED);
				
			}
		break;
		
		case GSM_SOCK_CLOSE:
			// close the socket and again open it
			GsmSocketCloseHandler();
			if(gsmSocket[gsmSocketIndex].closeState == GSM_SOCK_CLOSE_SUCCESS)
			{
                //flagCloseGsmSocket = 1; 
                //GsmSocketClose(0);  //in http user will set the flag to open  the socket
				GsmSocketSetState(GSM_SOCK_CHECK_OPEN);
			}
			else if(gsmSocket[gsmSocketIndex].closeState == GSM_SOCK_CLOSE_FAIL)
			{
               // flagCloseGsmSocket = 1;
               // GsmSocketClose(0); //in http user will set the flag to open  the socket
				GsmSocketSetState(GSM_SOCK_CHECK_OPEN);
			}
		break;
	}
}


/******** GSM SOCKET OPEN HANDLER AND ITS CB STARTS HERE ****/

gsmRespCb_et  GsmSocketOpenCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len)
{
    gsmRespCb_et gsmRespRetVal = GSM_RESP_CB_WAIT;
    char *p1 = NULL;
    char *p2 = NULL;
    uint8_t errCodeBuff[5];
    uint16_t errCode = 566;
   
    switch(gsmResp)
    {
        case RESP_OK:
		case RESP_OTHER:
		case RESP_IGNORE:
            gsmRespRetVal = GSM_RESP_CB_WAIT; 
			LOG_DBG(CH_SOCK,"Open CB Sock num : %d,Resp code : %d",gsmSocketIndex,gsmResp);
            break;
        
        case RESP_SOCKET_OPEN:      //+QIOPEN: 
            rcvConnectId = atoi((const char *)(lineBuff + 9));
            p1 = strstr((char*)lineBuff,",");
            p2 = strchr(p1,'\0');
            memset(errCodeBuff,0,sizeof(errCodeBuff));
            strncpy((char*)errCodeBuff, p1+1, p2-p1-1);
            errCode = atoi((const char*)errCodeBuff);
            //if((rcvConnectId == connectId) && (errCode == 0))
						if((rcvConnectId == gsmSocket[gsmSocketIndex].sockId) && (errCode == 0))
            {
                gsmRespRetVal = GSM_RESP_CB_OK_COMPLETE;  
                gsmSockAtCmdState = AT_CMD_SUCCESS;
                LOG_INFO(CH_SOCK,"Connected On Socket: IP/URL - %s,Port - %d Correct_ConnectId,rcvConnectId=%d, connectId=%d",gsmSocket[gsmSocketIndex].hostAddress,gsmSocket[gsmSocketIndex].portNum,rcvConnectId,connectId); 
            }
            else
            {
                gsmRespRetVal = GSM_RESP_CB_ERROR_COMPLETE; 
                gsmSockAtCmdState = AT_CMD_FAILURE;	
                LOG_ERR(CH_SOCK,"Fail to connect socket, rcvConnectId=%d, connectId=%d, errCode = %d",rcvConnectId,connectId,errCode);	
            }
            break;
          
        case RESP_QDNET_SOC_CONNECT_OK:
            gsmRespRetVal = GSM_RESP_CB_OK_COMPLETE; 
            gsmSockAtCmdState = AT_CMD_SUCCESS;
            LOG_INFO(CH_SOCK,"RESP_QDNET_SOC_CONNECT_OK: IP/URL - %s,Port - %d",gsmSocket[gsmSocketIndex].hostAddress,gsmSocket[gsmSocketIndex].portNum);
            break;
            
        case RESP_ALREADY_CONNECT:  
            gsmRespRetVal = GSM_RESP_CB_OK_COMPLETE;  
			gsmSockAtCmdState = AT_CMD_SUCCESS;
            gsmSocAlreadyConnect = 1;
            LOG_INFO(CH_SOCK,"ALREADY_CONNECT On Socket: IP/URL - %s,Port - %d",gsmSocket[gsmSocketIndex].hostAddress,gsmSocket[gsmSocketIndex].portNum); 
            break;
        
        case RESP_ERROR:
        case RESP_CONECT_FAIL:
		case RESP_CME_ERROR:
		case RESP_TIMEOUT:
			gsmRespRetVal = GSM_RESP_CB_ERROR_COMPLETE; 
			LOG_ERR(CH_SOCK,"Open CB Sock num : %d,Resp code : %d",gsmSocketIndex,gsmResp);	
			gsmSockAtCmdState = AT_CMD_FAILURE;			
            break;
        
        default:
        break;
    }
    return gsmRespRetVal;
}

void GsmSocketOpenHandler(void)
{
    switch(gsmSockAtCmdState)
    {
      case AT_CMD_SEND:
        if(GsmIsAtIdle())
        {
          // check the type of socket secure or unsecure
          if(gsmSocket[gsmSocketIndex].security == GSM_SOCK_SECURE)
          {
//                snprintf((char *)gsmSockTmpBuff,sizeof(gsmSockTmpBuff),"%s%d,%d,\"%s\",%d,%d,%d\r",gsmSecuredSocCmdSet[GSM_SOC_OPEN],gsmSocket[gsmSocketIndex].sockId,PDP_CONTEXT_ID,\
//                gsmSocket[gsmSocketIndex].address,gsmSocket[gsmSocketIndex].portNum,GSM_CONNECT_MODE,SSL_SOC_OPEN_TO);
//                GsmSendAtCmd(gsmSockTmpBuff,strlen((const char *)gsmSockTmpBuff),GSM_SOCK_OPEN_TIMEOUT,GsmSocketSslOpenCb);
          }
          else if(gsmSocketIndex==0)
          { //AT+QIOPEN=<contextID>,<connectID>,<service_type>,<IP_address>/<domain_name>,<remote_port>[,<local_port>[,<access_mode>]]buffer access mode
              snprintf((char *)gsmSockTmpBuff,sizeof(gsmSockTmpBuff),"%s%d,\"%s\",\"%s\",%d,0,0\r",gsmUnsecuredSocCmdSet[GSM_SOC_OPEN],gsmSocket[gsmSocketIndex].sockId,
                       SERVICE_TYPE,gsmSocket[gsmSocketIndex].hostAddress,gsmSocket[gsmSocketIndex].portNum);
              GsmSendAtCmd(gsmSockTmpBuff,strlen((const char *)gsmSockTmpBuff),GSM_SOCK_OPEN_TIMEOUT,GsmSocketOpenCb);
          }
					else if(gsmSocketIndex==1)
          { //AT+QIOPEN=<contextID>,<connectID>,<service_type>,<IP_address>/<domain_name>,<remote_port>[,<local_port>[,<access_mode>]]buffer access mode
              snprintf((char *)gsmSockTmpBuff,sizeof(gsmSockTmpBuff),"%s%d,\"%s\",\"%s\",%d,0,1\r",gsmUnsecuredSocCmdSet[GSM_SOC_OPEN],gsmSocket[gsmSocketIndex].sockId,
                       SERVICE_TYPE,gsmSocket[gsmSocketIndex].hostAddress,gsmSocket[gsmSocketIndex].portNum);
              GsmSendAtCmd(gsmSockTmpBuff,strlen((const char *)gsmSockTmpBuff),GSM_SOCK_OPEN_TIMEOUT,GsmSocketOpenCb);
							
							resetntripheaderflag();
          }
          gsmSockAtCmdState = AT_WAIT_REPLY;
         }
      break;
      
      case AT_WAIT_REPLY:
      break;
      
      case AT_CMD_SUCCESS:
              gsmSocket[gsmSocketIndex].openState = GSM_SOCK_OPEN_SUCCESS;
              //sockDataCheckTo = GetStartTime();
              LOG_DBG(CH_SOCK,"Open success sock no. : %d",gsmSocketIndex);
      break;
      
      case AT_CMD_FAILURE:
              gsmSocket[gsmSocketIndex].openState = GSM_SOCK_OPEN_FAIL;
              LOG_ERR(CH_SOCK,"Open failed sock no. : %d",gsmSocketIndex);
      break;
      
      case AT_CMD_RETRY_WAIT:
      break;
      
      default:
        if(TimeSpent(socUnknownStateTick, 60000))
        {
            LOG_DBGS(CH_GSM, "GsmSocketOpenHandler - default State");
            socUnknownStateTick = GetStartTime();
        }
        break;
    }
}

/******** GSM SOCKET OPEN HANDLER AND ITS CB ENDS HERE ***----------*/


/******** GSM SOCKET WRITE HANDLER AND ITS CB STARTS HERE ****/
gsmRespCb_et  GsmSocketWriteCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len)
{
	gsmRespCb_et gsmRespRetVal = GSM_RESP_CB_WAIT;;
   
    switch(gsmResp)
    {
        case RESP_OK:
		case RESP_ERROR:
		case RESP_SEND_FAIL:
		case RESP_CME_ERROR:
		case RESP_TIMEOUT:
            gsmRespRetVal = GSM_RESP_CB_ERROR_COMPLETE;  //we receive only SEND OK, SEND FAIL AND ERROR
			gsmSockAtCmdState = AT_CMD_FAILURE;
			LOG_ERR(CH_SOCK,"Write CB Sock num 1 : %d,Resp code : %d",gsmSocketIndex,gsmResp);
        break;
        
		case RESP_IGNORE:
		case RESP_OTHER:
			gsmRespRetVal = GSM_RESP_CB_WAIT;
			LOG_DBG(CH_SOCK,"Write CB Sock num 2 : %d,Resp code : %d",gsmSocketIndex,gsmResp);
        break;
		
		case RESP_PROMPT_CHAR:
			GsmUartSendData((char *)gsmSocket[gsmSocketIndex].writeBuf,gsmSocket[gsmSocketIndex].writeLen);
			gsmRespRetVal = GSM_RESP_CB_WAIT; 
        break;
        
        case RESP_SEND_OK:
			gsmRespRetVal = GSM_RESP_CB_OK_COMPLETE;                              //success in sending data
			gsmSockAtCmdState = AT_CMD_SUCCESS;
            LOG_DBG(CH_SOCK,"Write CB Sock num 3 : %d,Resp code : %d",gsmSocketIndex,gsmResp);
        break;
        
        default:
        break;
    }
    return gsmRespRetVal;
}

void GsmSocketWriteHandler(void)
{
	switch(gsmSockAtCmdState)
	{
		case AT_CMD_SEND:
          if(GsmIsAtIdle())
          {
                // check the type of socket secure or unsecure
				if(gsmSocket[gsmSocketIndex].security == GSM_SOCK_SECURE)
				{
					snprintf((char *)gsmSockTmpBuff,sizeof(gsmSockTmpBuff),"%s%d,%d\r",
					gsmSecuredSocCmdSet[GSM_SOC_SEND_DATA],gsmSocket[gsmSocketIndex].sockId,gsmSocket[gsmSocketIndex].writeLen);
				}
				else
				{
					snprintf((char *)gsmSockTmpBuff,sizeof(gsmSockTmpBuff), "%s%d,%d\r" ,gsmUnsecuredSocCmdSet[GSM_SOC_SEND_DATA],gsmSocket[gsmSocketIndex].sockId,
                    gsmSocket[gsmSocketIndex].writeLen);
				}
                GsmSendAtCmd(gsmSockTmpBuff,strlen((const char *)gsmSockTmpBuff),GSM_SOCK_SEND_DATA_TIMEOUT,GsmSocketWriteCb);
                gsmSockAtCmdState = AT_WAIT_REPLY;
         }
       break;
		
       case AT_WAIT_REPLY:
            gsmSocket[gsmSocketIndex].writeState = GSM_SOCK_WRITE_IN_PROG;
            //LOG_DBG(CH_SOCK,"WRITING PACKET IN PROGRESS",);
       break;
		
       case AT_CMD_SUCCESS:
			gsmSocket[gsmSocketIndex].writeState = GSM_SOCK_WRITE_SUCCESS;
			LOG_DBG(CH_SOCK,"WRITE success sock no. : %d",gsmSocketIndex);
       break;
		
       case AT_CMD_FAILURE:
			gsmSocket[gsmSocketIndex].writeState = GSM_SOCK_WRITE_FAIL;
			LOG_ERR(CH_SOCK,"WRITE failed sock no. : %d",gsmSocketIndex);
       break;
		
       case AT_CMD_RETRY_WAIT:
       break;
        
       default:
         if(TimeSpent(socUnknownStateTick, 60000))
         {
            LOG_DBGS(CH_GSM, "GsmSocketWriteHandler - default State");
            socUnknownStateTick = GetStartTime();
         }
       break;
	}
}

/*--------- GSM SOCKET WRITE HANDLER AND ITS CB ENDS HERE -------------------*/ 

/******** GSM SOCKET READ HANDLER AND ITS CB STARTS HERE ****/
 
/**
 *  @brief  : GSMGetServerDetailsAndDataLen() will parse the response of the data received cmd.
 *  @param  :[in] socketId  - socket Id
 *  @return : setFnState of type gsmCmdState_et
 *  e.g for Secured/Unsecured socket the cmd and reponse is as follwos:
 *   AT+QSSLRECV=0,0,1500/AT+QIRD=0,1,0,1500
 *   "If Buffer has data:
 *   +QSSLRECV/+QIRD: 169.45.2.20:8883,TCP,4
 *   OK
 *   If Buffer is Empty: only OK is received
 *   OK"
 *  Response brief:
 *  <ipaddr> IP address
 *  <port> The port of remote server
 *  <Protocol Type> TCP/UDP
 *  <actual length> The actual data length obtained by QSSLRECV
 */

uint16_t GSMGetServerDetailsAndDataLen(uint8_t *rcvBuffPtr ,uint8_t RcvDatLen)
{
   uint8_t *CheckSecuredUrc  = NULL;
   uint8_t *CheckUnsecureURC = NULL;
   uint8_t *q = NULL;                                               
   uint8_t ucTemp = 0;

   memset(gsmSockReceiveDataLen,0,sizeof(gsmSockReceiveDataLen));
   gsmSockReceiveLen = 0; 
   
   CheckUnsecureURC  = (uint8_t*)strstr((char *)rcvBuffPtr , "+QIRD: ");
   CheckSecuredUrc   = (uint8_t*)strstr((char *)rcvBuffPtr , "+QSSLRECV:");
   if((CheckUnsecureURC != NULL) || (CheckSecuredUrc != NULL))
   {
        CheckUnsecureURC = CheckUnsecureURC + 7;   
        q = (uint8_t *)strchr((char *)CheckUnsecureURC, '\0');
        for(ucTemp =0 ; CheckUnsecureURC < q ;ucTemp++, CheckUnsecureURC++)
        {
            gsmSockReceiveDataLen[ucTemp] = *CheckUnsecureURC;
        }
        gsmSockReceiveDataLen[ucTemp] = '\0';
        gsmSockReceiveLen = atoi((char *)gsmSockReceiveDataLen);    //here receive the actual rcv length
   }  
   return gsmSockReceiveLen;
}

uint16_t GetResponseLength(void)
{
    return receiveLen;
}

gsmRespCb_et  GsmSocketReadCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len)
{
	gsmRespCb_et gsmRespRetVal = GSM_RESP_CB_WAIT;
		
	switch(gsmResp)
    {
        case  RESP_OK: 
			gsmRespRetVal = GSM_RESP_CB_OK_COMPLETE;    //their is a data
			gsmSockAtCmdState = AT_CMD_SUCCESS;
        break;
        
        case RESP_ERROR:
		case RESP_CME_ERROR:
		case RESP_TIMEOUT:	
            gsmRespRetVal = GSM_RESP_CB_ERROR_COMPLETE;
			gsmSockAtCmdState = AT_CMD_FAILURE;
			LOG_ERR(CH_SOCK,"read CB Sock num : %d,Resp code : %d",gsmSocketIndex,gsmResp);
        break;
        
        case RESP_IGNORE:
		case RESP_OTHER:
            gsmRespRetVal = GSM_RESP_CB_WAIT;
        break;
        
		case RESP_RCV_SOCKET_DATA:
			receiveLen = GSMGetServerDetailsAndDataLen(lineBuff,len); 
			if(receiveLen > 0)
			{
				//GsmReadFixedLengthData(GsmSocketReadFixedLenCb,receiveLen);
				GsmReadFixedLengthDataIndexed(0, GsmSocketReadFixedLenCb, receiveLen);
      }
			gsmRespRetVal = GSM_RESP_CB_WAIT;		//GSM_RESP_CB_OK_COMPLETE;
		break;
		
        default:
		break;
    }
    return gsmRespRetVal;
}

void GsmSocketReadHandler(void)
{
	switch(gsmSockAtCmdState)
	{
		case AT_CMD_SEND:
			if(GsmIsAtIdle())
            {
                // check the type of socket secure or unsecure
				if(gsmSocket[gsmSocketIndex].security == GSM_SOCK_SECURE)
				{
					snprintf((char *)gsmSockTmpBuff,sizeof(gsmSockTmpBuff),"%s%d,%d,%d\r",
					gsmSecuredSocCmdSet[GSM_SOC_READ_DATA],\
					PDP_CONTEXT_ID,gsmSocket[gsmSocketIndex].sockId,GSM_SOCK_MAX_READ_DATA_LEN);
				}
				else
				{
                    snprintf((char *)gsmSockTmpBuff,sizeof(gsmSockTmpBuff),"%s%d,%d\r",gsmUnsecuredSocCmdSet[GSM_SOC_READ_DATA],gsmSocket[gsmSocketIndex].sockId,MAX_LEN_TO_BE_READ);
				}
                GsmSendAtCmd(gsmSockTmpBuff,strlen((const char *)gsmSockTmpBuff),GSM_SOCK_READ_DATA_TIMEOUT,GsmSocketReadCb);
                gsmSockAtCmdState = AT_WAIT_REPLY;
            }
		break;
		
		case AT_WAIT_REPLY:
		break;
		
		case AT_CMD_SUCCESS:
			gsmSocket[gsmSocketIndex].readState = GSM_SOCK_READ_SUCCESS;
			LOG_DBG(CH_SOCK,"READ success sock no. : %d",gsmSocketIndex);
		break;
		
		case AT_CMD_FAILURE:
			gsmSocket[gsmSocketIndex].readState = GSM_SOCK_READ_FAIL;
			LOG_ERR(CH_SOCK,"READ failed sock no. : %d",gsmSocketIndex);
		break;
		
		case AT_CMD_RETRY_WAIT:
		break;
        
        default:
        if(TimeSpent(socUnknownStateTick, 60000))
        {
            LOG_DBGS(CH_GSM, "GsmSocketReadHandler - default State");
            socUnknownStateTick = GetStartTime();
        }
        break;
	}
}

/******** GSM SOCKET READ HANDLER AND ITS CB ENDS HERE ***----------*/

/******** GSM SOCKET CLOSE HANDLER AND ITS CB STARTS HERE ****/

gsmRespCb_et  GsmSocketCloseCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len)
{
	gsmRespCb_et gsmRespRetVal = GSM_RESP_CB_WAIT;;
	
	switch(gsmResp)
    {
        case RESP_OK: 
		case RESP_S0_CLOSE_OK:
		case RESP_S1_CLOSE_OK:
			gsmRespRetVal = GSM_RESP_CB_OK_COMPLETE;    //their is a data
			gsmSockAtCmdState = AT_CMD_SUCCESS;
        break;
        
        case RESP_ERROR:
		case RESP_CME_ERROR:
		case RESP_TIMEOUT:		
			gsmRespRetVal = GSM_RESP_CB_ERROR_COMPLETE;
			gsmSockAtCmdState = AT_CMD_FAILURE;
			LOG_ERR(CH_SOCK,"Close CB Sock num : %d,Resp code : %d",gsmSocketIndex,gsmResp);
        break;
        
        case RESP_IGNORE:
		case RESP_OTHER:
            gsmRespRetVal = GSM_RESP_CB_WAIT;
        break;
        
		default:
		break;
    }
    return gsmRespRetVal;
}

void GsmSocketCloseHandler(void)
{
	switch(gsmSockAtCmdState)
	{
		case AT_CMD_SEND:
			if(GsmIsAtIdle())
      {
									// check the type of socket secure or unsecure
					if(gsmSocket[gsmSocketIndex].security == GSM_SOCK_SECURE)
					{
						snprintf((char *)gsmSockTmpBuff,sizeof(gsmSockTmpBuff),"%s%d\r" ,
						gsmSecuredSocCmdSet[GSM_SOC_CLOSE],gsmSocket[gsmSocketIndex].sockId);
					}
					else
					{
						snprintf((char *)gsmSockTmpBuff,sizeof(gsmSockTmpBuff), "%s%d\r",gsmUnsecuredSocCmdSet[GSM_SOC_CLOSE],gsmSocket[gsmSocketIndex].sockId);
					}
					
					if(gsmSocketIndex == 1)
					{
						resetntripheaderflag();
					}
									
					GsmSendAtCmd(gsmSockTmpBuff,strlen((const char *)gsmSockTmpBuff),GSM_SOCK_CLOSE_TIMEOUT,GsmSocketCloseCb);
					gsmSockAtCmdState = AT_WAIT_REPLY;
       }
		break;
		
		case AT_WAIT_REPLY:
		break;
		
		case AT_CMD_SUCCESS:
			gsmSocket[gsmSocketIndex].closeState = GSM_SOCK_CLOSE_SUCCESS;
			LOG_DBG(CH_SOCK,"Close success sock no. : %d",gsmSocketIndex);
		  
		break;
		
		case AT_CMD_FAILURE:
			gsmSocket[gsmSocketIndex].closeState = GSM_SOCK_CLOSE_FAIL;
			LOG_ERR(CH_SOCK,"Close failed sock no. : %d",gsmSocketIndex);
		break;
		
		case AT_CMD_RETRY_WAIT:
		break;
        
        default:
        if(TimeSpent(socUnknownStateTick, 60000))
        {
            LOG_DBGS(CH_GSM, "GsmSocketCloseHandler - default State");
            socUnknownStateTick = GetStartTime();
        }
        break;
	}
}

/******** GSM SOCKET CLOSE HANDLER AND ITS CB ENDS HERE ***----------*/

void UrcUnsecuredSocketReadCb(uint8_t *lineBuff, uint16_t len)
{
    char* p1 = NULL;
    char* p2 = NULL;
    uint8_t conId;

	p1 =(char *) (lineBuff + 8);		//	offset of +QIURC: 
    p2 = strstr(p1,",");
    if(p1 && p2)
	{
		memset(gsmSockStrTmp,0x0,sizeof(gsmSockStrTmp));
		strncpy((char *)gsmSockStrTmp, p1, (p2 - p1 - 1));
        
        if(strstr((const char *)gsmSockStrTmp,"closed") != 0)
		{
			//any of the range 0-5 SSL Socket is closed get the ssid of that socket
			memset(gsmSockStrTmp,0,sizeof(gsmSockStrTmp));
			p1 = p2;
			p2 = strchr(p1,'\0');
			if(p1 && p2)
			{
				strncpy((char *)gsmSockStrTmp, p1 + 1, p2 - p1 - 1);
				conId = atoi((const char *)gsmSockStrTmp);
				if(conId < MAX_SOC_SERVICE_INDEX)
				{
					//gsmSocket[gsmSocketIndex].closeState = GSM_SOCK_CLOSE_START;
					if(conId==0)
					{
					 GsmSockCloseURC = 1;
					}
					else if(conId==1)
					{
						GsmNtripSockCloseURC = 1;
					}
					LOG_DBG(CH_SOCK,"SSL close URC sock num : %d",conId);
////                    if(packetSendType >= PACKET_HISTORY)       //Comparing both History and Duplicate packet type
////                    {
////                        HttpSendSetState(CLOSE_HTTP_SOC_CONNECTION);
////                    }
				}
				else
				{
					LOG_ERR(CH_SOCK,"SSL close URC invalid sock num : %d",conId);
				}
			}
		}
		else if(strstr((const char *)gsmSockStrTmp,"recv") != 0)     //readSocData for its specific Connect ID
		{
			// this means any of the connect id btn 0-11 has sent some data to the modem 
			// get the connect id of that socket
			
			p1 = strstr(p2,","); //sslCtxId is skipped here to get the ssid as ctxId is 0 fixed
			if(p1)
			{
				p1++;
				conId = atoi(p1);
				if((conId < MAX_SOC_SERVICE_INDEX) && (gsmPacAckTimeoutFlag != 1))
				{
					  //gsmSocket[gsmSocketIndex].readState = GSM_SOCK_READ_START;
						if(conId==0)
						{
						 GsmReadSockURC = 1;
						 LOG_DBG(CH_SOCK,"Unsecure read URC sock num : %d",conId);
						}
					
					
					
						else if(conId==1)
						{
							 const char *start = strstr((char *)lineBuff, "+QIURC:");
							 if (start) 
							 {
								 if (sscanf(start, "+QIURC: \"recv\",%d,%Lf", &basertcmsocket_id, &basertcmdata_length) == 2) 
								 {
								
									 LOG_DBG(CH_SOCK, "Data Length: %Lf\n", basertcmdata_length);
									 //GsmReadFixedLengthData(NtripGsmSocketReadFixedLenCb,basertcmdata_length);
				           GsmReadFixedLengthDataIndexed(1, NtripGsmSocketReadFixedLenCb, basertcmdata_length);
									} 
									else 
									{
											LOG_ERR(CH_SOCK, "Failed to parse QIURC line.\n");
									}
								}
				    }
					}
				else
				{
                    //gsmSocket[gsmSocketIndex].readState = GSM_SOCK_READ_FAIL;
					LOG_ERR(CH_SOCK,"Unsecure read URC invalid sock num : %d",conId);
        }
			}        
		}
        else if(strstr((const char *)gsmSockStrTmp,"pdpdeact") != 0)
        {
            gprsActivated = 0;
            gsmGprsDeact = 1;
        }
    }
}

void UrcUnsecuredS0ClosedCb(uint8_t *lineBuff, uint16_t len)
{
    gsmSocket[0].closeState = GSM_SOCK_CLOSE_START;
}

void UrcUnsecuredS1ClosedCb(uint8_t *lineBuff, uint16_t len)
{
    gsmSocket[1].closeState = GSM_SOCK_CLOSE_START;
}









uint8_t GsmGetSockUrcStatus(void)
{
   return GsmSockCloseURC;
}

uint8_t GsmGetNtripSockUrcStatus(void)
{
   return GsmNtripSockCloseURC;
}





void GsmCloseHttpSocket(void)
{
  gsmSocket[0].closeState = GSM_SOCK_CLOSE_START;
}

void GsmCloseNtripSocket(void)
{
  gsmSocket[1].closeState = GSM_SOCK_CLOSE_START;
}





void GsmClearSockUrcStatus(void)
{
  GsmSockCloseURC = 0;
}

void GsmClearNtripSockUrcStatus(void)
{
  GsmNtripSockCloseURC = 0;
}

/******************************************************************************************
* Function:     GSMSecuredSocketURCHandler()
* Description:
*               This function is the entrance for GSM
*               SSL Secured Socket Unsolicited Result Code (URC) Handler.
* Parameters: In this function 2 types of URC's are handled as their prefix is same.
*               strURC:1] e.g. +QSSLURC: "closed",<ssid> --> Socket closed URC
*                      2] e.g. +QSSLURC: "recv",<cid>,<ssid> --> Modem has received some data from server
*                   [IN] a URC string terminated by '\0'.
* Return:       NULL 
*               
*********************************************************************************************/
void UrcSecuredSocketCb(uint8_t *lineBuff, uint16_t len)
{
    char* p1 = NULL;
	char* p2 = NULL;
	uint8_t ssid;
	p1 =(char *) lineBuff + 9;
	p2 =strstr(p1,",");
	if(p1 && p2)
	{
		memset(gsmSockStrTmp,0x0,sizeof(gsmSockStrTmp));
		strncpy((char *)gsmSockStrTmp, p1, (p2 - p1 - 1));
		
		if(strstr((const char *)gsmSockStrTmp,"closed") != 0)
		{
			//any of the range 0-5 SSL Socket is closed get the ssid of that socket
			memset(gsmSockStrTmp,0,sizeof(gsmSockStrTmp));
			p1 = p2;
			p2 = strchr(p1,'\0');
			if(p1 && p2)
			{
				strncpy((char *)gsmSockStrTmp, p1 + 1, p2 - p1 - 1);
				ssid = atoi((const char *)gsmSockStrTmp);
				if(ssid < GSM_SOCK_MAX_AVAILABLE)
				{
					gsmSocket[ssid].closeState = GSM_SOCK_CLOSE_START;
					LOG_DBG(CH_SOCK,"SSL close URC sock num : %d",ssid);
				}
				else
				{
					LOG_ERR(CH_SOCK,"SSL close URC invalid sock num : %d",ssid);
				}
			}
		}
		else if(strstr((const char *)gsmSockStrTmp,"recv") != 0)                               
		{
		{
			// this means any of the socket btn 0-5 has sent some data to the modem 
			// get the ctx id and ssid of that socket
			p2++;
			p1 = strstr(p2,","); //sslCtxId is skipped here to get the ssid as ctxId is 0 fixed
			if(p1)
			{
				p1++;
				ssid = atoi(p1);
				if(ssid < GSM_SOCK_MAX_AVAILABLE)
				{
					gsmSocket[ssid].readState = GSM_SOCK_READ_START;
					LOG_DBG(CH_SOCK,"SSL read URC sock num : %d",ssid);
				}
				else
				{
					LOG_ERR(CH_SOCK,"SSL read URC invalid sock num : %d",ssid);
				}
			}        
		}
	}
}
	}
	
	