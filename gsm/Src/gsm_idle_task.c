#include "gsm_idle_task.h"
#include "gsm_socket_api.h"
#include "gsm_utility.h"
#include "gsm_gprs_api.h"
#include "gsm_sms.h"
#include "packet_api.h"
#include <string.h>
#include <stdlib.h>

// the poll status is stored here
gsmPollCmd_et gsmPollCmd = GSM_POLL_RECV_SMS;
// at command state of poll command
gsmCmdState_et gsmPollAtCmdState = AT_CMD_SEND;

static uint8_t OperatorSelection[OPERATOR_SELECTION];
uint8_t flagOperatorNameUpdated = 0;

static uint8_t networkInfo[OPERATOR_SELECTION];

gsmTimePara_t GsmTime;
uint8_t changeConfigFlag = 0;
uint8_t GetDataFirstTime = 1;
uint8_t resetGsmModule = 0;
uint32_t gsmPollGprsStatusTick = 0;

gsmRespCb_et GprsStatusRespCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len);
gsmRespCb_et GsmCellIdRespCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len);
gsmRespCb_et GsmQueryGprsActRespCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len);
gsmRespCb_et GsmListSMSRespCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len);
gsmRespCb_et GsmSignalStrengthRespCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len);
gsmRespCb_et GsmDateTimeRespCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len);
//gsmRespCb_et GsmOperatorRespCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len);
gsmRespCb_et GsmQSPNHandlerCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len);
gsmRespCb_et GsmQueryNwInfoRespCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len);

uint8_t ResetModule = 0;


const gsmPollCmd_st gsmPollCmdList[] = 
{
//    {"AT+QISTATE=0,1\r",GPRS_STATUS_CMD_TIMEOUT,GprsStatusRespCb},
   // {"AT+QENG?\r",GSM_CELL_ID_CMD_TIMEOUT,GsmCellIdRespCb},
//    {"AT+QIACT?\r",GSM_QUERY_GPRS_ACT_CMD_TIMEOUT,GsmQueryGprsActRespCb},

//s
    {"AT+CMGL=\"REC UNREAD\"\r",GSM_SIG_STREN_CMD_TIMEOUT,GsmListSMSRespCb},
    {"AT+CSQ\r",GSM_SIG_STREN_CMD_TIMEOUT,GsmSignalStrengthRespCb},
    {"AT+CCLK?\r",GSM_DATE_TIME_CMD_TIMEOUT,GsmDateTimeRespCb},
    //     //{"AT+COPS?\r",GSM_OPERATOR_NAME_CMD_TIMEOUT,GsmOperatorRespCb},
    {"AT+QSPN\r",GSM_OPERATOR_NAME_CMD_TIMEOUT,GsmQSPNHandlerCb},
    {"AT+QNWINFO\r",GSM_QUERY_NW_INFO_CMD_TIMEOUT,GsmQueryNwInfoRespCb},
//s
    //vodafone 2G +QNWINFO: "EDGE","40420","GSM 900",10
    //jio 4G +QNWINFO: "TDD LTE","405874","LTE BAND 40",38900
};


BOOL Get_Gsm_time(TimePara_t* gsmtime_struct)
{
  if(GsmTime.year >= DEFAULT_YEAR)
  { 
    memset(gsmtime_struct,0,sizeof(TimePara_t));
    memcpy(gsmtime_struct,&GsmTime,sizeof(TimePara_t));
    return True;
  }
  return False;
}

//uint8_t GsmIsTimeToPollGprsStatus(void)
//{
//    if(GetDataFirstTime == 0)       //0
//    {
//      GetDataFirstTime++;
//      return 1;
//    }
//    else if(((GsmSocketIsOpened(socketId)) == 1) && (TimeSpent(gsmPollGprsStatusTick,GSM_POLL_GPRS_STATE_INTERVAL_MS)))
//    {
//        gsmPollGprsStatusTick = GetStartTime();
//        return 1;
//    }
//    return 0;
//}

//uint32_t gsmPollCellIdTick = 0;
//uint8_t GsmIsTimeToPollCellId(void)
//{
//    if(GetDataFirstTime == 1)     //1
//    {
//      GetDataFirstTime++;
//      return 1;
//    } 
//    else if(TimeSpent(gsmPollCellIdTick,GSM_POLL_CELL_ID_INTERVAL_MS))
//    {
//        gsmPollCellIdTick = GetStartTime();
//        return 1;
//    }
//    return 0;
//}
//extern uint8_t socketId;
//uint32_t gsmPollSocStatusTick = 0;
//uint8_t GsmIsTimeToPollSocketStatus(void)
//{
//    if(((GsmSocketIsOpened(socketId)) == 1) && (TimeSpent(gsmPollSocStatusTick,GSM_POLL_GPRS_STATE_INTERVAL_MS)))
//    {
//        gsmPollSocStatusTick = GetStartTime();
//        return 1;
//    }
//}

uint32_t gsmPollSigStrenTick = 0;
uint8_t GsmIsTimeToPollSignalStrength(void)
{
  //static uint8_t GetDataFirstTime =0;
    if(GetDataFirstTime == 1)       //2
    {
      GetDataFirstTime++;
      return 1;
    }
    else if(TimeSpent(gsmPollSigStrenTick,GSM_POLL_SIG_STRENGTH_INTERVAL_MS))
    {
        gsmPollSigStrenTick = GetStartTime();
        return 1;
    }
    return 0;
}


uint32_t gsmPollRecvSMSTick = 0;
uint8_t GsmIsTimeToPollReceivedSMS(void)
{
    if(GetDataFirstTime == 2)     //3
    {
      GetDataFirstTime++;
      return 1;
    }
    else if(TimeSpent(gsmPollRecvSMSTick,GSM_POLL_RECV_SMS_INTERVAL_MS))
    {
        gsmPollRecvSMSTick = GetStartTime();
        return 1;
    }
    return 0;
}


uint32_t gsmPollDateTimeTick = 0;
uint8_t GsmIsTimeToPollNwTime(void)
{
    if(GsmIsGprsRegistered() == 1)
    {
        if(GetDataFirstTime == 3)       //4
        {
            GetDataFirstTime++;
            return 1;
        }
        else if(TimeSpent(gsmPollDateTimeTick,GSM_POLL_DATE_TIME_INTERVAL_MS))
        {
            gsmPollDateTimeTick = GetStartTime();
            return 1;
        }
        return 0;
    }
    return 0;
}

 uint32_t gsmPollOperatorTick = 0;
 uint8_t GsmIsTimeToPollNwOperator(void)
 {
    if(GetDataFirstTime == 4)       //4
    {
      GetDataFirstTime++;
      return 1;
    }
    else if(TimeSpent(gsmPollOperatorTick,GSM_POLL_OPERATOR_INTERVAL_MS))
     {
         gsmPollOperatorTick = GetStartTime();
         return 1;
     }
     return 0;
 }

 uint32_t gsmPollNwInfoTick = 0;
 uint8_t GsmIsTimeToPollNwInfo(void)
 {
    if(GetDataFirstTime == 5)       //5
    {
      GetDataFirstTime++;
      return 1;
    }
    else if(TimeSpent(gsmPollNwInfoTick,GSM_POLL_NW_INFO_INTERVAL_MS))
     {
         gsmPollNwInfoTick = GetStartTime();
         return 1;
     }
     return 0;
 }

// uint32_t gsmPollgprsActTick = 0;
// uint8_t GsmIsTimeToPollGprsActivation(void)
// {
//    if(GetDataFirstTime == 6)       //5
//    {
//      GetDataFirstTime++;
//      return 1;
//    }
//    else if(TimeSpent(gsmPollgprsActTick,GSM_POLL_GPRS_ACT_INTERVAL_MS))
//    {
//        gsmPollgprsActTick = GetStartTime();
//        return 1;
//    }
//    return 0;
// }
 
// idle task handler will check all the events and CHANGES the gsmstate 
void GsmIdleTaskHandler(void)
{
    if(resetGsmModule)
    {
        GsmSetState(GSM_SW_POWER_RESET);
        resetGsmModule = 0;
    }
    // check whether gprs or network registration is available
    else if(changeConfigFlag == 1)
    {
      GsmSetState(GPRS_DEACTIVATE);
      changeConfigFlag = 0;
    }
    // check whether gsm is registered
    else if(GsmIsNetworkRegistered() == 0)
    {
        GsmSetState(CHECK_GSM_NW_STATUS);
    }
   // check whether gprs is registered
    else if(GsmIsGprsRegistered() == 0)
    {
        GsmSetState(CHECK_GPRS_REG_STATUS);
    }
//    else if(GsmIsTimeToPollGprsStatus())
//    {
//        GsmPollStatusSetCmd(GSM_POLL_GPRS_STATUS);
//        GsmSetState(POLL_STATUS);
//    }
//    else if(GsmIsTimeToPollCellId())
//    {
//        GsmPollStatusSetCmd(GSM_POLL_CELL_ID);
//        GsmSetState(POLL_STATUS);
//    }
//    else if(GsmIsTimeToPollSocketStatus())
//    {
//        GsmPollStatusSetCmd(GSM_POLL_SOC_STATUS);
//        GsmSetState(POLL_STATUS);
//    }
    else if(GsmIsTimeToPollReceivedSMS())
    {
        GsmPollStatusSetCmd(GSM_POLL_RECV_SMS);
        GsmSetState(POLL_STATUS);
    }
    // check poll for gsm time stamp
		
		
		//s
		
    else if(GsmIsTimeToPollNwTime())
    {
        GsmPollStatusSetCmd(GSM_POLL_NW_TIME);
        GsmSetState(POLL_STATUS);
    }
    else if(GsmIsTimeToPollSignalStrength())
    {
        GsmPollStatusSetCmd(GSM_POLL_SIGNAL_STRENGTH);
        GsmSetState(POLL_STATUS);
    }
//    else if(GsmIsTimeToPollNwOperator())
//    {
//        GsmPollStatusSetCmd(GSM_POLL_GET_OPERATOR_NAME);
//        GsmSetState(POLL_STATUS);
//    }
//    else if(GsmIsTimeToPollNwInfo())
//    {
//        GsmPollStatusSetCmd(GSM_POLL_NW_INFO);
//        GsmSetState(POLL_STATUS);
//    }
		//s
		
		
		
		
    else if(GsmGprsIsActive() == 0)
    {
        GsmSetState(GPRS_DEACTIVATE);
    }
}

uint8_t flagGprsStatusReceiveComplete = 0;

void GsmPollStatusSetCmd(gsmPollCmd_et cmd)
{   
    gsmPollCmd = cmd;
    
    switch(gsmPollCmd)
    {
//        case GSM_POLL_GPRS_ACT:
//            gsmPollAtCmdState = AT_CMD_SEND;
//        break;
        
        case GSM_POLL_RECV_SMS:
            gsmPollAtCmdState = AT_CMD_SEND;
        break;
        
//        case GSM_POLL_CELL_ID:
//            ProcessQENGStart();
//            gsmPollAtCmdState = AT_CMD_SEND;
//        break;
        
        case GSM_POLL_SIGNAL_STRENGTH:
            gsmPollAtCmdState = AT_CMD_SEND;
        break;
        
        case GSM_POLL_NW_TIME:
            gsmPollAtCmdState = AT_CMD_SEND;
        break;
        
        case GSM_POLL_GET_OPERATOR_NAME:
            gsmPollAtCmdState = AT_CMD_SEND;
        break;
        
        case GSM_POLL_NW_INFO:
            gsmPollAtCmdState = AT_CMD_SEND;
        break; 

        case GSM_NUM_POLL_CMD:
				
				break;
				
    }
}

void SetPowerResetModemFlag(void)
{
    resetGsmModule = 1;
}

void DeactGprsWhenIdle(void)
{
  changeConfigFlag = 1;
}

gsmCmdState_et GsmPollStatusHandler(void)
{
    switch(gsmPollAtCmdState)
    {
        case AT_CMD_SEND:
            if(GsmIsAtIdle())
            {
                GsmSendAtCmd(gsmPollCmdList[gsmPollCmd].cmd,strlen((const char *)gsmPollCmdList[gsmPollCmd].cmd),
                gsmPollCmdList[gsmPollCmd].cmdTimeoutInMs,gsmPollCmdList[gsmPollCmd].respCb);
                gsmPollAtCmdState = AT_WAIT_REPLY;
            }
        break;
        
        case AT_WAIT_REPLY:
        break;
        
        case AT_CMD_SUCCESS:
            // set the gsm state to idle
            LOG_DBG(CH_POLL,"cmd no. %d success",gsmPollCmd);
        break;
        
        case AT_CMD_FAILURE:
            // set the gsm state to idle
            LOG_ERR(CH_POLL,"cmd no. %d failure",gsmPollCmd);
       break;
        
        case AT_CMD_RETRY_WAIT:
        break;
    }
    return gsmPollAtCmdState;
}

void ProcessQISTATEData(uint8_t *SocStateLinedata ,uint8_t socStateLinelen)
{
    //char* p1 = NULL;
    //char* p2 = NULL;
    //uint8_t len = 0;// ucTemp =0;
  
    if((strncmp((char*)SocStateLinedata, (char *)"+QISTATE: 0,", 12) == 0)) 
    {
        // memset(gprsStr, 0x0, sizeof(gprsStr));
        // sprintf((char *)gprsStr, "+QISTATE:");
        // if (GSMStrPrefixMatch((char *)SocStateLinedata,(char*)gprsStr))
        // {
           // p1 = strstr((char *)SocStateLinedata,"+QISTATE:");
           // p1 += len;
           // p2 = strchr(p1,',');
           // if(p1 && p2)
           // {
          // }
       
        // }
    }

    else if((strncmp((char*)SocStateLinedata, (char *)"+QISTATE: 1,", 12) == 0))
    {
        // memset(gprsStr, 0x0, sizeof(gprsStr));
         // sprintf((char *)gprsStr, "+QISTATE:");
        // if (GSMStrPrefixMatch((char *)SocStateLinedata,(char*)gprsStr))
        // {
           // p1 = strstr((char *)SocStateLinedata,"+QISTATE:");
           // p1 += len;
           // p2 = strchr(p1,',');
           // if(p1 && p2)
           // {
           // }
       
        // }
    }
    else if(strncmp((char*)SocStateLinedata, (char *)"+QISTATE: 2,", 12) == 0)
    {
        
    }
    else if(strncmp((char*)SocStateLinedata, (char *)"+QISTATE: 3,", 12) == 0)
    {
        
    }
    else if(strncmp((char*)SocStateLinedata, (char *)"+QISTATE: 4,", 12) == 0)
    {
        
    }
    else if(strncmp((char*)SocStateLinedata, (char *)"+QISTATE: 5,", 12) == 0)
    {
        flagGprsStatusReceiveComplete  = 1;
    }
}

//static gprsState_et gprsPollStatus;
//
//gsmRespCb_et GprsStatusRespCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len)
//{
//    gsmRespCb_et gsmRespRetVal = GSM_RESP_CB_WAIT;
//   
//    switch(gsmResp)
//    {
//        case RESP_OK:
//            if(flagGprsStatusReceiveComplete == 0)
//            {
//                gsmRespRetVal = GSM_RESP_CB_WAIT;
//            }
////            else
////            {
////                gsmRespRetVal = GSM_RESP_CB_OK_COMPLETE;
////                if(gprsPollStatus == GPRS_STATUS || gprsPollStatus == GPRS_IP_PROCESSING)
////                {
////                    GsmSetState(GSM_MODEM_IDLE);
////                    gsmPollAtCmdState = AT_CMD_SUCCESS;
////                }
////                else
////                {
////                    // send failure
////                    GsmSetState(CHECK_GPRS_REG_STATUS);
////                    gsmPollAtCmdState = AT_CMD_SUCCESS;
////                }
////            }
//        break;
//         
//        case RESP_IP_INITIAL:
//            gprsPollStatus = GPRS_INITIAL;
//            gsmRespRetVal = GSM_RESP_CB_OK_COMPLETE;   //GSM_RESP_CB_OK_COMPLETE
//            gsmPollAtCmdState = AT_CMD_SUCCESS;
//            GsmSetState(GSM_MODEM_IDLE);
//        break;
//          
//        case RESP_IP_START:
//            gprsPollStatus = GPRS_STARTED;
//            gsmRespRetVal = GSM_RESP_CB_OK_COMPLETE;//GSM_RESP_CB_WAIT;
//            gsmPollAtCmdState = AT_CMD_SUCCESS;
//            GsmSetState(GSM_MODEM_IDLE);
//        break;
//         
//        case RESP_IP_CONFIG:
//            gprsPollStatus = GPRS_CONFIG;
//            gsmRespRetVal = GSM_RESP_CB_OK_COMPLETE;//GSM_RESP_CB_WAIT;
//            gsmPollAtCmdState = AT_CMD_SUCCESS;
//            GsmSetState(GSM_MODEM_IDLE);
//        break;
//          
//        case RESP_IP_GPRSACT:
//            gprsPollStatus = GPRS_ACTIVE;
//            gsmRespRetVal = GSM_RESP_CB_OK_COMPLETE;//GSM_RESP_CB_WAIT;
//            gsmPollAtCmdState = AT_CMD_SUCCESS;
//            GsmSetState(GSM_MODEM_IDLE);
//        break;
//        
//        case RESP_IP_STATUS:
//            gprsPollStatus = GPRS_STATUS;
//            gsmRespRetVal = GSM_RESP_CB_OK_COMPLETE;//GSM_RESP_CB_WAIT;
//            gsmPollAtCmdState = AT_CMD_SUCCESS;
//            GsmSetState(GSM_MODEM_IDLE);
//        break;
//          
////        case RESP_IP_PROCESSING:
////            gprsPollStatus = GPRS_IP_PROCESSING;
////            gsmRespRetVal = GSM_RESP_CB_OK_COMPLETE;//GSM_RESP_CB_WAIT;
////            gsmPollAtCmdState = AT_CMD_SUCCESS;
////            GsmSetState(GSM_MODEM_IDLE);
////        break;
//                
//        case RESP_SCONNECT_OK:
//            gprsPollStatus = TCP_CONNECT_OK;
//            flagGprsStatusReceiveComplete  = 1;
//            gsmRespRetVal = GSM_RESP_CB_OK_COMPLETE;//GSM_RESP_CB_WAIT;
//            gsmPollAtCmdState = AT_CMD_SUCCESS;
//            GsmSetState(GSM_MODEM_IDLE);
//        break;
//        
//        case RESP_IP_IND:
//            gprsPollStatus = GPRS_IND;
//            gsmRespRetVal = GSM_RESP_CB_OK_COMPLETE;//GSM_RESP_CB_WAIT;    //GSM_RESP_CB_ERROR_COMPLETE
//            gsmPollAtCmdState = AT_CMD_SUCCESS;
//            GsmSetState(GSM_MODEM_IDLE);
//        break;
//       
//        case RESP_PDP_DEACT:
//            gprsPollStatus = GPRS_DEACTIVE;
//            gsmRespRetVal = GSM_RESP_CB_OK_COMPLETE;//GSM_RESP_CB_WAIT;
//            gsmPollAtCmdState = AT_CMD_SUCCESS;
//            GsmSetState(GSM_MODEM_IDLE);
//        break;
//      
//        case RESP_SOCK_STATE:
//            ProcessQISTATEData(lineBuff,len);
//            gsmRespRetVal = GSM_RESP_CB_WAIT;
//        break;
//        
//      
//        case RESP_ERROR:
//        case RESP_TIMEOUT:
//            LOG_ERR(CH_POLL,"QISTATE response errocode : %d ",gsmResp);
//            gsmRespRetVal = GSM_RESP_CB_ERROR_COMPLETE;
//            gsmPollAtCmdState = AT_CMD_FAILURE;
//            GsmSetState(CHECK_GPRS_REG_STATUS);
//        break;
//        
//        case RESP_IGNORE:
//        case RESP_OTHER: 
//            gsmRespRetVal = GSM_RESP_CB_WAIT;  
//        break;
//      
//        default:
//        break;    
//    }
//   return gsmRespRetVal;
//}

gsmRespCb_et GsmCellIdRespCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len)
{
    gsmRespCb_et gsmRespRetVal = GSM_RESP_CB_WAIT;
    switch(gsmResp)
    {
        //lines reading and processing done complete the timeout
        case RESP_OK: 
            if(ProcessQENGResultGet())
            {
                gsmRespRetVal = GSM_RESP_CB_OK_COMPLETE;
                gsmPollAtCmdState = AT_CMD_SUCCESS;
                GsmSetState(GSM_MODEM_IDLE);
            }
        break;
        
        case RESP_ERROR:    
        case RESP_CME_ERROR:
        case RESP_TIMEOUT:
            gsmPollAtCmdState = AT_CMD_FAILURE;
            GsmSetState(CHECK_GSM_NW_STATUS);
            gsmRespRetVal = GSM_RESP_CB_ERROR_COMPLETE;
        break;
        
        case RESP_CELLID_DATA:
            ProcessQENGData(lineBuff,len);   
            //wait as we are reading the lines and processing
            gsmRespRetVal = GSM_RESP_CB_WAIT;
        break;
    }
    return gsmRespRetVal;
}
gsmRespCb_et GsmQueryGprsActRespCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len)
{
  gsmRespCb_et gsmRespRetVal = GSM_RESP_CB_WAIT;
    
    switch(gsmResp)
    {
        case  RESP_OK:
           gsmRespRetVal      = GSM_RESP_CB_OK_COMPLETE;                        //messgae sent and ok received
           gsmPollAtCmdState  = AT_CMD_SUCCESS;
           GsmSetState(GSM_MODEM_IDLE);
           LOG_DBGS(CH_SMS,"list CB success");
        break;
        
        case RESP_ERROR:  
        case RESP_CMS_ERROR:
        case RESP_TIMEOUT:
          gsmRespRetVal      = GSM_RESP_CB_ERROR_COMPLETE;
          gsmPollAtCmdState  = AT_CMD_FAILURE;
          GsmSetState(CHECK_GPRS_REG_STATUS);
          break;
          
        case RESP_QUERY_GPRS_ACT:  
          break;
    }
    return gsmRespRetVal;
}

uint8_t newSMSIndex = 0;
uint8_t smsErrCount = 0;

gsmRespCb_et GsmListSMSRespCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len)
{
  gsmRespCb_et gsmRespRetVal = GSM_RESP_CB_WAIT;
    
    switch(gsmResp)
    {
        case  RESP_OK:
           gsmRespRetVal      = GSM_RESP_CB_OK_COMPLETE;                        //message sent and ok received
           gsmPollAtCmdState  = AT_CMD_SUCCESS;
           GsmSetState(GSM_MODEM_IDLE);
           LOG_DBGS(CH_SMS,"list CB success");
        break;
        
        case RESP_ERROR:  
        case RESP_CMS_ERROR:
        case RESP_TIMEOUT:
            gsmRespRetVal      = GSM_RESP_CB_ERROR_COMPLETE;
            gsmPollAtCmdState  = AT_CMD_FAILURE;
            GsmSetState(CHECK_GSM_NW_STATUS);
            smsErrCount++;
            LOG_ERR(CH_SMS,"list CB failure %d, smsErrCount = %d",gsmResp, smsErrCount);
            if(smsErrCount == 3)
            {
              smsErrCount = 0;
              NVIC_SystemReset();
            }
        break;
        
        case  RESP_LIST_SMS:                                                    //+CMGR:
            newSMSIndex = GsmGetSMSIndex(lineBuff,len); 
            //read new sms
            GsmSmsRead(newSMSIndex);
            gsmRespRetVal = GSM_RESP_CB_WAIT; 
        break; 
        
        case RESP_IGNORE:
        case RESP_OTHER:                                                        //here get the data 
//            strcpy((char*)GsmReadSmsData.rcvContentData ,(char*)lineBuff);
//            GsmReadSmsData.rcvSmsLength = len;
            gsmRespRetVal = GSM_RESP_CB_WAIT;
            //LOG_INFO(CH_SMS,"list CB msg : %s",lineBuff);
            //LOG_INFO(CH_SMS,"msgLen : %d",len);
        break;
        
        default:
        break;
    }
    return gsmRespRetVal;
  
}

gsmRespCb_et GsmSignalStrengthRespCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len)
{
    gsmRespCb_et gsmRespRetVal = GSM_RESP_CB_WAIT;
    switch(gsmResp)
    {
        //lines reading and processing done complete the timeout
        case  RESP_OK:        
            gsmRespRetVal = GSM_RESP_CB_OK_COMPLETE;
            gsmPollAtCmdState = AT_CMD_SUCCESS;
            GsmSetState(GSM_MODEM_IDLE);
        break;
        
        case RESP_ERROR:    //if received "ERROR 
        case RESP_CME_ERROR:
        case RESP_TIMEOUT:
            gsmRespRetVal = GSM_RESP_CB_ERROR_COMPLETE;
            gsmPollAtCmdState = AT_CMD_FAILURE;
            GsmSetState(CHECK_GSM_NW_STATUS);
        break;
        
        
        case RESP_SIGNAL_DATA:
            ProcessCSQData(lineBuff,len);
            //wait as we are reading the lines and processing
            gsmRespRetVal = GSM_RESP_CB_WAIT;
        break;
    }
    return gsmRespRetVal;
}


static char tmpbuf[30];
/**
*  @brief  :This gsmDateTimeCb() callback for the gsm date and time expecting +CCLK
 *   @Inputs:  gsmResp of type enum  gsmCmdResponseCode_et
 *             lineBuff - Pointer to the buffer in which modem response(processlinebuffer - Extended Response or SerialLineBuff)is received.
 *             len      - Length of the data received.
 *  @param     [out] gsmRespCb_et.
 *  @return    1 value of enum @ref gsmRespCb_et        
 *             :RESP_OK --> GSM_RESP_CB_COMPLETE if OK received or 
 *             :RESP_ERROR --> GSM_RESP_CB_ERROR_COMPLETE if ERROR is received.
*              :RESP_CME_ERROR --> GSM_RESP_CB_ERROR_COMPLETE if cme error is received.
*              :RESP_GSM_TIME --> GSM_RESP_CB_WAIT when CCLK prefix is received wait to receive OK.
*/
extern uint8_t flagRtcTimeUpdate;
gsmRespCb_et GsmDateTimeRespCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len)
{
    gsmRespCb_et gsmRespRetVal = GSM_RESP_CB_WAIT;
    static uint8_t tempYear = 0;
     
    switch(gsmResp)
    {
        case  RESP_OK:        
            gsmRespRetVal = GSM_RESP_CB_OK_COMPLETE;
            gsmPollAtCmdState = AT_CMD_SUCCESS;
            GsmSetState(GSM_MODEM_IDLE);
        break;
        
        case RESP_ERROR:   
        case RESP_CME_ERROR:
        case RESP_TIMEOUT:
            gsmRespRetVal = GSM_RESP_CB_ERROR_COMPLETE;
            gsmPollAtCmdState = AT_CMD_FAILURE;
            GsmSetState(CHECK_GSM_NW_STATUS);
        break;

        case RESP_IGNORE:
          gsmRespRetVal = GSM_RESP_CB_WAIT;
        break;
        
        case RESP_GSM_TIME:
            if(lineBuff[8]!= 0x22) //ascii "
            {
                memset(tmpbuf,0,sizeof(tmpbuf));
                strncpy(tmpbuf, (char*)lineBuff+8, 2);
                //year check is included bcoz when gsm is not registered, cclk cmd sometimes give below mentioned reply 
                //+CCLK: "80/01/06,00:00:14+22" which is wrong for current year
                tempYear = (uint8_t)atoi(tmpbuf);
                if(tempYear == 80)
                  GsmTime.year = 0;
                else
                  GsmTime.year =  (uint8_t)atoi(tmpbuf);
                strncpy(tmpbuf, (char*)lineBuff+11, 2);
                GsmTime.month = (uint8_t)atoi(tmpbuf);
                strncpy(tmpbuf, (char*)lineBuff+14, 2);
                GsmTime.day = (uint8_t)atoi(tmpbuf);
                strncpy(tmpbuf, (char*)lineBuff+17, 2);
                GsmTime.hour = (uint8_t)atoi(tmpbuf);
                strncpy(tmpbuf, (char*)lineBuff+20, 2);
                GsmTime.minute = (uint8_t)atoi(tmpbuf);
                strncpy(tmpbuf, (char*)lineBuff+23, 2);
                GsmTime.second = (uint8_t)atoi(tmpbuf);
                if((GsmTime.year >= CURRENT_DEFAULT_YEAR) && (GsmTime.year != 70) && (GsmTime.year != 0))
								{
                  flagRtcTimeUpdate = 2;
								}
                else
								{
                  flagRtcTimeUpdate = 0;
								}
            }
            //LOG_DBG(CH_POLL,"GSM Time:- %d:%d:%d", (int)GsmTime.Hours, (int)GsmTime.Minutes, (int)GsmTime.Seconds);
            gsmRespRetVal = GSM_RESP_CB_WAIT;  
        break;
        
        default:
        break;
    }
    return gsmRespRetVal;
}

gsmRespCb_et GsmQSPNHandlerCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len)
{
    gsmRespCb_et gsmRespRetVal = GSM_RESP_CB_WAIT;
    char *p = NULL;
    char *q = NULL; 
    char *r = NULL;
    
    switch(gsmResp)
    {
        case  RESP_OK: 
            gsmRespRetVal = GSM_RESP_CB_OK_COMPLETE;
            gsmPollAtCmdState = AT_CMD_SUCCESS;
            GsmSetState(GSM_MODEM_IDLE);
        break;
        
        case RESP_ERROR:   
        case RESP_CME_ERROR:
        case RESP_TIMEOUT:
            gsmRespRetVal = GSM_RESP_CB_ERROR_COMPLETE;
            gsmPollAtCmdState = AT_CMD_FAILURE;
        break;
           
        case RESP_IGNORE:
            gsmRespRetVal = GSM_RESP_CB_WAIT;
        break;
        
        case RESP_SERVICE_PROVIDER_NAME:    
            memset(OperatorSelection,0x00,sizeof(OperatorSelection));
            p = (char *)lineBuff + 6;
            p++;
            q = strchr(p,'"');
            q++;
            r = strchr(q,'"');
            if(q && r)
            {
                strncpy((char*)OperatorSelection,q,r-q);
                flagOperatorNameUpdated = 1;
            }
            gsmRespRetVal = GSM_RESP_CB_WAIT;   
        break;
          
        default:
        break;
    }
    return gsmRespRetVal;
}

//gsmRespCb_et GsmOperatorRespCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len)
//{
//    gsmRespCb_et gsmRespRetVal = GSM_RESP_CB_WAIT;
//     
//    switch(gsmResp)
//    {
//        case RESP_OK:        
//            gsmRespRetVal = GSM_RESP_CB_OK_COMPLETE;
//            gsmPollAtCmdState = AT_CMD_SUCCESS;
//            GsmSetState(GSM_MODEM_IDLE);
//        break;
//        
//        case RESP_ERROR:   
//        case RESP_CME_ERROR:
//        case RESP_TIMEOUT:
//            gsmRespRetVal = GSM_RESP_CB_ERROR_COMPLETE;
//            gsmPollAtCmdState = AT_CMD_FAILURE;
//            GsmSetState(CHECK_GSM_NW_STATUS);
//        break;
//
//        case RESP_IGNORE:
//          gsmRespRetVal = GSM_RESP_CB_WAIT;
//        break;
//        
//        case RESP_OPS_DATA:
//          gsmRespRetVal = GSM_RESP_CB_WAIT;  
//          break;
//       
//        default:
//        break;
//    }
//    return gsmRespRetVal;
//}

gsmRespCb_et GsmQueryNwInfoRespCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len)
{
    gsmRespCb_et gsmRespRetVal = GSM_RESP_CB_WAIT;
    char *p = NULL;
    char *q = NULL; 
    char *r = NULL;
    
    switch(gsmResp)
    {
        case RESP_OK:        
            gsmRespRetVal = GSM_RESP_CB_OK_COMPLETE;
            gsmPollAtCmdState = AT_CMD_SUCCESS;
            GsmSetState(GSM_MODEM_IDLE);
        break;
        
        case RESP_ERROR:   
        case RESP_CME_ERROR:
        case RESP_TIMEOUT:
            gsmRespRetVal = GSM_RESP_CB_ERROR_COMPLETE;
            gsmPollAtCmdState = AT_CMD_FAILURE;
            GsmSetState(CHECK_GSM_NW_STATUS);
        break;

        case RESP_IGNORE:
          gsmRespRetVal = GSM_RESP_CB_WAIT;
        break;
        
        case RESP_NW_INFORMATION:
          memset(networkInfo,0,sizeof(networkInfo));
          p = (char *)lineBuff + 6;
          p++;
          q = strchr(p,'"');
          q++;
          r = strchr(q,'"');
          if(q && r)
          {
            strncpy((char*)networkInfo,q,r-q);
          }
          gsmRespRetVal = GSM_RESP_CB_WAIT;  
          break;
       
        default:
        break;
    }
    return gsmRespRetVal;
}

void GetGsmTime(gsmTimePara_t* gsmTimeParaPtr)
{ 
    memset(gsmTimeParaPtr,0,sizeof(gsmTimePara_t));
    memcpy(gsmTimeParaPtr, &GsmTime, sizeof(gsmTimePara_t));
}

uint8_t* GetOperatorName(void)
{
    return(OperatorSelection);
}

uint8_t* GetNetworkInfoName(void)
{
  return (networkInfo);
}
	

void GetOperatorNameInit(uint8_t* opsDataArray)
{
    if(flagOperatorNameUpdated == 1)
    {
       strncpy((char*)opsDataArray,(char*)networkInfo,OPERATOR_NAME_SIM_SIZE);
    }
    else
    {
      snprintf((char*)opsDataArray,sizeof(opsDataArray),"%s","0");
    }
//  return(OperatorNameFrmSIM);
}

void UpdateGprsConfig(void)
{
  changeConfigFlag = 1;
}