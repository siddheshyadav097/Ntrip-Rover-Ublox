#include "rfid_data_handler.h"
#include "serial_rfid_api.h"


// response callback function to be called when a line is received as a reply for the command
readerCmdResponseCbFnPtr_t RfRespCbFnPtr = NULL;  

// rfid reader command handler state 
rfidProcessRespState_et rfidProcessRespState = RFID_WAIT_FOR_LEN_BYTE;

// received ring buff is stored here
uint8_t rfidSerialRingBuf[MAX_RFID_RX_BUFFER_SIZE];

// received line is stored here
uint8_t rfidSerialLineBuf[MAX_RFID_RX_BUFFER_SIZE];

// the number of bytes stored in the line buff
uint16_t rfidSerialLineIndex = 0;

// circular buff that holds the serial data received from RF Reader
ringBuffer_st rfidRespRing;

uint32_t  rfCmdRespTimeout = 0;

// rfid reader command handler state 
rfidResponseState_et rfidResponseState = RFID_CMD_IDLE;

uint8_t rfCmdBuff[50];

// tick timer used in at command response timeout
uint32_t rfRespCheckStartTick =0;

uint8_t  flagCrcMatched   = 0;
uint8_t  flagLenByteRcvd  = 0;

uint16_t rfRespLen = 0 , lineLen = 0;
uint8_t rcvByte = 0;
uint32_t GetRespDataTick =0;

const static rfidCmdResp_st gsmReCmdByte[] =
{
    0x21,GET_READER_INFO_CMD,        //This Command is used to get reader-related information such as reader address (Adr), firmware version, supported protocol type, Inventory ScanTime, power and frequency.
    0x22,SET_READERS_REGION_CMD,     //Sets the current region. The function is used to set the reader working of the lower limit and the upper limit of frequency. 
    0x24,SET_READERS_ADDRESS_CMD,    //This Command is used to set a new address of the reader.
    0x25,SET_READERS_SCAN_TIME_CMD,  //This Command is used to set a new value to Inventory ScanTime of an appointed reader. The range is 3~255 corresponding to 3*100ms~255*100ms Inventory ScanTime. 
    0x28,SET_READERS_BAUD_RATE_CMD,  //The Command is used to change the serial port baud rate.
    0x2F,SET_READERS_POWER_CMD,      //The Command is used to set the power of reader. 
    0x35,SET_READERS_WORK_MODE_CMD,  //The Command is used to set work mode parameter.
    0x36,GET_READERS_WORK_MODE_CMD,  //The Command is used to get work mode parameter.
    0x01,INVENTORY_TAGS_CMD          //The Command is used to inventory tags in the effective field and get their EPC values.
};
 

/*
* brief: uiCrc16Cal() - Cyclic Redundancy Check (CRC) computation includes all data from Len. 
* param: pData : Data whose crc needs to be calculated
*      : DataLen: Len of the data passed
*/
void uiCrc16Cal(unsigned char* pData,  uint16_t DataLen)
{
	unsigned int i,j;
	unsigned short int  uiCrcValue = PRESET_VALUE;

   	   for(i = 0; i <= DataLen - 1; i++)
	   {
		   uiCrcValue = (uiCrcValue ^ (pData[i]));
	  	   for(j = 0; j < 8; j++)
	   	  {
		 	if((uiCrcValue & 0x0001) != 0)
		   	{
		    	uiCrcValue = (uiCrcValue >> 1) ^ POLYNOMIAL;
		   	}
		 	else
		   	{
		    	uiCrcValue = (uiCrcValue >> 1);
		   	}
		}
 	}
     pData[i++] = (uiCrcValue & 0x000000ff);
     pData[i] = ((uiCrcValue >> 8) & 0x000000ff);
}

/*
* brief: CheckCRC() - Checks the CRC of the received data.
* param: pData : Data whose crc needs to be calculated
*      : len   : Len of the data passed
*/
unsigned int CheckCRC(unsigned char* pData, uint16_t len)
{
    uiCrc16Cal(pData, len);
    if ((pData[len + 1] == 0) && (pData[len] == 0))
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
    RingBufferFill(&rfidRespRing,rfReceiveData);
}

/**
 *  @brief Initialise the variables used by at response handler and 
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
    memset(rfCmdBuff,0,sizeof(rfCmdBuff));
//  RfidbytesToHex((unsigned char*)buf,len,(char*)rfCmdBuff);   //to print the command on the debug uncomment this function
//  LOG_INFO(CH_RFID,"RFID Command : %s",(char *)rfCmdBuff);
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
	while(RingBufferDrain(&rfidRespRing,readBuff))
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
            case RFID_WAIT_FOR_LEN_BYTE:
              if(RfidPortRead(&byte,1) == 1) 
              {
                  memset(rfidSerialLineBuf,0,sizeof(rfidSerialLineBuf));//everytime for the new response from reader memset the linebuff
                  rfidSerialLineIndex = 0;
                  
                  rfidSerialLineBuf[rfidSerialLineIndex] = byte;
                  rfRespLen = byte;
                  if(rfRespLen > 0)
                  {
                     flagLenByteRcvd = 1;
                     rfidSerialLineIndex++;  //first byte stored in the serial line buff is length
                     rfidProcessRespState =  RFID_READ_RESP_DATA;
                     GetRespDataTick      =  GetStartTime();
                  }
              }
              else if(TimeSpent(GetRespDataTick,MAX_GET_RF_RESP_TIMEOUT) && flagLenByteRcvd)
              {
                 GetRespDataTick = GetStartTime();
                 rfidProcessRespState = RFID_DATA_RECV_TIMEOUT;
              }
              break;
              
            case RFID_READ_RESP_DATA:
               if(RfidPortRead(&byte,1) == 1) 
               {
                    if(rfidSerialLineIndex < sizeof(rfidSerialLineBuf))
                    {
                        rfidSerialLineBuf[rfidSerialLineIndex] =  byte;
                        rfidSerialLineIndex++;
                        rfRespLen--;
                        if(rfRespLen <= 0)
                        {
                             lineLen = rfidSerialLineIndex;
                             rfidProcessRespState = RFID_CHECK_CRC;   
                        }
                    }
                    else
                    {
                        rfidSerialLineIndex--;  //should not come here
                    } 
                }
                else if(TimeSpent(GetRespDataTick,MAX_GET_RF_RESP_TIMEOUT))
                {
                    GetRespDataTick = GetStartTime();
                    rfidProcessRespState = RFID_DATA_RECV_TIMEOUT;
                }
              break;
              
              case RFID_CHECK_CRC:
                if(CheckCRC(rfidSerialLineBuf,lineLen) == 0) //if CRC matched
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
          
              case RFID_DATA_RECV_TIMEOUT:
                 memset(rfidSerialLineBuf,0,sizeof(rfidSerialLineBuf));
                 rfidSerialLineIndex = 0;
                 flagLenByteRcvd =0;
                 flagCrcMatched =0;
                 rfidProcessRespState = RFID_WAIT_FOR_LEN_BYTE;
              break;
        }
        
        GetRfidResponseData();  //check the response datas reCmd and then pass it to the callback
}

rfidReaderCmdCode_et CheckRfCmdResponse(uint8_t *resp, uint16_t respLen)
{
	uint8_t i;
	rfidReaderCmdCode_et rfRespCode = INVENTORY_TAGS_CMD;
	for(i = 0; i < NUM_ELEMS(gsmReCmdByte); i++)
	{
		if(resp[2] ==  gsmReCmdByte[i].response)
		{
			rfRespCode = gsmReCmdByte[i].respCode;
			break;
        }
	}
	return rfRespCode;
}

void GetRfidResponseData(void)
{
      rfidRespCb_et rfRespCbRet;
      rfidReaderCmdCode_et reCmdCode;
      
      switch(rfidResponseState)
      {
      case RFID_CMD_IDLE:
        break;
        
      case RFID_SEND_CMD:
        break;
        
      case RFID_CHECK_CMD_RESPONSE:
        if(flagCrcMatched) //if crc is matched then only process the data
        {        
            // process received RFID line
//            LOG_INFO(CH_RFID,"Response : %s",rfidSerialLineBuf); 
            reCmdCode = CheckRfCmdResponse(rfidSerialLineBuf,lineLen);
            rfRespCbRet = RfRespCbFnPtr(reCmdCode,rfidSerialLineBuf,lineLen);
            if(rfRespCbRet == RFID_RESP_CB_OK_COMPLETE || rfRespCbRet == RFID_RESP_CB_ERROR_COMPLETE)
            {
                 rfidResponseState = RFID_CMD_IDLE;
                 rfidProcessRespState = RFID_WAIT_FOR_LEN_BYTE;
            }
            else if(rfRespCbRet ==  RFID_RESP_CB_WAIT)
            {
                rfidProcessRespState = RFID_WAIT_FOR_LEN_BYTE;  //if more data to come as status is 0x03 just chnage the rfidProcessState
            }
        }
        else if(TimeSpent(rfRespCheckStartTick,rfCmdRespTimeout))
        {
           LOG_INFO(CH_RFID,"RFID Response Timeout"); 
           reCmdCode = RF_RESP_TIMEOUT;
           rfRespCbRet = RfRespCbFnPtr(reCmdCode,rfidSerialLineBuf,lineLen);
           rfidResponseState = RFID_CMD_IDLE;
           rfidProcessRespState = RFID_WAIT_FOR_LEN_BYTE;
        }
        flagCrcMatched = 0;
        break;   
      }

}
