#include <string.h>
#include <stdlib.h>
#include "gsm_statemachine.h"

#include "gsm_common.h"
#include "gsm_gprs_api.h"
#include "gsm_ssl_config.h"
#include "gsm_utility.h"
#include "gsm_socket_api.h"
#include "gsm_sms.h"
#include "gsm_idle_task.h"
#include "gsm_ftp_api.h"
#include "packet_api.h"

static gsmState_et gsmState = GSM_NO_INIT;
gsmCmdState_et gsmNwCheckAtCmdState = AT_CMD_SEND;
gsmCmdState_et  gsmRssiAtCmdState = AT_CMD_SEND;

uint8_t gsmNwRegStateCheck = 0;
uint8_t gsmRssiFlag  = 0;
static uint8_t flagNetworkRegistered = 0;
static uint8_t flagGprsNetworkRegisterd = 0;

uint8_t gprsActFailCount = 0;
static const char GSMQueryGSMNWRegistration[] ="AT+CREG?\r";
static const char GSMQueryGPRSNWRegistration[] ="AT+CGREG?\r";
static const char *gsmRegQuery = GSMQueryGSMNWRegistration;
static const char GsmGetRSSICmd[] = "AT+CSQ\r";
static const char *gsmRssiQuery = GsmGetRSSICmd;
//static const uint8_t GSMQueryTime[] = "AT+CCLK?\r";

//gsm modem software shutdown command
static const char GSMModemSofShutDown[] ="AT+QPOWD=1\r";

//at cmd state for gsm modem soft shutdown
gsmCmdState_et  gsmSoftShutState = AT_CMD_SEND;
static uint8_t strTmp[50];

// NETWORK check registration variables
uint32_t gsmNwCheckTick = 0;
gsmNetworkState_et networkState = NETWORK_STAT_NOT_REGISTERED;
gsmNetworkState_et gprsState = NETWORK_STAT_NOT_REGISTERED;
uint8_t gsmNwNumRetry = 0;
uint8_t gprsActFailCnt = 0;

uint32_t gprsActTimeout  = 0, gprsDeactTimeout = 0;
uint32_t gsmSimNotPresentTick = 0;
uint32_t unknownStateStartTick = 0;

extern uint8_t gprsActivated;
extern uint8_t  gsmGprsDeact;
extern uint8_t gprsConfigFailCnt;
uint32_t oneSecTick;
 uint8_t numericOpsCode[50];													  
gsmCmdState_et  gsmCopsAtCmdState = AT_CMD_SEND;

static const uint8_t GsmGetCopsCmd[] = "AT+COPS?\r";
static const uint8_t *gsmCopsQuery = GsmGetCopsCmd;

void GsmUpdateNetworkRegistration(uint8_t flagRegistered, gsmNetworkState_et nwState)
{
    flagNetworkRegistered = flagRegistered;
    networkState = nwState;
    gprsActivated = 0;
    gsmGprsDeact = 1;
    LOG_DBG(CH_GSM,"Network State : %d",networkState);
}

void GsmUpdateGprsRegistration(uint8_t flagRegistered,gsmNetworkState_et nwState)
{
    flagGprsNetworkRegisterd = flagRegistered;
    gprsState = nwState;
    gprsActivated = 0;
    gsmGprsDeact = 1;
    LOG_DBG(CH_GSM,"GPRS State : %d",gprsState);
}

uint8_t GsmIsNetworkRegistered(void)
{
    return flagNetworkRegistered;
}

uint8_t GsmIsGprsRegistered(void)
{
    return flagGprsNetworkRegisterd;
}

void GsmStateMachineInit(void)
{
    GsmSetState(GSM_HW_POWER_RESET);
    flagNetworkRegistered = 0;
    flagGprsNetworkRegisterd = 0;
}

/**
*  @brief  :This GsmQueryOpsCb() callback for the query manual gsm registaration commands expecting +COPS , OK , ERROR
*  @Inputs:  gsmResp of type enum  gsmResPonse_et
*             lineBuff - Pointer to the buffer in which modem response(processlinebuffer - Extended Response or SerialLineBuff)is received.
*             len      - Length of the data received.
*  AT+COPS=?
*  +COPS: (2,"Vodafone - Mumbai","Vodafone","40420"),(1,"Mahanagar Telephone Nigam","DOLPHIN","40469"),
*  (1,"Tata TeleServices(Maharashtra) Limited","TATA DO","405039"),(1,"Idea Cellular Ltd","IDEA","405799"),(1,"airtel","airtel","40492"),,(0-4),(0-2)
*  OK
*  @param     [out] gsmRespCb_et.
*  @return    1 value of enum @ref gsmRespCb_et        
*             :RESP_OK --> GSM_RESP_CB_COMPLETE if OK received or 
*             :RESP_ERROR --> GSM_RESP_CB_ERROR_COMPLETE if ERROR is received.
*             :RESP_CME_ERROR --> GSM_RESP_CB_ERROR_COMPLETE if cme error is received.
*             :RESP_OPS_DATA --> GSM_RESP_CB_WAIT when COPS prefix is received wait to receive OK.
*/

gsmRespCb_et    GsmQueryOpsCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len)
{
    uint8_t *cBuffAdd,*cBuffAdd1 ,*start= NULL;   
   //what we want to return from the calllback depending on the reponse received
    gsmRespCb_et gsmRespRetVal = GSM_RESP_CB_WAIT;
    
    switch(gsmResp)
    {
        case RESP_OK: 
            //  the resp ret val is used by the call back function 
            gsmRespRetVal  = GSM_RESP_CB_OK_COMPLETE;
            gsmCopsAtCmdState = AT_CMD_SUCCESS;
        break;
        
        case RESP_ERROR:
        case RESP_CME_ERROR:
        case RESP_TIMEOUT:
            //LOG_ERR(CH_GPRS,"Ops Search cmd :%d errcode :%d",pollOpsCmdIndex,gsmResp);
            gsmRespRetVal   = GSM_RESP_CB_ERROR_COMPLETE;   
            gsmCopsAtCmdState = AT_CMD_FAILURE;
        break;
        
        case RESP_IGNORE:
        case RESP_OTHER: 
            gsmRespRetVal = GSM_RESP_CB_WAIT;    //+QNITZ/+CCLK URC is rcvd then ignore it.
        break;
        
        case RESP_OPS_DATA:
         // +COPS: (2,"Vodafone - Mumbai","Vodafone","40420"),(1,"Mahanagar Telephone Nigam","DOLPHIN","40469"),
          //(1,"Tata TeleServices(Maharashtra) Limited","TATA DO","405039"),(1,"Idea Cellular Ltd","IDEA","405799"),(1,"airtel","airtel","40492"),,(0-4),(0-2)
         // OK
          memset(numericOpsCode,0,sizeof(numericOpsCode));
          cBuffAdd = pucUTL_eSearchChar_Exe (',', 2, &lineBuff[0], &lineBuff[len]);   //extract the firts nw operator's numeric code
          if(cBuffAdd != 0x00)
          {
            cBuffAdd++;
            if(*cBuffAdd == '"')
            {
              cBuffAdd++;
              cBuffAdd1 = pucUTL_eSearchChar_Exe ('"',1, cBuffAdd, &lineBuff[len]);
              memcpy(numericOpsCode, cBuffAdd, cBuffAdd1 - cBuffAdd);
              
              if((start = (uint8_t*)strstr((const char*) numericOpsCode,"Vi India")) != NULL)
              {
                 //if it is vodafone card then set the apn as www
                 UpdateSimApn("qdnet.vodafone.in",17);     //"qdnet.vodafone.in"             
              }
              else if((start = (uint8_t*)strstr((const char*) numericOpsCode,"airtel")) != NULL)
              {
                //if it is airtel card then set the apn as airtelgprs.com
                UpdateSimApn("airtelgprs.com",14);
              }
              else if((start = (uint8_t*)strstr((const char*) numericOpsCode,"JIO")) != NULL)
              {
                //if it is a jio card then set the apn as jionet
                UpdateSimApn("jionet",6);
              }
              else
              {
                //else use the apn which is set into the memory
                
              }
                
            }
          }
          gsmRespRetVal = GSM_RESP_CB_WAIT;
         break;
        
        default:
        break;
    }
    return gsmRespRetVal;
}

/**
*  @brief  :This GsmCREGHandlerCb() callback for the query gsm registaration commands expecting +CREG: OK or ERROR.
*  @Inputs:  gsmResp of type enum  gsmResPonse_et
*             lineBuff - Pointer to the buffer in which modem response(processlinebuffer - Extended Response or SerialLineBuff)is received.
*             len      - Length of the data received.
*  @param     [out] gsmRespCb_et.
*  @return    1 value of enum @ref gsmRespCb_et        
*             :RESP_OK --> GSM_RESP_CB_COMPLETE if OK received or 
*             :RESP_ERROR --> GSM_RESP_CB_ERROR_COMPLETE if ERROR is received.
*             :RESP_CME_ERROR --> GSM_RESP_CB_ERROR_COMPLETE if cme error is received.
*             :RESP_GSM_NW --> GSM_RESP_CB_WAIT when CREG prefix is received wait to receive OK.
*/
gsmRespCb_et  GsmCREGHandlerCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len)
{
    gsmRespCb_et gsmRespRetVal = GSM_RESP_CB_WAIT;
    char* p1 = NULL;
    char* p2 = NULL;
    switch(gsmResp)
    {
        case  RESP_OK:        
            gsmRespRetVal = GSM_RESP_CB_OK_COMPLETE;
            // check whether it is gsm network registration or gprs registration
            if(gsmRegQuery == GSMQueryGSMNWRegistration)
            {
                if(networkState == NETWORK_STAT_REGISTERED || networkState == NETWORK_STAT_REGISTERED_ROAMING)
                {
                    flagNetworkRegistered = 1;
                    gsmNwRegStateCheck    = 1;
                    gsmNwCheckAtCmdState = AT_CMD_SUCCESS;
                }
                else
                {
                    flagNetworkRegistered = 0;
                    gsmNwCheckAtCmdState = AT_CMD_FAILURE;
                }
            }
            else
            {
                if(gprsState == NETWORK_STAT_REGISTERED || gprsState == NETWORK_STAT_REGISTERED_ROAMING)
                {
                    flagGprsNetworkRegisterd = 1;
                    gsmNwCheckAtCmdState = AT_CMD_SUCCESS;
                }
                else
                {
                    flagGprsNetworkRegisterd = 0;
                    gsmNwCheckAtCmdState = AT_CMD_FAILURE;
                }
            }
        break;
        
        case RESP_ERROR:   
        case RESP_CME_ERROR:
        case RESP_TIMEOUT:
            gsmRespRetVal = GSM_RESP_CB_ERROR_COMPLETE;
            gsmNwCheckAtCmdState = AT_CMD_FAILURE;
        break;
           
        case RESP_IGNORE:
        case RESP_OTHER:
            gsmRespRetVal = GSM_RESP_CB_WAIT;
        break;
        
        case RESP_GSM_NW:
            p1 = (char *)lineBuff + 7;
            p2 = strchr(p1,',');  
            if (p1 && p2)
            {
                p2++;
                memset(strTmp, 0x0, sizeof(strTmp));
                memcpy(strTmp, p2, p2 - p1);
                networkState = (gsmNetworkState_et)atoi((const char *)strTmp);
            }
            gsmRespRetVal = GSM_RESP_CB_WAIT;  
        break;
        
        case RESP_GPRS_NW:
            p1 = (char *)lineBuff + 8;
            p2 = strchr(p1,',');  
            if (p1 && p2)
            {
                p2++;
                memset(strTmp, 0x0, sizeof(strTmp));
                memcpy(strTmp, p2, p2 - p1);
                gprsState = (gsmNetworkState_et)atoi((const char *)strTmp);
            }
            gsmRespRetVal = GSM_RESP_CB_WAIT;
        break;
        
        default:
        break;
    }
    return gsmRespRetVal;
}

void GsmCheckRegistrationHandler(void)
{
    switch(gsmNwCheckAtCmdState)
    {
        case AT_CMD_SEND:
            // check whether is at command is idle
            if(GsmIsAtIdle())
            {
                // send the at command
                GsmSendAtCmd((char *)gsmRegQuery,strlen((const char *)gsmRegQuery),GSM_NW_REG_CHECK_CMD_TIMEOUT_MS,GsmCREGHandlerCb);
                gsmNwCheckAtCmdState = AT_WAIT_REPLY;
            }
        break;
        
        case AT_WAIT_REPLY:
            // the callback function will change the state of the initAtState
        break;
        
        case AT_CMD_SUCCESS:
            
        break;
        
        case AT_CMD_FAILURE:
            gsmNwNumRetry++;
            if(gsmNwNumRetry >= GSM_NW_REG_CHECK_NUM_RETRY)
            {
                // set state to network not registering after 15 retries 
                LOG_DBGS(CH_GSM, "GSM_NOT_REGISTERING retries");
                GsmSetState(GSM_NOT_REGISTERING);
            }
            else
            {
                gsmNwCheckAtCmdState = AT_CMD_RETRY_WAIT;
                gsmNwCheckTick = GetStartTime();
            }
        break;
        
        case AT_CMD_RETRY_WAIT:
            if(TimeSpent(gsmNwCheckTick,GSM_NW_REG_CHECK_RETRY_INTERVAL_MS))
            {
                gsmNwCheckAtCmdState = AT_CMD_SEND;
            }
        break;
        
        default:
          if(TimeSpent(unknownStateStartTick, 60000))
          {
                LOG_DBGS(CH_GSM, "GsmCheckRegistrationHandler - default State");
                unknownStateStartTick = GetStartTime();
          }
          break;
    }
}

gsmRespCb_et GsmRSSIRespCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len)
{
    gsmRespCb_et gsmRespRetVal = GSM_RESP_CB_WAIT;
    switch(gsmResp)
    {
        //lines reading and processing done complete the timeout
        case  RESP_OK:        
            gsmRespRetVal = GSM_RESP_CB_OK_COMPLETE;
            gsmRssiAtCmdState = AT_CMD_SUCCESS;
        break;
       
        case RESP_ERROR:    //if received "ERROR
        case RESP_CME_ERROR:
        case RESP_TIMEOUT:
            LOG_ERR(CH_GSM,"SIGNAL strength response errocode : %d ",gsmResp);
            gsmRespRetVal = GSM_RESP_CB_ERROR_COMPLETE;
            gsmRssiAtCmdState = AT_CMD_FAILURE;
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

uint8_t GsmPollSignalHandler(void)
{
    switch(gsmRssiAtCmdState)
    {
        case AT_CMD_SEND:
            if(GsmIsAtIdle())
            {
                GsmSendAtCmd((char*)gsmRssiQuery,strlen((const char *)gsmRssiQuery),
                300,GsmRSSIRespCb);
                gsmRssiAtCmdState = AT_WAIT_REPLY;
            }
        break;
       
        case AT_WAIT_REPLY:
        break;
       
        case AT_CMD_SUCCESS:
            gsmRssiFlag  =1;
            // set the gsm state to idle
            //LOG_DBG(CH_POLL,"get RSSI Cmd Success");
        break;
       
        case AT_CMD_FAILURE:
            gsmRssiFlag = 2;
            // set the gsm state to idle
            LOG_ERR(CH_GSM,"get RSSI Cmd Failure");
        break;
       
        case AT_CMD_RETRY_WAIT:
        break;
    }
    return gsmRssiFlag;
}

//void GsmQueryNetworkTimeHandler()
//{
//  switch(gsmNwCheckAtCmdState)
//    {
//        case AT_CMD_SEND:
//            // check whether is at command is idle
//            if(GsmIsAtIdle())
//            {
//                // send the at command
//                GsmSendAtCmd((uint8_t *)GSMQueryTime,strlen((const char *)GSMQueryTime),
//                GSM_NW_REG_CHECK_CMD_TIMEOUT_MS,GsmDateTimeRespCb);
//                gsmNwCheckAtCmdState = AT_WAIT_REPLY;
//            }
//        break;
//        
//        case AT_WAIT_REPLY:
//            // the callback function will change the state of the initAtState
//        break;
//        
//        case AT_CMD_SUCCESS:
//            
//        break;
//        
//        case AT_CMD_FAILURE:
//            gsmNwNumRetry++;
//            if(gsmNwNumRetry >= GSM_NW_REG_CHECK_NUM_RETRY)
//            {
//                // set state to network not registering
//                GsmSetState(GSM_NOT_REGISTERING);
//            }
//            else
//            {
//                gsmNwCheckAtCmdState = AT_CMD_RETRY_WAIT;
//                gsmNwCheckTick = GetStartTime();
//            }
//        break;
//        
//        case AT_CMD_RETRY_WAIT:
//            if(TimeSpent(gsmNwCheckTick,GSM_NW_REG_CHECK_RETRY_INTERVAL_MS))
//            {
//                gsmNwCheckAtCmdState = AT_CMD_SEND;
//            }
//        break;
//        
//        default:
//          if(TimeSpent(unknownStateStartTick, 60000))
//          {
//                LOG_DBGS(CH_GSM, "GsmQueryNetworkTimeHandler - default State");
//                unknownStateStartTick = GetStartTime();
//          }
//          break;
//    }
//}

gsmRespCb_et GsmSofShutDownRespCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len)
{
    gsmRespCb_et gsmRespRetVal = GSM_RESP_CB_WAIT;
    switch(gsmResp)
    {
        //lines reading and processing done complete the timeout
        case  RESP_OK:          //not used currently //if at+qpowd = 0 then get OK
            //gsmRespRetVal = GSM_RESP_CB_WAIT;
            gsmSoftShutState = AT_WAIT_REPLY;
        break;
        
        case RESP_ERROR:    //if received "ERROR 
        case RESP_CME_ERROR:
        case RESP_CMS_ERROR:
        case RESP_TIMEOUT:
            LOG_ERR(CH_GSM,"GSM Soft shut down response errocode : %d ",gsmResp);
            gsmRespRetVal = GSM_RESP_CB_ERROR_COMPLETE;
            gsmSoftShutState = AT_CMD_FAILURE;
        break;
        
        
        case RESP_SOFT_SHUT_CMD:
            //wait as we are reading the lines and processing
             LOG_ERR(CH_GSM,"GSM Soft shut down Success response code : %d ",gsmResp);   
             gsmRespRetVal = GSM_RESP_CB_OK_COMPLETE;
             gsmSoftShutState = AT_CMD_SUCCESS;
        break;
    }
    return gsmRespRetVal;
}

gsmCmdState_et GsmOpsNameHandler(void)
{
    switch(gsmCopsAtCmdState)
    {
        case AT_CMD_SEND:
            if(GsmIsAtIdle())
            {
                GsmSendAtCmd((char*)gsmCopsQuery,strlen((const char *)gsmCopsQuery),
                GSM_QUERY_OPS_TIMEOUT_MS,GsmQueryOpsCb);
                gsmCopsAtCmdState = AT_WAIT_REPLY;
            }
        break;
        
        case AT_WAIT_REPLY:
        break;
        
        case AT_CMD_SUCCESS:
//            gsmRssiFlag  =1;
            // set the gsm state to idle
//            LOG_DBG(CH_POLL,"get COPS Cmd Success");
        break;
        
        case AT_CMD_FAILURE:
//            gsmRssiFlag = 1;
            // set the gsm state to idle
            LOG_ERR(CH_POLL,"get COPS Cmd Failure");
        break;
        
        case AT_CMD_RETRY_WAIT:
        break;
    }
    return gsmCopsAtCmdState;
}


gsmCmdState_et GsmSofShutDownHandler(void)
{
    switch(gsmSoftShutState)
    {
        case AT_CMD_SEND:
            if(GsmIsAtIdle())
            {
                GsmSendAtCmd((char*)GSMModemSofShutDown,strlen((const char *)GSMModemSofShutDown),
                60000,GsmSofShutDownRespCb);
                 gsmSoftShutState = AT_WAIT_REPLY;
            }
        break;
        
        case AT_WAIT_REPLY:
        break;
        
        case AT_CMD_SUCCESS:
            
            // set the gsm state to idle
            LOG_DBG(CH_GSM,"GSM Soft shut down Success");
        break;
        
        case AT_CMD_FAILURE:
            
            // set the gsm state to idle
            LOG_ERR(CH_GSM,"GSM Soft shut down Failure");
        break;
        
        case AT_CMD_RETRY_WAIT:
        break;
    }
    return gsmSoftShutState;
}

gsmState_et GetGsmState(void)
{
    return gsmState;
}

void GsmSetState(gsmState_et state)
{
    if(gsmState == state)
    {
        return;
    }
    gsmState = state;
    switch(gsmState)
    {
        case GSM_NO_INIT:
          LOG_DBGS(CH_GSM, "State - GSM_NO_INIT");
        break;
        
        case GSM_HW_POWER_RESET:
            flagNetworkRegistered = 0;
            gprsActivated = 0;
            gsmNwNumRetry = 0;
           // GPRS_LED_KEEP_ON();
            LOG_DBGS(CH_GSM, "State - GSM_HW_POWER_RESET");
            GsmResetPowerSetState(GSM_POWER_OFF_WAIT);
        break;
        
        case GSM_SW_POWER_RESET:
            gsmSoftShutState  = AT_CMD_SEND;
            LOG_DBGS(CH_GSM, "State - GSM_SW_POWER_RESET");
            break;
            
        case GSM_INIT:
            LOG_DBGS(CH_GSM, "State - GSM_INIT");
            GsmInitStart();
        break;        
        
        case SIM_CARD_NOT_PRESENT:
            LOG_ERRS(CH_GSM, "State - SIM_CARD_NOT_PRESENT");
        break;
        
        case GSM_MODEM_NOT_RESPONDING:
            LOG_ERRS(CH_GSM, "State - GSM_MODEM_NOT_RESPONDING");
        break;
	
        case CHECK_GSM_NW_STATUS:
            LOG_DBGS(CH_GSM, "State - CHECK_GSM_NW_STATUS");
            gsmNwCheckAtCmdState = AT_CMD_SEND;
            flagNetworkRegistered = 0;
            gsmNwNumRetry = 0;
            gsmNwRegStateCheck = 0;
            gsmRegQuery = GSMQueryGSMNWRegistration;
            networkState = NETWORK_STAT_NOT_REGISTERED;
        break;
        
        case GSM_GET_SIGNAL_STRENGTH:
           gsmRssiAtCmdState = AT_CMD_SEND;
           gsmRssiFlag = 0;
           GsmPollStatusSetCmd(GSM_POLL_SIGNAL_STRENGTH);
        break;
				
				  case GSM_GET_OPS_NAME:
//           LOG_DBGS(CH_GSM, "State - GSM_GET_OPS_NAME");
           gsmCopsAtCmdState = AT_CMD_SEND;
           break;
           
        case GSM_NOT_REGISTERING:
            LOG_ERRS(CH_GSM, "State - GSM_NOT_REGISTERING");
        break;
        
        case GSM_MODEM_IDLE:
            LOG_DBGS(CH_GSM, "State - GSM_MODEM_IDLE");
        break;
        
        case POLL_STATUS:
            LOG_DBGS(CH_GSM, "State - POLL_STATUS");
        break;
        
//        case QUERY_NETWORK_TIME:
//            LOG_DBGS(CH_GSM, "State - QUERY_NETWORK_TIME");
//            gsmNwCheckAtCmdState = AT_CMD_SEND;
//        break;
          
        case CHECK_GPRS_REG_STATUS:
            LOG_DBGS(CH_GSM, "State - CHECK_GPRS_REG_STATUS");
            gsmNwCheckAtCmdState = AT_CMD_SEND;
            flagGprsNetworkRegisterd = 0;
            gsmNwNumRetry = 0;
            gsmRegQuery = GSMQueryGPRSNWRegistration;
            gprsState = NETWORK_STAT_NOT_REGISTERED;
        break;
        
        case GPRS_CHECK_SSL_CONFIG:
            LOG_DBGS(CH_GSM, "State - GPRS_CHECK_SSL_CONFIG");
            GsmSslConfigStart();
        break;
        
//        case GPRS_NOT_REGISTERING:
//            LOG_ERRS(CH_GSM, "State - GPRS_NOT_REGISTERING");
//        break;
        
        case GPRS_NOT_ACTIVATING:
            LOG_ERRS(CH_GSM, "State - GPRS_NOT_ACTIVATING");
        break;
        
        case GPRS_ACTIVATE:
            LOG_DBGS(CH_GSM, "State - GPRS_ACTIVATE");
            GprsActivateStart();
//            GsmSocketStateReset();
        break;
        
        case GPRS_DEACTIVATE:
            LOG_DBGS(CH_GSM, "State - GPRS_DEACTIVATE");
            gprsDeactTimeout = GetStartTime();
            GprsDeactivateStart();
            GsmSocketStateReset();
        break;
        
        case GSM_SOCKET_PROCESSING:
            LOG_DBGS(CH_GSM, "State - GSM_SOCKET_PROCESSING");
        break;
        
        case GSM_HTTP_SOC_PROCESSING:  
            LOG_DBGS(CH_GSM, "State - GSM_HTTP_SOC_PROCESSING");
        break;
        
        case GSM_SMS_PROCESSING:
            LOG_DBGS(CH_GSM, "State - GSM_SMS_PROCESSING");
        break;
        
        case GSM_FTP_PROCESSING:  
            LOG_DBGS(CH_GSM, "State - GSM_FTP_PROCESSING");
        break;
        
        case GSM_FILE_PROCESSING:  
            LOG_DBGS(CH_GSM, "State - GSM_FILE_PROCESSING");
        break;
    }
}

void GsmStateMachineHandler(void)
{
    gsmInitCmdState_et gsmInitState;
    gprsActivateState_et gprsActivateState;
    sslConfigState_et sslConfigState;
    gprsDeactivateState_et gprsDeactiveState;
    gsmCmdState_et gsmSwShutState;
    gsmCmdState_et gsmPollCmdResult;
    gsmCmdState_et gsmOpsState;
	
    switch(gsmState)
    {
        case GSM_NO_INIT:
            
        break;
        
        case GSM_HW_POWER_RESET:
            if(GsmResetPowerHandler() == GSM_POWERKEY_DONE)
            {
                GsmSetState(GSM_INIT);
            }
        break;
        
        case GSM_SW_POWER_RESET:
             gsmSwShutState = GsmSofShutDownHandler();
             if(gsmSwShutState == AT_CMD_SUCCESS)
             { //compulsory software shutdown before hw reset   
                GsmSetState(GSM_HW_POWER_RESET);
             }
             else if(gsmSwShutState == AT_CMD_FAILURE)
             {
                GsmSetState(GSM_HW_POWER_RESET);
             }
          break;
        
        case GSM_INIT:
            gsmInitState = GsmInitHandler();
            if(gsmInitState == GSM_INIT_CMD_SUCCESS)
            {
                if(flagNetworkRegistered == 1)
                {
                    GsmSetState(CHECK_GPRS_REG_STATUS);
                }
                else
                {
                    GsmSetState(CHECK_GSM_NW_STATUS);
                }
            }
            else if(gsmInitState == GSM_INIT_CMD_FAIL)
            {
                GsmSetState(GSM_MODEM_NOT_RESPONDING);
            }
        break;        
        
        case SIM_CARD_NOT_PRESENT:
            if(TimeSpent(gsmSimNotPresentTick,SIM_NOT_PRESENT_RESTART_MS))
            {
                GsmSetState(GSM_HW_POWER_RESET);
            }
        break;
        
        case GSM_MODEM_NOT_RESPONDING:
            GsmSetState(GSM_HW_POWER_RESET);
        break;
	
        case CHECK_GSM_NW_STATUS:
            // check whether network is already registered
            if(flagNetworkRegistered == 0)
            {
                GsmCheckRegistrationHandler();
            }
            else
            {
                 if(gsmNwRegStateCheck)
                 {
                   gsmNwRegStateCheck = 0;
                   GsmSetState(GSM_GET_SIGNAL_STRENGTH); //GsmSetState(GPRS_ACTIVATE);   -- 210618 //If gsm nw reg is ok then get the signal strength
                 }
                 else
                 {
                    //If gprs nw is reg. directly check the activation
                    GsmSetState(CHECK_GPRS_REG_STATUS);
                 }
            }
        break;
        
         case GSM_GET_SIGNAL_STRENGTH:
             gsmRssiFlag =  GsmPollSignalHandler();
             if(gsmRssiFlag == 1)
             {
                     gsmRssiFlag = 0;
                    //If gprs nw is reg. directly check the activation
                    GsmSetState(GSM_GET_OPS_NAME);
             }
             else if(gsmRssiFlag == 2)
             {
                    gsmRssiFlag = 0;
                    //If gprs nw is reg. directly check the activation
                    GsmSetState(GSM_GET_OPS_NAME);
             }
        break;
						 
			  case GSM_GET_OPS_NAME:
             gsmOpsState =  GsmOpsNameHandler();
             if(gsmOpsState == AT_CMD_SUCCESS)
             {
               //If gprs nw is reg. directly check the activation 
                    GsmSetState(CHECK_GPRS_REG_STATUS);
             }
             else if(gsmOpsState == AT_CMD_FAILURE)
             {
               //If gprs nw is reg. directly check the activation 
                    GsmSetState(CHECK_GPRS_REG_STATUS);
             }
          break;
        
        case GSM_NOT_REGISTERING:
            //GsmSetState(GSM_SW_POWER_RESET);
						GsmSetState(GSM_HW_POWER_RESET);
        break;
        
        case GSM_MODEM_IDLE:
            GsmIdleTaskHandler();
        break;
        
        case POLL_STATUS:
            gsmPollCmdResult = GsmPollStatusHandler();
            if(gsmPollCmdResult == AT_CMD_FAILURE)
            {
                GsmSetState(GSM_MODEM_IDLE);
            }
        break;
        
//        case QUERY_NETWORK_TIME:
//            GsmQueryNetworkTimeHandler();
//            if(IsCurrentYearUpdated() >= CURRENT_DEFAULT_YEAR)
//            {
//                GsmSetState(CHECK_GPRS_REG_STATUS);
//            }
//        break;
        
        case CHECK_GPRS_REG_STATUS:
            if(flagGprsNetworkRegisterd == 0)
            {
                GsmCheckRegistrationHandler();
            }
            else
            {
                GsmSetState(GPRS_ACTIVATE);
            }
        break;
        
        case GPRS_CHECK_SSL_CONFIG:
            sslConfigState = GsmSslConfigHandler();
            if(sslConfigState == SSL_CONFIG_SUCCESS)
            {
                GsmSetState(GSM_MODEM_IDLE);
            }
            else if(sslConfigState == SSL_CONFIG_FAIL)
            {
                GsmSetState(GSM_MODEM_IDLE);
            }
        break;
        
        case GPRS_ACTIVATE:
            gprsActivateState = GprsActivationHandler();
            if(gprsActivateState == GPRS_ACTIVATION_SUCCESS)
            {
                gprsActFailCnt = 0;
                GsmResetGprsActFlag();
                gprsActTimeout = GetStartTime();
                
                // activation success
                // check whether gprs ssl config is required
                if(GsmSocketIsAnySecure())
                {
                    GsmSetState(GPRS_CHECK_SSL_CONFIG);
                }
                else
                {
                    GsmSetState(GSM_MODEM_IDLE);
                }
            }
            else if(gprsActivateState == GPRS_ACTIVATION_FAIL)
            {
                // deactivate GPRS and then try activating
                  if(TimeSpent(gprsActTimeout, MAX_GPRS_ACT_TIMEOUT)) 
                  {
                    LOG_ERR(CH_GSM,"FAILED to Activate GPRS for 5 minutes, Reset the GSM Modem");
                    gprsActFailCnt = 0;
                    GsmResetGprsActFlag();
                    GsmSetState(GPRS_NOT_ACTIVATING);
                    gprsActTimeout = GetStartTime();
                  }
                  else  if(!gprsActFailCnt)
                  {
                    LOG_ERR(CH_GSM,"Failed to activate the GPRS, Set The Modem IDLE");
                    // deactivate GPRS and then try activating
                    gprsActFailCnt = 1;
                    GsmSetState(GSM_MODEM_IDLE);  //GPRS_DEACTIVATE
                  }
                  else
                  {
                    LOG_ERR(CH_GSM,"Failed to activate the GPRS, Deactivate the GPRS");
                    gprsActFailCnt = 0;
                    GsmSetState(GPRS_DEACTIVATE);
                  }
            }
        break;
        
        case GPRS_NOT_ACTIVATING:
            GsmSetState(GSM_SW_POWER_RESET);
        break;
        
        case GPRS_DEACTIVATE:
            gprsDeactiveState = GprsDeactivationHandler();
            if(gprsDeactiveState == GPRS_DEACTIVATION_SUCCESS)
            {
                GsmSetState(CHECK_GPRS_REG_STATUS);
                oneSecTick = GetStartTime();
            }
            else if(gprsDeactiveState == GPRS_DEACTIVATION_FAILURE)
            {
                GsmSetState(CHECK_GSM_NW_STATUS);
            }
            else if(TimeSpent(gprsDeactTimeout, MAX_GPRS_DEACT_TIMEOUT))
            {
                gprsDeactTimeout = GetStartTime();
                LOG_DBGS(CH_GSM, "MAX_GPRS_DEACT_TIMEOUT");
                GsmSetState(CHECK_GSM_NW_STATUS);
            }
        break;
        
        case GSM_SOCKET_PROCESSING:
          break;
        
        case GSM_HTTP_SOC_PROCESSING:

          break;
        case GSM_SMS_PROCESSING:
        case GSM_FTP_PROCESSING:  
        case GSM_FILE_PROCESSING:  
        break;
        
        default:
          if(TimeSpent(unknownStateStartTick, 60000))
          {
                LOG_DBGS(CH_GSM, "GsmStateMachineHandler - default State");
                unknownStateStartTick = GetStartTime();
          }
          break;
    }
}

gsmState_et GsmGetModemState(void)
{
   return gsmState;

}

uint8_t GsmStateIsIdle(void)
{
	if(gsmState == GSM_MODEM_IDLE)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

uint8_t GsmStateIsFileProcessing(void)
{
    if(gsmState == GSM_FILE_PROCESSING)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void GsmIndependentTask(void)
{
//    if(IsPacketSendProtocolTypeHttp() != 1)
//    {
        GsmSocketHandler();
//    }
//    else
//    {
//        HttpSendPacketHandler();
//    }
    GsmSmsHandler();
    GsmFtpHandler();
}

void GsmStateMachine(void)
{
    GsmStateMachineHandler();
    GsmIndependentTask();
}