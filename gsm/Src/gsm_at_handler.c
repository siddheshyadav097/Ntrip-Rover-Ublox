#include "gsm_at_handler.h"

// gsm at command handler state 
gsmResponseState_et gsmResponseState = GSM_CMD_IDLE;
// received data is store in this buffer
uint8_t gsmSerialRingBuf[MAX_GSM_RX_BUFFER_SIZE] = {0};
// circular buff that holds the serial data received from GSM
ringBuffer_st gsmRespRing;
// the received serial data from gsm is sepearated line by line and is stored in gsmSerialLineBuf
uint8_t gsmSerialLineBuf[MAX_GSM_RX_BUFFER_SIZE] = {0};
// the number of bytes stored in the line buff
uint16_t gsmSerialLineIndex = 0;
// the length of the received line from GSM
uint16_t gsmSerialLineLength = 0;
uint8_t flagLineReceived = 0;

// the received data from gsm is to be processed as a line or fixed length data to be received is
// set here
gsmProcessReceiveState_et gsmProcessReceiveState = GSM_WAIT_FOR_LINE;

// it holds the address of the at command to be sent
uint8_t *atCmd;
// the length of the at command string is stored
uint16_t atCmdLen = 0;
// timeout of the at command response in ms
uint32_t gsmAtRespTimeout = 0;
// tick timer used in at command response timeout
uint32_t gsmATCmdStartTick = 0;

// this flag is set when prompt char is received
uint8_t flagPromptChar = 0;
// this flag is set when 0x0d is received
uint8_t flagCarriageRetReceived = 0;

// response callback function to be called when a line is received as a reply for the command
GsmCmdResponseCbFnPtr_t GsmRespCbFnPtr = NULL;  
GsmReadFixedLenDataFnPtr_t GsmReadFixedLenCb = NULL;

// the total length of the fixed length data
uint16_t gsmReadFixedLenDataTotalSize = 0;
// the number of bytes received from fixed length bytes
uint16_t gsmReadFixedLenDataNumReceived = 0;
// this flag is set when the complete fixed length data is received
uint8_t flagFixedLenDataReceived = 0;





static GsmSocketRx_t socketRx[MAX_SOCKETS];









// this function is called by gsm UART reception ISR
void GsmUartReceiveDataCb(uint8_t receiveData);
/**
 *  @brief Initialise the variables used by at response handler and 
 *  @param [in] fnPtr URC callback function pointer will receive the line
 *  @return none
 */
void GsmAtHandlerInit(void)
{
	RingBufferInit(&gsmRespRing,gsmSerialRingBuf,sizeof(gsmSerialRingBuf));
	GsmUartPortInit(GsmUartReceiveDataCb);
	gsmResponseState = GSM_CMD_IDLE;
}

void GsmUartReceiveDataCb(uint8_t receiveData)
{
    RingBufferFill(&gsmRespRing,receiveData);
}

void GsmReadFixedLengthData(GsmReadFixedLenDataFnPtr_t fnPtr ,uint16_t length)
{
	GsmReadFixedLenCb = fnPtr;
	gsmReadFixedLenDataTotalSize = length;
	gsmReadFixedLenDataNumReceived = 0;
	// the first byte will be a line feed so skip and then proceed
	gsmProcessReceiveState = GSM_WAIT_FOR_FIXED_LEN;
	flagFixedLenDataReceived = 0;
    gsmATCmdStartTick = GetStartTime();
}



void GsmReadFixedLengthDataIndexed(uint8_t socketIndex, GsmReadFixedLenDataFnPtr_t fnPtr, uint16_t length)
{
    if (socketIndex >= MAX_SOCKETS) return;

    socketRx[socketIndex].cb = fnPtr;
    socketRx[socketIndex].totalLen = length;
    socketRx[socketIndex].received = 0;
    socketRx[socketIndex].flagDataReceived = 0;
    socketRx[socketIndex].inProgress = 1;
}













uint8_t GsmIsAtIdle(void)
{
	if(gsmResponseState == GSM_CMD_IDLE)
	{
		return 1;
	}
	return 0;
}

gsmSendCmdRet_et GsmSendAtCmd(char *cmdBuf, uint16_t cmdLen, uint32_t timeout,GsmCmdResponseCbFnPtr_t gsmRespCbFnPtr)  
{
    if(gsmResponseState != GSM_CMD_IDLE)
    {
        return GSM_SEND_CMD_FAILURE;
    }
	GsmRespCbFnPtr = gsmRespCbFnPtr;
    gsmAtRespTimeout = timeout;
    LOG_DBGS(CH_GSM,(char *)cmdBuf);
	GsmUartSendData(cmdBuf,cmdLen);
	gsmATCmdStartTick = GetStartTime();
    gsmResponseState = GSM_CHECK_CMD_RESPONSE;
	
    return GSM_SEND_CMD_SUCCESS;
}
/*
void GsmResponseHandler(void)
{
    uint8_t rByte;
	gsmCmdResponseCode_et cmdRespCode;
	gsmUrcResponseCode_et urcRespCode;
	gsmRespCb_et respCbRet;
	uint8_t flagCallCmdRespCb = 0;
    
    // check whether a line is received
	while(RingBufferDrain(&gsmRespRing,&rByte) == 1)
	{
		switch(gsmProcessReceiveState)
		{
			case GSM_WAIT_FOR_LINE:
				if (rByte == 0x0D)
				{  
					gsmSerialLineBuf[gsmSerialLineIndex] = '\0';
					if(gsmSerialLineIndex != 0)
					{
						flagCarriageRetReceived = 1;
						gsmSerialLineLength = gsmSerialLineIndex;
					}
				}
				else if(rByte == 0x0A)
				{
					if(flagCarriageRetReceived)
					{
						gsmSerialLineLength = gsmSerialLineIndex;
						flagLineReceived = 1;
						gsmSerialLineIndex = 0;
						flagCarriageRetReceived = 0;
					}
				}
				else
				{
					if (gsmSerialLineIndex == 0)
                    {
                        if (rByte == '>')
                        {
                            flagPromptChar = 1;
                        }
                    }
					gsmSerialLineBuf[gsmSerialLineIndex] = rByte;
                    gsmSerialLineIndex++;
                    if (gsmSerialLineIndex >= MAX_GSM_RX_BUFFER_SIZE)
                    {
                        // should not come here this can happen only if the line buffer size is smaller
						// than the line received
						gsmSerialLineIndex = gsmSerialLineIndex - 1;
                    }
				}
			break;
			
			case GSM_WAIT_FOR_FIXED_LEN:
				GsmReadFixedLenCb(rByte);
				gsmReadFixedLenDataNumReceived++;
				if(gsmReadFixedLenDataNumReceived >= gsmReadFixedLenDataTotalSize)
				{
					//TODO inform the cmd call back function that the data is received completely
					flagFixedLenDataReceived = 1;
					gsmProcessReceiveState = GSM_WAIT_FOR_LINE;
				}
			break;
        }
        if(flagLineReceived == 1 || flagPromptChar == 1 || flagFixedLenDataReceived == 1)
        {
            break;
        }
	}
	
	// the above processing with either set a flagline received or flagFixedLenDataReceived
	switch(gsmResponseState)
	{
		case GSM_CMD_IDLE:
            if(flagLineReceived)
            {
                // process received line
				urcRespCode = CheckUrcResonse(gsmSerialLineBuf,gsmSerialLineLength);
				flagLineReceived = 0;
            }                                                  
        break;
        
        case GSM_SEND_CMD:
            
        break;
		
		case GSM_CHECK_CMD_RESPONSE:
			if(flagPromptChar)
			{
                GsmRespCbFnPtr(RESP_PROMPT_CHAR,gsmSerialLineBuf,gsmSerialLineLength);
                flagPromptChar = 0;
			}
			else if(flagLineReceived)
            {
                // process received line
                LOG_DBG(CH_GSM,"line : %s",gsmSerialLineBuf);
				cmdRespCode = CheckCmdResponse(gsmSerialLineBuf,gsmSerialLineLength);
				if(cmdRespCode == RESP_OTHER)
				{
					urcRespCode = CheckUrcResonse(gsmSerialLineBuf,gsmSerialLineLength);
					if(urcRespCode == URC_OTHER)// || (urcRespCode == URC_READ_SOCKET_DATA))
					{
						flagCallCmdRespCb = 1;
					}
				}
				else
				{
					flagCallCmdRespCb = 1;
				}
				
				// check whether the cmd resp cb function to be called
				if(flagCallCmdRespCb)
				{
                    flagCallCmdRespCb = 0;
					respCbRet = GsmRespCbFnPtr(cmdRespCode,gsmSerialLineBuf,gsmSerialLineLength);
					if(respCbRet == GSM_RESP_CB_OK_COMPLETE || respCbRet == GSM_RESP_CB_ERROR_COMPLETE)
					{
						 gsmResponseState = GSM_CMD_IDLE;
					}
				}
				flagLineReceived = 0;
            }
			else if(TimeSpent(gsmATCmdStartTick,gsmAtRespTimeout))
			{
				//timeour occurred, now call the callback with the timeout event and change the state to idle
				cmdRespCode = RESP_TIMEOUT;
				GsmRespCbFnPtr(cmdRespCode,gsmSerialLineBuf,gsmSerialLineLength);
				gsmResponseState = GSM_CMD_IDLE;        
			}
			else if(flagFixedLenDataReceived == 1)
			{
				//TODO call the call back function
				flagFixedLenDataReceived = 0;
			}
		break;
	}
}
*///original


void GsmResponseHandler(void)
{
    uint8_t rByte;
    gsmCmdResponseCode_et cmdRespCode;
    gsmUrcResponseCode_et urcRespCode;
    gsmRespCb_et respCbRet;
    uint8_t flagCallCmdRespCb = 0;

    while (RingBufferDrain(&gsmRespRing, &rByte) == 1)
    {
        uint8_t handledBySocket = 0;

        // Handle fixed-length receive for each socket
        for (uint8_t i = 0; i < MAX_SOCKETS; i++)
        {
            if (socketRx[i].inProgress)
            {
                socketRx[i].cb(rByte);
                socketRx[i].received++;

                if (socketRx[i].received >= socketRx[i].totalLen)
                {
                    socketRx[i].flagDataReceived = 1;
                    socketRx[i].inProgress = 0;
                }

                handledBySocket = 1;
                break; // One byte only goes to one socket
            }
        }

        if (handledBySocket)
            continue;

        // Regular line-based AT command processing
        switch (gsmProcessReceiveState)
        {
            case GSM_WAIT_FOR_LINE:
                if (rByte == 0x0D)
                {
                    gsmSerialLineBuf[gsmSerialLineIndex] = '\0';
                    if (gsmSerialLineIndex != 0)
                    {
                        flagCarriageRetReceived = 1;
                        gsmSerialLineLength = gsmSerialLineIndex;
                    }
                }
                else if (rByte == 0x0A)
                {
                    if (flagCarriageRetReceived)
                    {
                        gsmSerialLineLength = gsmSerialLineIndex;
                        flagLineReceived = 1;
                        gsmSerialLineIndex = 0;
                        flagCarriageRetReceived = 0;
                    }
                }
                else
                {
                    if (gsmSerialLineIndex == 0 && rByte == '>')
                    {
                        flagPromptChar = 1;
                    }
                    gsmSerialLineBuf[gsmSerialLineIndex++] = rByte;
                    if (gsmSerialLineIndex >= MAX_GSM_RX_BUFFER_SIZE)
                        gsmSerialLineIndex--;
                }
                break;

            default:
                break;
        }

        if (flagLineReceived || flagPromptChar)
            break;
    }

    // Process parsed responses
    switch (gsmResponseState)
    {
        case GSM_CMD_IDLE:
            if (flagLineReceived)
            {
                urcRespCode = CheckUrcResonse(gsmSerialLineBuf, gsmSerialLineLength);
                flagLineReceived = 0;
            }
            break;

        case GSM_SEND_CMD:
            // Not handled here
            break;

        case GSM_CHECK_CMD_RESPONSE:
            if (flagPromptChar)
            {
                GsmRespCbFnPtr(RESP_PROMPT_CHAR, gsmSerialLineBuf, gsmSerialLineLength);
                flagPromptChar = 0;
            }
            else if (flagLineReceived)
            {
                LOG_DBG(CH_GSM, "line : %s", gsmSerialLineBuf);
                cmdRespCode = CheckCmdResponse(gsmSerialLineBuf, gsmSerialLineLength);

                if (cmdRespCode == RESP_OTHER)
                {
                    urcRespCode = CheckUrcResonse(gsmSerialLineBuf, gsmSerialLineLength);
                    if (urcRespCode == URC_OTHER)
                        flagCallCmdRespCb = 1;
                }
                else
                {
                    flagCallCmdRespCb = 1;
                }

                if (flagCallCmdRespCb)
                {
                    flagCallCmdRespCb = 0;
                    respCbRet = GsmRespCbFnPtr(cmdRespCode, gsmSerialLineBuf, gsmSerialLineLength);
                    if (respCbRet == GSM_RESP_CB_OK_COMPLETE || respCbRet == GSM_RESP_CB_ERROR_COMPLETE)
                    {
                        gsmResponseState = GSM_CMD_IDLE;
                    }
                }
                flagLineReceived = 0;
            }
            else if (TimeSpent(gsmATCmdStartTick, gsmAtRespTimeout))
            {
                cmdRespCode = RESP_TIMEOUT;
                GsmRespCbFnPtr(cmdRespCode, gsmSerialLineBuf, gsmSerialLineLength);
                gsmResponseState = GSM_CMD_IDLE;
            }
            break;
    }
}