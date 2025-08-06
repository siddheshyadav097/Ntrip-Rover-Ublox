#include "gsm_gprs_api.h"
#include <string.h>

// the current gprs configuration is stored here
gprsConfig_st *gprsSim1Config;
gprsConfig_st *gprsSim2Config;

// the current gprs configuration is stored here
gprsConfig_st *gprsSim0MemConfig;

gprsConfig_st* gprsConfig_sim0Value;

gprsActivateState_et gprsInitState = GPRS_WAIT_FOR_ACTIVATE;

gsmCmdState_et gprsInitAtCmdState = AT_CMD_SEND;

gprsInitCmd_et gprsInitCurrCmd = CONFIGURE_GPRS;        //ENABLE_TCPIP_MUX;
uint8_t gprsActivated = 0;
uint8_t gsmlocalIpBuf[30] = {0};
char gprsConfigCmdBuff[100] = {0};
uint8_t flagLocalIpUpdated = 0;
uint8_t gprsActSucessFlag = 0;
const char gprsConfigCmd[] = "AT+QICSGP=1,1,";
const char gprsDeactivateCmd[] = "AT+QIDEACT=1\r";

uint8_t gprsConfigFailCnt = 0;
extern uint8_t gprsActFailCnt;	
extern uint32_t gprsActTimeout;
uint32_t gprsStateStartTick = 0;

gsmRespCb_et GprsOkRespCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len);
gsmRespCb_et GprsLocalIpRespCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len);

gprsInitCmd_st gprsInitCmdList[]=
{
    //{"AT+QIMUX=0\r",GPRS_INIT_CMD_DEFAULT_TIMEOUT,GprsOkRespCb},
    //{"AT+QIFGCNT=0\r",GPRS_INIT_CMD_DEFAULT_TIMEOUT,GprsOkRespCb},
    {"AT+QICSGP=1,1,",GPRS_INIT_CMD_DEFAULT_TIMEOUT,GprsOkRespCb},        //AT+QICSGP=1,1,"www","","",0
    //{"AT+QIREGAPP\r",GPRS_INIT_CMD_DEFAULT_TIMEOUT,GprsOkRespCb},
    //{"AT+QINDI=1\r",READ_SOCKET_URC_TIMEOUT,GprsOkRespCb},
    //{"AT+QIDNSIP=1\r",GPRS_INIT_CMD_DEFAULT_TIMEOUT,GprsOkRespCb},
    {"AT+QIACT=1\r",GPRS_ACT_CMD_TIMEOUT,GprsOkRespCb},                       //AT+QIACT=1 
    //{"AT+QILOCIP\r",GSM_GET_LOC_IP_TIMEOUT,GprsLocalIpRespCb},
};

void GetGprsConfigsFromMem(void)
{
  gprsSim0MemConfig = GetGprs0SimConfig();
}

void GprsSetSim1Config(gprsConfig_st *config)
{
    gprsSim1Config = config;
}

void GprsSetSim2Config(gprsConfig_st *config)
{
    gprsSim2Config = config;
}

gsmRespCb_et GprsLocalIpRespCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len)
{
    gsmRespCb_et gsmRespRetVal;
    switch(gsmResp)
    {
        case RESP_OK:        
            gsmRespRetVal = GSM_RESP_CB_OK_COMPLETE;
            gprsInitAtCmdState = AT_CMD_SUCCESS;
        break;
        
        case RESP_ERROR:    
        case RESP_CME_ERROR:
        case RESP_CMS_ERROR:
        case RESP_TIMEOUT:
            LOG_ERR(CH_GSM,"GPRS Init Cmd:%d errcode :%d",gprsInitCurrCmd,gsmResp);
            gsmRespRetVal = GSM_RESP_CB_ERROR_COMPLETE;
            gprsInitAtCmdState = AT_CMD_FAILURE;
        break;
        
        case RESP_IGNORE:
            gsmRespRetVal = GSM_RESP_CB_WAIT;
        break;
        
        case RESP_OTHER:  
              //Capture the loacl ip  here.
            memset(gsmlocalIpBuf,0x00,sizeof(gsmlocalIpBuf));
            strncpy((char*)gsmlocalIpBuf,(char*)lineBuff,len);
            flagLocalIpUpdated = 1;
            gsmRespRetVal = GSM_RESP_CB_OK_COMPLETE;
            gprsInitAtCmdState = AT_CMD_SUCCESS;
            LOG_INFO(CH_GPRS,"Local IP : %s",gsmlocalIpBuf);
        break;
        
        default:
        break;
    }
    return gsmRespRetVal;
}

/**
 *  @brief     :This AtCmdReplyCb() defualt callback for the commands expecting OK or ERROR.
 *  @Inputs:   gsmResp of type enum  gsmResPonse_et
 *             lineBuff - Pointer to the buffer in which modem response(processlinebuffer - Extended Response or SerialLineBuff)is received.
 *             len      - Length of the data received.
 *  @param     [out] gsmRespCb_et.
 *  @return    1 value of enum @ref gsmRespCb_et        
 *             :GSM_RESP_CB_COMPLETE if OK received or 
 *             GSM_RESP_CB_ERROR_COMPLETE if ERROR is received. */
gsmRespCb_et GprsOkRespCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len)
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
            gprsInitAtCmdState = AT_CMD_SUCCESS;
        break;
        
        case RESP_ERROR:
        case RESP_CME_ERROR:
        case RESP_CMS_ERROR:
        case RESP_TIMEOUT:
            LOG_ERR(CH_GSM,"GPRS Init Cmd:%d errcode :%d",gprsInitCurrCmd,gsmResp);
            gsmRespRetVal   = GSM_RESP_CB_ERROR_COMPLETE;   
            // this is done to inform the init handler that the cmd response is received as it is 
            // waiting for the reply
            gprsInitAtCmdState = AT_CMD_FAILURE;
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

void getGPRSmemconfig(void)
{
    gprsConfig_sim0Value = GetGprs0SimConfig();
}

void GsmResetGprsActTime(void)
{
    gprsActTimeout = GetStartTime();
}

void GprsActivateStart(void)
{
    gprsInitAtCmdState = AT_CMD_SEND;
    gprsInitState = GPRS_ACTIVATE_IN_PROGRESS;
    gprsInitCurrCmd = CONFIGURE_GPRS;
    flagLocalIpUpdated = 0;
    gprsActivated = 0;
        
    snprintf((char *)gprsConfigCmdBuff,sizeof(gprsConfigCmdBuff),"%s\"%s\",\"%s\",\"%s\",0\r",gprsConfigCmd,gprsConfig_sim0Value->apn,gprsConfig_sim0Value->username,gprsConfig_sim0Value->pass); 
    gprsInitCmdList[CONFIGURE_GPRS].cmd = gprsConfigCmdBuff;
}

void UpdateSimApn(char* apnString,uint8_t apnLen)
{
  memset(gprsSim0MemConfig->apn,0,sizeof(gprsSim0MemConfig->apn));
  
  strncpy((char*)gprsSim0MemConfig->apn,(char*)apnString,apnLen);

}
gprsActivateState_et GprsActivationHandler(void)
{
    switch(gprsInitState)
    {
        case GPRS_ACTIVATE_IN_PROGRESS:
            switch(gprsInitAtCmdState)
            {
                case AT_CMD_SEND:
                    // check whether is at command is idle
                    if(GsmIsAtIdle())
                    {
                        // send the at command
                        // check whether if the command is 
                        GsmSendAtCmd(gprsInitCmdList[gprsInitCurrCmd].cmd,strlen((const char *)gprsInitCmdList[gprsInitCurrCmd].cmd),
                                gprsInitCmdList[gprsInitCurrCmd].cmdTimeoutInMs,gprsInitCmdList[gprsInitCurrCmd].respCb);
                        gprsInitAtCmdState = AT_WAIT_REPLY;
                    }
                break;
                
                case AT_WAIT_REPLY:
                    // the callback function will change the state of the initAtState
                break;
                
                case AT_CMD_SUCCESS:
                    LOG_DBG(CH_GPRS,"GPRS Init Cmd %d success",gprsInitCurrCmd);
                    //(gprsInitCmd_et)gprsInitCurrCmd++;
								    gprsInitCurrCmd++;
                    if(gprsInitCurrCmd >= GPRS_NUM_INIT_CMDS)
                    {
                        gprsInitState = GPRS_ACTIVATION_SUCCESS;
                        gprsActivated = 1;
                        gprsActSucessFlag  = 0;
                    }
                    else
                    {
                        gprsInitAtCmdState = AT_CMD_SEND;
                    }
                break;
                
                case AT_CMD_FAILURE:
                    gprsInitState = GPRS_ACTIVATION_FAIL;
                    gprsActivated = 0;
                    if(gprsInitCurrCmd == ACTIVATE_GPRS)//if failure for gprs act commnd  
                    {
                      if(!gprsActSucessFlag)
                      {
                          gprsActSucessFlag = 1;
                          gprsActTimeout = GetStartTime();
                      }
                    }
                    else
                    {
                      if(!gprsActSucessFlag)
                      {
                        gprsActSucessFlag = 1;
                        gprsActTimeout = GetStartTime();
                      }
                    }
                break;
                
                case AT_CMD_RETRY_WAIT:
                break;
            } 
        break;
        
        case GPRS_ACTIVATION_SUCCESS:
        break;
        
        case GPRS_ACTIVATION_FAIL:
        break;
        
        default:
          if(TimeSpent(gprsStateStartTick, 60000))
          {
                LOG_DBGS(CH_GSM, "GprsActivationHandler - default State");
                gprsStateStartTick = GetStartTime();
          }
          break;
    }
    return gprsInitState;
}

gprsDeactivateState_et gprsDeactivateState = GPRS_DEACTIVATE_WAIT_FOR_START;
gsmCmdState_et gprsDeactivateAtCmdState = AT_CMD_SEND;

gsmRespCb_et GprsDeactivateCmdCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len)
{
    gsmRespCb_et gsmRespRetVal;
    switch(gsmResp)
    {
        case RESP_OK:
        case RESP_DEACT_OK:
            gsmRespRetVal = GSM_RESP_CB_OK_COMPLETE;
            gprsDeactivateAtCmdState = AT_CMD_SUCCESS;
        break;
        
        case RESP_OTHER:
            gsmRespRetVal = GSM_RESP_CB_WAIT;
        break;
        
        case RESP_ERROR:    
        case RESP_TIMEOUT:
        case RESP_IGNORE:
        case RESP_CME_ERROR:
            gsmRespRetVal = GSM_RESP_CB_ERROR_COMPLETE;
            gprsDeactivateAtCmdState = AT_CMD_FAILURE;
        break;
        
        default:
        break;
    }
    return gsmRespRetVal;
}

void GprsDeactivateStart(void)
{
    // check for deactivation state
    gprsDeactivateState = GPRS_DEACTIVATE_IN_PROGRESS;
    gprsDeactivateAtCmdState = AT_CMD_SEND;
}

gprsDeactivateState_et GprsDeactivationHandler(void)
{
    switch(gprsDeactivateState)
    {
        case GPRS_DEACTIVATE_WAIT_FOR_START:
        break;
        
        case GPRS_DEACTIVATE_IN_PROGRESS:
            switch(gprsDeactivateAtCmdState)
            {
                case AT_CMD_SEND:
                     // check whether if the command is 
                    GsmSendAtCmd((char *)gprsDeactivateCmd,strlen((const char *)gprsDeactivateCmd),
                            GPRS_PDP_DEACT_CMD_TIMEOUT,GprsDeactivateCmdCb);
                    gprsDeactivateAtCmdState = AT_WAIT_REPLY;
                break;
                
                case AT_WAIT_REPLY:
                    
                break;
								
								case AT_CMD_RETRY_WAIT:
									break;
                
                case AT_CMD_FAILURE:
                    LOG_DBGS(CH_GPRS,"Deactivation failed");
                    gprsDeactivateState = GPRS_DEACTIVATION_FAILURE;
                    gprsActivated = 0;
                break;
                
                case AT_CMD_SUCCESS:
                    LOG_DBGS(CH_GPRS,"Deactivation success");
                    gprsDeactivateState = GPRS_DEACTIVATION_SUCCESS;
                    gprsActivated = 0;
                break;
            }
        break;
        
        case GPRS_DEACTIVATION_SUCCESS:
        break;
        
        case GPRS_DEACTIVATION_FAILURE:
        break;
        
        default:
          if(TimeSpent(gprsStateStartTick, 60000))
          {
                LOG_DBGS(CH_GSM, "GprsDeactivationHandler - default State");
                gprsStateStartTick = GetStartTime();
          }
          break;
    }
    return gprsDeactivateState;
}

uint8_t GsmGprsIsActive(void)
{
    if(gprsActivated)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void GsmResetGprsActFlag(void)
{
    gprsActSucessFlag = 0;
}

void UrcGprsDeactiveCb(uint8_t *lineBuff, uint16_t len)
{
    gprsActivated = 0;
}
