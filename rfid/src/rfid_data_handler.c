#include "rfid_data_handler.h"



// response callback function to be called when a line is received as a reply for the command
readerCmdResponseCbFnPtr_t RfRespCbFnPtr = NULL;  

// rfid reader command handler state 
rfidProcessRespState_et rfidProcessRespState = RFID_WAIT_FOR_PREAMBLE;

// received ring buff is stored here
uint8_t rfidSerialRingBuf[MAX_RFID_RX_BUFFER_SIZE]= {0};

// received line is stored here
uint8_t rfidSerialLineBuf[MAX_RFID_RX_BUFFER_SIZE]= {0};

// the number of bytes stored in the line buff
uint16_t rfidSerialLineIndex = 0;

// circular buff that holds the serial data received from RF Reader
ringBuffer_st rfidRespRing;

uint32_t  rfCmdRespTimeout = 0;

// rfid reader command handler state 
rfidResponseState_et rfidResponseState = RFID_CMD_IDLE;

uint8_t rfCmdBuff[256]= {0};

// tick timer used in at command response timeout
uint32_t rfRespCheckStartTick =0;

uint8_t  flagCrcMatched   = 0;
uint8_t  flagLenByteRcvd  = 0;
uint8_t  flagPreambleByteRcvd = 0;

uint16_t rfRespLen = 0 , lineLen = 0;
uint16_t payLoadLength = 0;
uint8_t rcvByte = 0;
uint32_t GetRespDataTick =0;

uint8_t crcBuff[512];
uint16_t crcLen = 0;
uint8_t endMarkCrcCount = 0;

rfidMsgType_et msgType = MSG_TYPE_COMMAND;



const static rfidCmdResp_st gsmReCmdByte[] =
{
    0x08,SET_SYSTEM_RESET_CMD,       
    0x34,GET_ANTI_COL_MODE_CMD,      
    0x0D,GET_TYPE_CA_QUERY_CMD,      
//    0x03,GET_READER_INFO_MODE_CMD,   
    0x03,GET_READER_INFO_SN_CMD,     
    0x15,GET_READER_POWER_CMD,       
    0x16,SET_READER_POWER_CMD,       
    0x06,GET_REGION_CMD,             
    0x07,SET_REGION_CMD,             
    0x13,GET_FHLBT_PARAM_CMD, 
    0x36,START_AUTO_READ_CMD,
    0x37,STOP_AUTO_READ_CMD,
    0x22,START_AUTO_READ_NOTIFICATION
    
};
 

/*
* brief: uiCrc16Cal() - Cyclic Redundancy Check (CRC) computation includes all data from Len. 
* param: pData : Data whose crc needs to be calculated
*      : DataLen: Len of the data passed
*/
void uiCrc16Cal(unsigned char* pData,  uint16_t DataLen)
{
  
   uint8_t i;
    uint16_t wCrc = 0xffff;
    while (DataLen--) {
        wCrc ^= *(unsigned char *)pData++ << 8;
        for (i=0; i < 8; i++)
            wCrc = wCrc & 0x8000 ? (wCrc << 1) ^ 0x1021 : wCrc << 1;
    }
    
     pData[i++] = (wCrc & 0xffff);
     pData[i] = ((wCrc >> 8) & 0xffff);
    
    //return wCrc & 0xffff;
    
    
//	unsigned int i,j;
//	unsigned short int  uiCrcValue = PRESET_VALUE;
//
//   	   for(i = 0; i <= DataLen - 1; i++)
//	   {
//		   uiCrcValue = (uiCrcValue ^ (pData[i]));
//	  	   for(j = 0; j < 8; j++)
//	   	  {
//		 	if((uiCrcValue & 0x0001) != 0)
//		   	{
//		    	uiCrcValue = (uiCrcValue >> 1) ^ POLYNOMIAL;
//		   	}
//		 	else
//		   	{
//		    	uiCrcValue = (uiCrcValue >> 1);
//		   	}
//		}
// 	}
//     pData[i++] = (uiCrcValue & 0x000000ff);
//     pData[i] = ((uiCrcValue >> 8) & 0x000000ff);
}

/*
* brief: CheckCRC() - Checks the CRC of the received data.
* param: pData : Data whose crc needs to be calculated
*      : len   : Len of the data passed
*/
unsigned int CheckCRC(unsigned char* pData, uint16_t len)
{
    uiCrc16Cal(pData, len);
    if ((pData[len + 1] == 0) && (pData[len] == 0))  //check whether crc is matched
    {
        return 0;
    }
    else
    {
        return 0x31;
    }
}

/*
* brief: RfidUartReceiveDataCb() - callback function for the data received from the rfid uart port.
* param: rfReceiveData : single byte received on uart
*/
void RfidUartReceiveDataCb(uint8_t rfReceiveData)
{
    RingBufferFill(&rfidRespRing,rfReceiveData); //pass the received byte to the rfid ring buffer
}

void RfidDataHandlerReset(void)
{
  rfidResponseState = RFID_CMD_IDLE;
  rfidProcessRespState = RFID_WAIT_FOR_PREAMBLE;
}

/**
 *  @brief Initialise the variables used by the rfid response handler and 
 *  @param [in] fnPtr URC callback function pointer will receive the line
 *  @return none
 */
void RfidCmdHandlerInit(void)
{
	RingBufferInit(&rfidRespRing,rfidSerialRingBuf,sizeof(rfidSerialRingBuf));
	InitRfidSerialPort(RfidUartReceiveDataCb);
}

/**
 *  @brief SendDataToRfidPort() - Sends data to the rfid port
 *  @param [in] : buf - command to be send
 *              : len- length of the command
 *              : timeout to send the command
 *              : address of the callback
 *  return : of type rfidSendCmdRet_et 
 *  RFID_SEND_CMD_FAILURE if  rfidResponseState is not RFID_CMD_IDLE
 *  RFID_SEND_CMD_SUCCESS if data is sent successfully on the server.
 */
rfidSendCmdRet_et SendDataToRfidPort(uint8_t *buf,uint16_t len,uint16_t timeout,readerCmdResponseCbFnPtr_t rfRespCbFnPtr)
{
    if(rfidResponseState != RFID_CMD_IDLE)
    {
      return RFID_SEND_CMD_FAILURE;
    }
    RfRespCbFnPtr    = rfRespCbFnPtr;
    rfCmdRespTimeout = timeout;
    
    ////////to be commented
    memset(rfCmdBuff,0,sizeof(rfCmdBuff));
    RfidBytesToHex((unsigned char*)buf,len,(char*)rfCmdBuff);   //to print the command on the debug uncomment these 3 lines
    LOG_INFO(CH_RFID,"RFID Command : %s",(char *)rfCmdBuff);
    
    RfidUartSendData(buf,len);
    rfRespCheckStartTick  = GetStartTime();
    rfidResponseState = RFID_CHECK_CMD_RESPONSE;
    
    return RFID_SEND_CMD_SUCCESS;
}

/**
 *  @brief RfidPortRead() - This function drains the data filled in the rfid ring buffer
 *  @param [in] : readBuff - pointer to the data to be read
 *              : length of the data to be read
 *  return : length of the data drained from the rfid ring buffer
 */
uint16_t RfidPortRead(uint8_t *readBuff, uint16_t readLen)
{
    uint16_t len = 0;
	while(RingBufferDrain(&rfidRespRing,readBuff)) //drain the received data from the rfidRespRing
	{
		readBuff++;
		len++;
		if(len >= readLen)
		{
			break;
		}
	}
	return len;
}

/**
 *  @brief RfidIsCmdIdle() - This function checks whether the rfidResponseState is RFID_CMD_IDLE or not
 *  @param [in] - void
 *  return : 1 or 0
 */
uint8_t RfidIsCmdIdle(void)
{
	if(rfidResponseState == RFID_CMD_IDLE)
	{
		return 1;
	}
	return 0;
}

/**
 *  @brief :RfidResponseHandler() - This function is called in the while(1) of the main
 *          This function continuously monitors for data received from the rfid port.     
 *  @param [in] - void
 *  return : void
 */
void RfidResponseHandler(void) //to be called in while(1)
{
        uint8_t byte = 0;
        switch(rfidProcessRespState)
        {
            case RFID_WAIT_FOR_PREAMBLE:
              if(RfidPortRead(&byte,1) == 1) 
              {
                  if(byte == RFID_PREAMBLE_BYTE)  //here 0xBB Preamble will be received
                 {
                      memset(rfidSerialLineBuf,0,sizeof(rfidSerialLineBuf));//everytime for the new response from reader memset the linebuff
                      rfidSerialLineIndex = 0;
                      rfRespLen = 0;
                      
                      flagPreambleByteRcvd = 1;
                      
                      rfidSerialLineBuf[rfidSerialLineIndex] = byte;
                      
                      rfidSerialLineIndex++;  //first byte stored in the serial line buff
                      
                      rfidProcessRespState = RFID_WAIT_FOR_HEADER;
                      
                      GetRespDataTick      =  GetStartTime();
                      
                 }
              }
              else if(TimeSpent(GetRespDataTick,MAX_GET_RF_RESP_TIMEOUT) && flagPreambleByteRcvd)  //If no data is received from the reader then set the state to RFID_DATA_RECV_TIMEOUT   - 300 msec timeout
              {
                 GetRespDataTick = GetStartTime();
                 rfidProcessRespState = RFID_DATA_RECV_TIMEOUT;
              }
              break;
              
           case  RFID_WAIT_FOR_HEADER:  // header consists of 4 bytes 1 st byte == msg type, 1 byte code of the  command, 2 bytes payload length
              if(RfidPortRead(&byte,1) == 1) 
              {
                if(rfidSerialLineIndex == 1) //received message type
                {
                   rfidSerialLineBuf[rfidSerialLineIndex] = byte;
                   msgType = (rfidMsgType_et)rfidSerialLineBuf[rfidSerialLineIndex];
                   GetRespDataTick = GetStartTime(); //reload the start time again
                }
                else if(rfidSerialLineIndex == 2)  //received code
                {
                   rfidSerialLineBuf[rfidSerialLineIndex] = byte;
                }
                else  //receive 2 bytes of payload length
                {
                   rfidSerialLineBuf[rfidSerialLineIndex] = byte;
                }
                rfidSerialLineIndex++; 
                
                if(rfidSerialLineIndex >= 5)  //if payload length is received then wait for payload data
                {
                    //copy the payload length here
                    rfRespLen = (((unsigned int)rfidSerialLineBuf[3] << 8) + rfidSerialLineBuf[4]);
                    payLoadLength= rfRespLen;
                      
                    if(msgType == MSG_TYPE_RESPONSE)
                    {
                       rfidProcessRespState = RFID_READ_COMMAND_PAYLOAD_DATA;
                    }
                    else if(msgType == MSG_TYPE_NOTIFICATION)
                    {
                       rfidProcessRespState = RFID_READ_EPC_PAYLOAD_DATA;
                    }
                }
              }
              else if(TimeSpent(GetRespDataTick,MAX_GET_RF_RESP_TIMEOUT) && flagPreambleByteRcvd)  //If no data is received from the reader then set the state to RFID_DATA_RECV_TIMEOUT   - 300 msec timeout
              {
                 GetRespDataTick = GetStartTime();
                 rfidProcessRespState = RFID_DATA_RECV_TIMEOUT;
              }
             break;
             
       
           
            
            case RFID_READ_EPC_PAYLOAD_DATA:
            case RFID_READ_COMMAND_PAYLOAD_DATA:
               if(RfidPortRead(&byte,1) == 1)     //Read 1 byte data from the ring buffer
               {
                    if(rfidSerialLineIndex < sizeof(rfidSerialLineBuf))
                    {
                        rfidSerialLineBuf[rfidSerialLineIndex] =  byte; //pass the recived byte in the rfidSerialLineBuf
                        rfidSerialLineIndex++;
                        rfRespLen--;
                        if(rfRespLen <= 0)
                        {
                             //lineLen = rfidSerialLineIndex; 
                             endMarkCrcCount = 3;
                             rfidProcessRespState = RFID_WAIT_FOR_ENDMARK_CRC;   //when total length is received , get the endmark and crc
                        }
                    }
                    else
                    {
                        rfidSerialLineIndex--;  //should not come here
                    } 
                }
                else if(TimeSpent(GetRespDataTick,MAX_GET_RF_RESP_TIMEOUT)) //If only a length byte is received and data not receieved the set the state to RFID_DATA_RECV_TIMEOUT
                {
                    GetRespDataTick = GetStartTime();
                    rfidProcessRespState = RFID_DATA_RECV_TIMEOUT;
                }
              break;
              
               case RFID_WAIT_FOR_ENDMARK_CRC:
                if(RfidPortRead(&byte,1) == 1)     //Read 1 byte data from the ring buffer
               {
                    if(rfidSerialLineIndex < sizeof(rfidSerialLineBuf))
                    {
                        rfidSerialLineBuf[rfidSerialLineIndex] =  byte; //pass the recived byte in the rfidSerialLineBuf
                        rfidSerialLineIndex++;
                        endMarkCrcCount--;
                        if(endMarkCrcCount <= 0)
                        {
                             lineLen = rfidSerialLineIndex; 
                             endMarkCrcCount = 0;
                             rfidProcessRespState = RFID_CHECK_CRC;   //when total length is received , get the endmark and crc
                        }
                    }
                    else
                    {
                        rfidSerialLineIndex--;  //should not come here
                    } 
                }
                else if(TimeSpent(GetRespDataTick,MAX_GET_RF_RESP_TIMEOUT)) //If only a length byte is received and data not receieved the set the state to RFID_DATA_RECV_TIMEOUT
                {
                    GetRespDataTick = GetStartTime();
                    rfidProcessRespState = RFID_DATA_RECV_TIMEOUT;
                }
                 break;
              
              case RFID_CHECK_CRC:
                crcLen = lineLen -1; //as to skip  preamble 0xBB 
                
                memset(crcBuff,0,sizeof(crcBuff));
                memcpy(crcBuff,(char*)&rfidSerialLineBuf[1],crcLen);
                
                if(CheckCRC(crcBuff,crcLen) == 0) //if CRC matched
                {
                      flagCrcMatched  = 1;
                }
                else   //if CRC Mismatched
                {
                       LOG_INFO(CH_RFID,"RFID Response CRC Mismatched"); 
                       flagCrcMatched = 0;
                       rfidProcessRespState =  RFID_DATA_RECV_TIMEOUT;  //to discard the whatever received in the rf serial line Buff
                }
                break;
          
              case RFID_DATA_RECV_TIMEOUT:                      //clear the  serial line buff and reset all the flags
                 memset(rfidSerialLineBuf,0,sizeof(rfidSerialLineBuf));
                 rfidSerialLineIndex = 0;
                 flagLenByteRcvd =0;
                 flagCrcMatched =0;
                 rfidProcessRespState = RFID_WAIT_FOR_PREAMBLE;
              break;
        }
        
        GetRfidResponseData();  //check the response datas reCmd and then pass it to the callback
}

/**
 *  @brief CheckRfCmdResponse() - This function checks for which rfid command response is received
 *  @param [in] - resp - data received on serial line
 *                respLen - length of the received response
 *  return :rfRespCode of type rfidReaderCmdCode_et 
 */
rfidReaderCmdCode_et CheckRfCmdResponse(uint8_t *resp, uint16_t respLen)
{
	uint8_t i;
	rfidReaderCmdCode_et rfRespCode = START_AUTO_READ_NOTIFICATION; //By defualt the command code for received data is 0x22
	for(i = 0; i < NUM_ELEMS(gsmReCmdByte); i++)  //check in the array which response matches with the received response
	{
		if(resp[2] ==  gsmReCmdByte[i].response)  //3rd byte received from the reader is the re-cmd
		{
			rfRespCode = gsmReCmdByte[i].respCode;
			break;
        }
	}
	return rfRespCode;
}

/**
 *  @brief GetRfidResponseData() - This function handles the different states of the rfidResponseState
 *  when rfidResponseState is RFID_CHECK_CMD_RESPONSE it keeps on monitoring whether flagCrcMatched is set 
 *  if it is set then passes that command code,data received and length of the data received.
 *  @param [in] - void
 *  return :void
 */
void GetRfidResponseData(void)
{
      rfidRespCb_et rfRespCbRet;
      rfidReaderCmdCode_et reCmdCode;
      
      switch(rfidResponseState)
      {
      case RFID_CMD_IDLE:  //if it is in auto read start and any packet is received then that is a notification packet for the rfid tag which is received
        if(flagCrcMatched) //if crc is matched then only process the data
        {        
            reCmdCode = CheckRfCmdResponse(rfidSerialLineBuf,lineLen);
            rfRespCbRet = RfRespCbFnPtr(reCmdCode,rfidSerialLineBuf,lineLen);
            if(rfRespCbRet == RFID_RESP_CB_OK_COMPLETE || rfRespCbRet == RFID_RESP_CB_ERROR_COMPLETE)//if cb is compeleted or not set the state to idle and wait for length byte
            {
                 rfidResponseState = RFID_CMD_IDLE;
                 rfidProcessRespState = RFID_WAIT_FOR_PREAMBLE;
            }
            else if(rfRespCbRet ==  RFID_RESP_CB_WAIT)
            {
                rfidProcessRespState = RFID_WAIT_FOR_PREAMBLE;  //if more data to come as status is 0x03 just chnage the rfidProcessState
            }
        }
        flagCrcMatched = 0;
        break;
        
      case RFID_SEND_CMD:
        break;
        
      case RFID_CHECK_CMD_RESPONSE:
        if(flagCrcMatched) //if crc is matched then only process the data
        {        
            // process received RFID line
//            LOG_INFO(CH_RFID,"Response : %s",rfidSerialLineBuf); 
            
           ////////to be commented
            memset(rfCmdBuff,0,sizeof(rfCmdBuff));
            RfidBytesToHex((unsigned char*)rfidSerialLineBuf,lineLen,(char*)rfCmdBuff);   //to print the command on the debug uncomment these 3 lines
            LOG_INFO(CH_RFID,"RFID Response : %s",(char *)rfCmdBuff);
    
    
            reCmdCode = CheckRfCmdResponse(rfidSerialLineBuf,lineLen);
            rfRespCbRet = RfRespCbFnPtr(reCmdCode,rfidSerialLineBuf,lineLen);
            if(rfRespCbRet == RFID_RESP_CB_OK_COMPLETE || rfRespCbRet == RFID_RESP_CB_ERROR_COMPLETE)//if cb is compeleted or not set the state to idle and wait for length byte
            {
                 rfidResponseState = RFID_CMD_IDLE;
                 rfidProcessRespState = RFID_WAIT_FOR_PREAMBLE;
            }
            else if(rfRespCbRet ==  RFID_RESP_CB_WAIT)
            {
                rfidProcessRespState = RFID_WAIT_FOR_PREAMBLE;  //if more data to come as status is 0x03 just chnage the rfidProcessState
            }
        }
        else if(TimeSpent(rfRespCheckStartTick,rfCmdRespTimeout))
        {
           LOG_INFO(CH_RFID,"RFID Response Timeout");     //If timeout occurred and no data is received from the rfid then pass the reCmdCode RF_RESP_TIMEOUT to the cb and again wait for the length byte
           reCmdCode = RF_RESP_TIMEOUT;
           rfRespCbRet = RfRespCbFnPtr(reCmdCode,rfidSerialLineBuf,lineLen);
           rfidResponseState = RFID_CMD_IDLE;
           rfidProcessRespState = RFID_WAIT_FOR_PREAMBLE;
        }
        flagCrcMatched = 0;
        break;   
      }

}

uint16_t RfidGetPayloadLen(void)
{
  return payLoadLength;

}
