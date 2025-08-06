#include <string.h>
#include <stdlib.h>
#include "gsm_sms.h"
#include "gsm_statemachine.h"

gsmCmdState_et sendSmsAtCmdState   = AT_CMD_SEND;
gsmCmdState_et deleteSmsAtCmdState = AT_CMD_SEND;
gsmCmdState_et readSmsAtCmdState   = AT_CMD_SEND;

smsDeleteState_et  gsmDelSmsFnState  = SMS_DELETE_WAIT_FOR_START;
smsSendState_et    gsmSendSmsFnState = SMS_SEND_WAIT_FOR_START;
smsReadState_et    gsmReadSmsFnState = SMS_READ_WAIT_FOR_START;

smsDeleteFlag_et   deleteValue      = SMS_DEL_ALL_MSG;

gsmSmsState_et smsState = GSM_SMS_IDLE;
//uint8_t flagSmsSendSuccess = 0;
//uint8_t flagSmsDeleteSuccess = 0;
//uint8_t flagSmsReadSuccess = 0;
uint8_t flagSMSlineReceived =0;
uint8_t flagCmtiUrcRcvd = 0;
extern uint8_t fotaResponse;

gsmRespCb_et SendSmsRespCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len);
gsmRespCb_et ReadSmsRespCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len);
gsmRespCb_et SmsOkRespCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len);

const uint8_t sendSmsCmd[] = "AT+CMGS=";
const uint8_t delSmsCmd[]  = "AT+CMGD=";
const uint8_t readSmsCmd[] = "AT+CMGR=";


uint8_t smsReceiveIndex = 0;
uint8_t flagDelCurrentSms=0;
uint8_t flagSendSMSResponse =0;
char strIndex[10];

uint32_t gsmPollSMSTick = 0;
smsAtCmd_st smsCmdList[3]= {
    {"AT+CMGS=",SEND_SMS_TIMEOUT,SendSmsRespCb},
    {"AT+CMGD=",DELETE_SMS_TIMEOUT,SmsOkRespCb},
    {"AT+CMGR=",READ_SMS_TIMEOUT,ReadSmsRespCb}
};

// the current init command is in progress
gsmSmsReadCMD_et gsmSmsReadCmdIndex = READ_SMS_TEXT_MODE;

smsAtCmd_st smsReadList[2]={
  {"AT+CMGF=1\r",500,SmsOkRespCb},
//  {"AT+CMGR=",READ_SMS_TIMEOUT,ReadSmsRespCb}
  
};

char smsAtCmdBuff[100];
uint8_t delsmsidx =1;
char *smsSendBuff = NULL;
uint8_t* smsSendNum = NULL;
uint8_t smsSendLen =0;
uint8_t readSmsIndex  =1;
gsmReadSmsData_st  GsmReadSmsData = {0};

GsmReceiveSmsCallbackFnPtr_t GsmReceiveSmsCallback;

void GsmSmsInit(GsmReceiveSmsCallbackFnPtr_t fnPtr)
{
    GsmReceiveSmsCallback = fnPtr;  
	gsmPollSMSTick = GetStartTime();
}

void GsmSmsSendStart(void)
{
   sendSmsAtCmdState   = AT_CMD_SEND;
   gsmSendSmsFnState   = SMS_SENDING_IN_PROGRESS;
   //flagSmsSendSuccess  = 0;
	flagSendSMSResponse  =0;
   memset(smsAtCmdBuff,0x00,sizeof(smsAtCmdBuff));
   snprintf((char *)smsAtCmdBuff,sizeof(smsAtCmdBuff),"%s\"%s\"\r",sendSmsCmd,smsSendNum);
   smsCmdList[0].cmd = smsAtCmdBuff;
}

void GsmSmsDeleteStart(void)
{
   deleteSmsAtCmdState  = AT_CMD_SEND;
   gsmDelSmsFnState     = SMS_DELETE_IN_PROGRESS;
//   flagSmsDeleteSuccess = 0;
   flagDelCurrentSms     =0;
   memset(smsAtCmdBuff,0x00,sizeof(smsAtCmdBuff));
//   snprintf((char *)smsAtCmdBuff,sizeof(smsAtCmdBuff),"%s%d,%d\r",delSmsCmd,smsReceiveIndex,SMS_DEL_INDEXED_MSG); //SMS_DEL_ALL_MSG
   snprintf((char *)smsAtCmdBuff,sizeof(smsAtCmdBuff),"%s%d,%d\r",delSmsCmd,readSmsIndex,SMS_DEL_INDEXED_MSG); //SMS_DEL_ALL_MSG
   smsCmdList[1].cmd = smsAtCmdBuff;
}

void GsmSmsSetTextMode(void)
{
   readSmsAtCmdState   = AT_CMD_SEND;
   gsmReadSmsFnState   = SMS_READING_IN_PROGRESS;
//   flagSmsReadSuccess  = 0;
   readSmsIndex        = smsReceiveIndex;
   gsmSmsReadCmdIndex  = READ_SMS_TEXT_MODE;
   flagSMSlineReceived =0;
}

void GsmSmsReadStart(void)
{
   readSmsAtCmdState   = AT_CMD_SEND;
   gsmReadSmsFnState   = SMS_READING_IN_PROGRESS;
//   flagSmsReadSuccess  = 0;
   flagSMSlineReceived = 0;
   flagDelCurrentSms   = 0;
   readSmsIndex         = smsReceiveIndex;
   memset(smsAtCmdBuff,0x00,sizeof(smsAtCmdBuff));
   snprintf((char *)smsAtCmdBuff,sizeof(smsAtCmdBuff),"%s%d\r",readSmsCmd,readSmsIndex);
   smsCmdList[2].cmd = smsAtCmdBuff;
}

/**
 *  @brief This function is called from main app to send new sms, it will get phone num 
 *  & msg content pointer & data length  from main app, for sending new SMS. 
 *  It Copies that pointer values in global pointer of this file
 *  @param [in] phno to send new SMS
 *  @param [in] send_buf data content of new SMS
 *  @param [in] length of data content
 *  @return void
 */
void SendSMS(uint8_t* phno, char* sendBuf, uint8_t length)
{
	smsSendNum = phno;
	smsSendBuff = sendBuf;
	smsSendLen = length;
	gsmSendSmsFnState = SMS_SEND_START;
}
/**
 *  @brief     :This SmsOkRespCb() callback for sms the commands expecting OK or ERROR.
 *  @Inputs:   gsmResp of type enum  gsmResPonse_et
 *             lineBuff - Pointer to the buffer in which modem response(processlinebuffer - Extended Response or SerialLineBuff)is received.
 *             len      - Length of the data received.
 *  @param     [out] gsmRespCb_et.
 *  @return    1 value of enum @ref gsmRespCb_et        
 *             :GSM_RESP_CB_COMPLETE if OK received or 
 *             :GSM_RESP_CB_ERROR_COMPLETE if ERROR is received. */
gsmRespCb_et SmsOkRespCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len)
{
    //what we want to return from the calllback depending on the reponse received
    gsmRespCb_et gsmRespRetVal = GSM_RESP_CB_WAIT;
    
    switch(gsmResp)
    {
        case RESP_OK: 
            //  the resp ret val is used by the call back function 
            gsmRespRetVal  = GSM_RESP_CB_OK_COMPLETE;
            // this is done to inform the init handler that the cmd response is received as it is 
            // waiting for the reply
            deleteSmsAtCmdState = AT_CMD_SUCCESS;
            readSmsAtCmdState  = AT_CMD_SUCCESS;
        break;
        
        case RESP_ERROR:
        case RESP_CME_ERROR:
        case RESP_CMS_ERROR:
        case RESP_TIMEOUT:
            LOG_ERR(CH_SMS,"SMS Delete/Set Text Mode CB failure : %d",gsmResp);
            gsmRespRetVal   = GSM_RESP_CB_ERROR_COMPLETE;   
            // this is done to inform the init handler that the cmd response is received as it is 
            // waiting for the reply
            deleteSmsAtCmdState = AT_CMD_FAILURE;
            readSmsAtCmdState   = AT_CMD_FAILURE;  
        break;
        
        case RESP_IGNORE:
        case RESP_OTHER: 
            gsmRespRetVal = GSM_RESP_CB_WAIT;    //+QNITZ/+CCLK URC is rcvd then ignore it.
        break;
        
        default:
        break;
    }
    return gsmRespRetVal;
}

/*@brief SendSmsRespCb() this callback handles the sms send callbacks.
 * if send succeds the response is OK
 * if send fails then response is ERROR
 * If error is related to ME functionality:+CMS ERROR:<err>
 *   e.g. AT+CMGS="15021012496"
 *   > 
 *   This is a test from Quectel //Enter in text, 
 *   <CTRL+Z> send message/0x1A,<ESC> quits
 *   without sending
 *   +CMGS: 247
 *   OK
*/
gsmRespCb_et SendSmsRespCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len)
{
     //what we want to return from the calllback depending on the reponse received
    gsmRespCb_et gsmRespRetVal = GSM_RESP_CB_WAIT;
    
    switch(gsmResp)
    {
        case  RESP_OK:
           LOG_DBG(CH_SMS,"Send SMS CB Success : %d",gsmResp);
           gsmRespRetVal = GSM_RESP_CB_OK_COMPLETE;                             //messgae sent and ok received
            // this is done to inform the sms handler that the cmd response is received as it is 
            // waiting for the reply
            sendSmsAtCmdState = AT_CMD_SUCCESS;
        break;
        
        case RESP_ERROR:
		case RESP_CME_ERROR:
        case RESP_CMS_ERROR:
        case RESP_TIMEOUT:
        case RESP_IGNORE:
           LOG_ERR(CH_SMS,"Send SMS CB failure : %d",gsmResp);
           gsmRespRetVal   = GSM_RESP_CB_ERROR_COMPLETE;   
            // this is done to inform the sms handler that the cmd response is received as it is 
            // waiting for the reply
            sendSmsAtCmdState = AT_CMD_FAILURE;
        break;
        
        case RESP_PROMPT_CHAR:
          GsmUartSendData(smsSendBuff,smsSendLen);                              //here send the data and then wait for data response
          GsmUartSendByte(0x1A);
          gsmRespRetVal = GSM_RESP_CB_WAIT; 
        break;
        
        case RESP_SEND_SMS:
        case RESP_OTHER:
          gsmRespRetVal = GSM_RESP_CB_WAIT; 
        break;       
       
         default:
        break;
    }
    return gsmRespRetVal;
}

/*@brief ReadSmsRespCb() this callback handles the sms read callback.
 * if send succeds the response is OK
 * if send fails then response is ERROR
 * +CMGR:
 * <stat>,<oa>,[<alpha>],<scts>[,<tooa>,<fo>,<pid>,<dcs>,<
 * sca>,<tosca>,<length>]<CR><LF><data>
 *  if sms is read successfully then gives +CMGR:
 //+CMGR: "REC UNREAD","+8615021012496","","2010/09/25 15:06:37+32",145,4,0,241,"+8
 //613800210500",145,27
*/
gsmRespCb_et ReadSmsRespCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len)
{
    gsmRespCb_et gsmRespRetVal;
    
    switch(gsmResp)
    {
        case  RESP_OK:
           gsmRespRetVal      = GSM_RESP_CB_OK_COMPLETE;                        //messgae sent and ok received
           readSmsAtCmdState  = AT_CMD_SUCCESS;
           LOG_DBGS(CH_SMS,"Read SMS CB success");
        break;
        
        case RESP_IGNORE:
            gsmRespRetVal = GSM_RESP_CB_WAIT; //0805
        break;
        
        case RESP_CME_ERROR:
        case RESP_ERROR:  
        case RESP_CMS_ERROR:
        case RESP_TIMEOUT:
            gsmRespRetVal      = GSM_RESP_CB_ERROR_COMPLETE;
            readSmsAtCmdState  = AT_CMD_FAILURE;
            LOG_ERR(CH_SMS,"Read SMS CB failure : %d",gsmResp);
        break;
        
        case  RESP_READ_SMS:                                                    //+CMGR:
            flagSMSlineReceived = 1;
            memset(GsmReadSmsData.rcvMobileNo,0,sizeof(GsmReadSmsData.rcvMobileNo));
            GsmGetSenderMobileNo(lineBuff,len); 
            gsmRespRetVal = GSM_RESP_CB_WAIT; 
        break; 
        
        case RESP_OTHER:                                                        //here get the data 
            if(flagSMSlineReceived)
            {
                memset(GsmReadSmsData.rcvContentData,0,sizeof(GsmReadSmsData.rcvContentData));
                GsmReadSmsData.rcvSmsLength = 0;
                if(len < 256)  //if received sms is less than 512 bytes then only copy it
                {
                  strncpy((char*)GsmReadSmsData.rcvContentData ,(char*)lineBuff,len);
                  GsmReadSmsData.rcvSmsLength = len;
                }
                gsmRespRetVal = GSM_RESP_CB_WAIT;
                //LOG_INFO(CH_SMS,"read CB msg : %s",lineBuff);
                //LOG_INFO(CH_SMS,"msgLen : %d",len);
            }
            else
            {
              gsmRespRetVal = GSM_RESP_CB_WAIT;
            }
        break;
        
        default:
        break;
    }
    return gsmRespRetVal;
}

void GsmGetSenderMobileNo(uint8_t* cmgrLinePtr,uint16_t cmgrLineLen)
{
     char *cBuffAdd1,*cBuffAdd2 =  NULL;  
     //+CMGR: "REC UNREAD","+8615021012496","","2010/09/25 15:06:37+32",145,4,0,241,"+8
     //613800210500",145,27
     //Read Sender's mobile number
     cBuffAdd1 =  strstr((char*)cmgrLinePtr,",");  
     cBuffAdd1++;
     if(*cBuffAdd1 == '"')
     {
        cBuffAdd1++;
        cBuffAdd2 = strchr(cBuffAdd1,'"');  
        cBuffAdd2--;
        if ((cBuffAdd2 - cBuffAdd1 + 1) >= 10)
        {
            memset(GsmReadSmsData.rcvMobileNo, 0x0, sizeof(GsmReadSmsData.rcvMobileNo));
            memcpy(GsmReadSmsData.rcvMobileNo, cBuffAdd1, (cBuffAdd2 - cBuffAdd1) + 1);
            LOG_INFO(CH_GSM,"Sender Mobile Number = %s",GsmReadSmsData.rcvMobileNo);
        }  
     }
}
/**
 *  @brief This function is called when gsm state is set to DELETE_SMS state, it will call DeleteSMS function defined
 *  in sms_api file, passing the index no of sms that has to be deleted & deleteValue parameter
 *  if SUCCESS, it gives gsm status as SMS_DELETED to main app, & set gsm state Idle
 *  if FAILURE, it logs gsm error as SMS_DELETE_FAILURE & checks for GSM network registration
 *  @return void
 */
smsDeleteState_et GsmDeleteSMSHandler(void)
{
   switch(gsmDelSmsFnState)
    {
			
		 case SMS_DELETE_WAIT_FOR_START:
			 break;
       
		 case SMS_DELETE_START:
			 break;
		 
        case SMS_DELETE_IN_PROGRESS:
            switch(deleteSmsAtCmdState)
            {
                case AT_CMD_SEND:
                    // check whether is at command is idle
                    if(GsmIsAtIdle())
                    {
                        // send the at command
                        // check whether if the command is 
                        GsmSendAtCmd(smsCmdList[1].cmd,strlen((const char *)smsCmdList[1].cmd),
                        smsCmdList[1].cmdTimeoutInMs,smsCmdList[1].respCb);
                        deleteSmsAtCmdState = AT_WAIT_REPLY;
                    }
                break;
                
                case AT_WAIT_REPLY:   
                break;
                
                case AT_CMD_SUCCESS:
                      gsmDelSmsFnState = SMS_DELETE_SUCCESS;
//                      flagSmsDeleteSuccess =1;
                break;
                
                case AT_CMD_FAILURE:
                    gsmDelSmsFnState = SMS_DELETE_FAIL;
                break;
                
                case AT_CMD_RETRY_WAIT:   
                break;
            }
        break;
        
        case SMS_DELETE_SUCCESS:   
        break;
        
        case SMS_DELETE_FAIL:   
        break;
    }
    return gsmDelSmsFnState;
}

/**
 *  @brief This function is called when gsm State is set to SEND_SMS state, it will call SendNewSMS function defined
 *  in gsm_sms_api file, passing addresses for sms_sendnum &  sms_sendbuff buffers & sms_sendlen variable
 *  if AT_CMD_SUCCESS, it gives gsm status as NEW_SMS_SENT to main app, checks for changeConfigFlag & set gsm state to either 
 *  GPRS_ACTIVATE or Idle
 *  if FAILURE, checks for GSM network registration
 *  @return void
 */
smsSendState_et GsmSendSMSHandler(void)
{
    switch(gsmSendSmsFnState)
    {
			case SMS_SEND_WAIT_FOR_START:
				break;
			
      case SMS_SEND_START:
				break;
			
        case SMS_SENDING_IN_PROGRESS:
            switch(sendSmsAtCmdState)
            {
                case AT_CMD_SEND:
                    // check whether is at command is idle
                    if(GsmIsAtIdle())
                    {
                        // send the at command
                        // check whether if the command is 
                        GsmSendAtCmd(smsCmdList[0].cmd,strlen((const char *)smsCmdList[0].cmd),
                        smsCmdList[0].cmdTimeoutInMs,smsCmdList[0].respCb);
                        sendSmsAtCmdState = AT_WAIT_REPLY;
                    }
                break;
                
                case AT_WAIT_REPLY: 
                break;
                
                case AT_CMD_SUCCESS:
                      gsmSendSmsFnState = SMS_SENDING_SUCCESS;
//                    flagSmsSendSuccess =1;
                break;
                
                case AT_CMD_FAILURE:
                    gsmSendSmsFnState = SMS_SENDING_FAIL;
                break;
                
                case AT_CMD_RETRY_WAIT:
                break;
            }
        break;
        
        case SMS_SENDING_SUCCESS:  
        break;
        
        case SMS_SENDING_FAIL:  
        break;
    }
   return  gsmSendSmsFnState;
}
/**
 *  @brief This function is called when gsm state is set to PROCESS_RECEIVED_SMS state, it will call ReceiveSMS function defined
 *  in this file, passing new msg index value, addresses for sms_readnum & sms_readbuff buffers & sms_readlen variable
 *  if AT_CMD_SUCCESS, it gives gsm status as NEW_SMS_RECEIVED to gsm_at_handler, executes receive_sms_callback function, & set gsm state
 *  to Idle.
 *  if AT_CMD_FAILURE, it logs gsm error as SMS_RECEIVE_FAILURE & checks for GSM network registration
 *  @return void
 */
smsReadState_et GsmReceiveSMSHandler(void)
{
  switch(gsmReadSmsFnState)
    {
		case  SMS_READ_WAIT_FOR_START:
			break;
		
    case    SMS_READ_START:
			break;
			
        case SMS_READING_IN_PROGRESS:
            switch(readSmsAtCmdState)
            {
                case AT_CMD_SEND:
                    // check whether is at command is idle
                    if(GsmIsAtIdle())
                    {
                        // send the at command
                        // check whether if the command is 
                        if(gsmSmsReadCmdIndex == READ_SMS_TEXT_MODE)
                        {
                            GsmSendAtCmd(smsReadList[gsmSmsReadCmdIndex].cmd,strlen((const char *)smsReadList[gsmSmsReadCmdIndex].cmd),
                            smsReadList[gsmSmsReadCmdIndex].cmdTimeoutInMs,smsReadList[gsmSmsReadCmdIndex].respCb);
                            readSmsAtCmdState = AT_WAIT_REPLY;
                        }
                        else if(gsmSmsReadCmdIndex == READ_SMS)
                        {  
                            GsmSendAtCmd(smsCmdList[2].cmd,strlen((const char *)smsCmdList[2].cmd),
                            smsCmdList[2].cmdTimeoutInMs,smsCmdList[2].respCb);
                            readSmsAtCmdState = AT_WAIT_REPLY;
                        }
                    }
                break;
                
                case AT_WAIT_REPLY: 
                break;
                
                case AT_CMD_SUCCESS:
                  if(gsmSmsReadCmdIndex >= 1)
                  {
                    gsmReadSmsFnState = SMS_READING_SUCCESS;
                    if((flagSMSlineReceived) && (GsmReadSmsData.rcvSmsLength < 256)) //if any long sms is received do not process it direcly delete 
                    {
                      flagSMSlineReceived = 0;
                      GsmReceiveSmsCallback(GsmReadSmsData.rcvMobileNo, GsmReadSmsData.rcvContentData, GsmReadSmsData.rcvSmsLength);
                    }  //if +CMGR is recievd then only process the sms ,if only ok is received then dont process it
                    else
                    {
                      //if long sms is received directly delete that sms
                      GsmDeleteSMS();
                    }
                  }
                  else
                  {
                     //(gsmSmsReadCMD_et)gsmSmsReadCmdIndex++;
										 gsmSmsReadCmdIndex++;
                     GsmSmsReadStart();
                     readSmsAtCmdState = AT_CMD_SEND;
                  }
                break;
                
                case AT_CMD_FAILURE:
                    gsmReadSmsFnState = SMS_READING_FAIL;
                break;
                
                case AT_CMD_RETRY_WAIT:
                break;
            }
        break;
        
        case SMS_READING_SUCCESS:  
        break;
        
        case SMS_READING_FAIL:  
        break;
    }
   return  gsmReadSmsFnState;
}


void GsmSmsSetState(gsmSmsState_et state)
{
    if(smsState == state)
    {
        return;
    }
    
    smsState = state;
    
    switch(smsState)
    {
        case GSM_SMS_IDLE:
            // check whether send sms is received
            GsmSetState(GSM_MODEM_IDLE);
        break;
        
        case GSM_SMS_READ:
            GsmSetState(GSM_SMS_PROCESSING);    //for statemachine
            GsmSmsSetTextMode();   //while reading the sms first set it to text mode then then send the read command
            //GsmSmsReadStart();
        break;
        
        case GSM_SMS_SEND:
            GsmSetState(GSM_SMS_PROCESSING);
            GsmSmsSendStart();
        break;
        
        case GSM_SMS_DELETE:
            GsmSetState(GSM_SMS_PROCESSING);
            
            GsmSmsDeleteStart();
        break;
    }
}

void GsmDeleteSMS(void)
{
  flagDelCurrentSms = 1;
}
void GsmSendSMS(void)
{
  flagSendSMSResponse  = 1;
}

void GsmSmsHandler(void)
{
    switch(smsState)
    {
        case GSM_SMS_IDLE:
            // check whether send sms is received
            if(GsmStateIsIdle())
            {
                if(gsmDelSmsFnState == SMS_DELETE_START)
                {
                    GsmSmsSetState(GSM_SMS_DELETE);
                }
                else if(gsmSendSmsFnState == SMS_SEND_START)
                {
                    GsmSmsSetState(GSM_SMS_SEND);
                }
                else if((gsmReadSmsFnState == SMS_READ_START) || (flagCmtiUrcRcvd))
                {
                    if(flagCmtiUrcRcvd)
                    {
                        flagCmtiUrcRcvd =0;
                    }
                    GsmSmsSetState(GSM_SMS_READ);
                } 
            }
        break;
        
        case GSM_SMS_READ:
            GsmReceiveSMSHandler();
            if(gsmReadSmsFnState == SMS_READING_SUCCESS)
            {
                if(flagDelCurrentSms)
                {
                   flagDelCurrentSms = 0;
                   GsmSmsSetState(GSM_SMS_DELETE);
                }
                else if(flagSendSMSResponse)   //If a sms received from the user needs to send the response to the user then just clear the flag
                {
                   flagSendSMSResponse =0;
                   GsmSmsSetState(GSM_SMS_IDLE);
                }
                else
                {
                  GsmSmsSetState(GSM_SMS_IDLE);
                }
            }
            else if(gsmReadSmsFnState == SMS_READING_FAIL)
            {
                GsmSmsSetState(GSM_SMS_IDLE);
            }
        break;
        
        case GSM_SMS_SEND:
            GsmSendSMSHandler();
            if(gsmSendSmsFnState == SMS_SENDING_SUCCESS)
            {
                if(fotaResponse)
                {fotaResponse = 0;}
                GsmSmsSetState(GSM_SMS_DELETE);
            }
            else if(gsmSendSmsFnState == SMS_SENDING_FAIL)
            {
                if(fotaResponse)
                {
                    fotaResponse = 0;
                    GsmSmsSetState(GSM_SMS_DELETE);
                }
                else
                {
                    GsmSmsSetState(GSM_SMS_DELETE);  
                }
            }
        break;
        
        case GSM_SMS_DELETE:
            GsmDeleteSMSHandler();
            if(gsmDelSmsFnState == SMS_DELETE_SUCCESS)
            {
                GsmSmsSetState(GSM_SMS_IDLE);
            }
            else if(gsmDelSmsFnState == SMS_DELETE_FAIL)
            {
                GsmSmsSetState(GSM_SMS_IDLE);
            }
        break;
    }
}

uint8_t GsmGetSendState(void)
{
   if((gsmSendSmsFnState == SMS_SENDING_SUCCESS) || (gsmSendSmsFnState == SMS_SENDING_FAIL))
   {
        return 1 ;
   }
   return 0;
}

void UrcSmsCb(uint8_t *lineBuff, uint16_t len)
{
    char* p1 = NULL;
    char* p2 = NULL;
    
    // Get 'mem' storage where SMS is Stored
    p1 = strstr((const char *)lineBuff, ":");
    p1 += 3;
    p2 = strstr(p1, ","); //+CMTI: "mem",3
    if (p1 && p2)
    {
        //memset(mem, 0x0, sizeof(mem));
        //strncpy(mem, p1, (p2 - p1 - 1));
    }

    // Get index of the stored SMS Index
    p1 = p2;
    p2 = strchr(p1, '\0');
    if (p1 && p2)
    {
        memset(strIndex, 0x0, sizeof(strIndex));
        strncpy(strIndex, p1 + 1, p2 - p1 - 1);
        smsReceiveIndex = atoi(strIndex);                //get the index of the received sms
        flagCmtiUrcRcvd = 1; 
    }
}

void GsmSmsRead(uint8_t smsIndex)
{
    smsReceiveIndex = smsIndex;                //get the index of the received sms
    gsmReadSmsFnState = SMS_READ_START;
}