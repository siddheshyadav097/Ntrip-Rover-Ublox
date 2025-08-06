#include "gsm_sim_api.h"
#include "gsm_idle_task.h"
#include "gsm_socket_api.h"
#include "gsm_utility.h"
#include "gsm_gprs_api.h"
#include <string.h>
#include <stdlib.h>
#include  "packet_api.h"


// the state of the init 
gsmInitCmdState_et gsmInitState = GSM_INIT_CMD_WAIT;
// the total number of init commands to be called
uint8_t gsmInitTotalCmds = 0;
// the current init command is in progress
gsmInitCMD_et gsmInitCurrentCmdIndex = AT_CMD;
// the at command state 
gsmCmdState_et gsmInitAtCmdState = AT_CMD_SEND;

// retry count to be incremented when cmd failure is received
uint8_t gsmInitCmdRetryCount = 0;
// tick timer used for waiting during retry interval
uint32_t gsmInitRetryCmdTick = 0;

// buffer to store ccid
static uint8_t imeiBuff[IMEI_BUFF_SIZE];
uint8_t flagImeiUpdated = 0;

// buffer to store imei
static uint8_t ccidBuff[CCID_BUFF_SIZE];
uint8_t flagCcidUpdated = 0;

static uint8_t OperatorNameFrmSIM[OPERATOR_NAME_SIM_SIZE]= {0};
//uint8_t flagOperatorNameUpdated = 0;
//static uint8_t OperatorSelection[OPERATOR_SELECTION];
//uint8_t flagOperatorNameUpdated = 0;

sysInitState_et gsmInitQueryState;
char strTmp[100];



simState_et simState  = SIM_STAT_READY;
extern uint8_t GetDataFirstTime;

gsmRespCb_et GsmOkRespCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len);
gsmRespCb_et GsmCcidImeiRespCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len);
gsmRespCb_et GsmQueryInitCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len);
gsmRespCb_et GsmSimStatusRespCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len);
//gsmRespCb_et GsmQSPNHandlerCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len);

#if JIO_SIM_SET_APN  
// list of all the init commands with their callback function
gsmInitCmd_st gsmInitCmdList[]  = {
	
	{"AT\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmOkRespCb,0},
	{"AT&F\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmOkRespCb,0},
	{"ATE0\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmOkRespCb,0},
	{"AT+CMEE=2\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmOkRespCb,0},
        {"AT+IPR=115200\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmOkRespCb,0},
	{"AT+CFUN=1\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmOkRespCb,0},
		{"AT+GSN\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmCcidImeiRespCb,0},
	{"AT+QINISTAT\r",GSM_QUERY_INIT_NUM_RETRY,GSM_QUERY_INIT_TIMEOUT,GSM_QUERY_INIT_RETRY_WAIT_MS,GsmQueryInitCb,0},
	{"AT+CREG=1\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmOkRespCb,0},
	{"AT+CGREG=1\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmOkRespCb,0},
        {"AT+CEREG=1\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmOkRespCb,0},  //EPS Network Registration Status
//	{"AT+GSN\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmCcidImeiRespCb,0},
	{"AT+QCCID\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmCcidImeiRespCb,0},
        {"AT+GMR\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmOkRespCb,0},      //EC25EFAR06A06M4G
        {"AT+CREG?\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmOkRespCb,0}, 
        {"AT+CSQ\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmOkRespCb,0}, 
	{"AT+CGDCONT?\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmOkRespCb,0}, 
	{"AT+CGDCONT=1,\"IPV4V6\",\"jionet\"\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmOkRespCb,0}, 
        {"AT+QURCCFG=\"urcport\",\"uart1\"\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmOkRespCb,0},
        {"AT+QURCCFG=\"urcport\"\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmOkRespCb,0},
        {"AT+QCFG=\"urc/cache\",0\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT+200,GSM_INIT_RETRY_WAIT_MS,GsmOkRespCb,0},
        //{"AT+COPS=0,1\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmOkRespCb,0},
//	{"AT+CTZU=1\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmOkRespCb,0},
	{"AT+CMGF=1\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmOkRespCb},
	{"AT+CSCS=\"IRA\"\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT+200,GSM_INIT_RETRY_WAIT_MS,GsmOkRespCb,0},
	//{"AT+CSMP=17,167,0,241\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmOkRespCb,0},																													
        {"AT&W\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmOkRespCb,0},
	{"AT+CPIN?\r",GSM_SIM_STAT_NUM_RETRY,GSM_SIM_STAT_TIMEOUT,GSM_SIM_STAT_RETRY_WAIT_MS,GsmSimStatusRespCb},
//	{"AT+QSPN\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmOperatorNameRespCb},
//      {"AT+CCLK?\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmDateTimeRespCb},
        {"AT+CMGD=1,4\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_DEL_ALL_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmOkRespCb}
};


#else
gsmInitCmd_st gsmInitCmdList[]  = {
	
	{"AT\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmOkRespCb,0},
	{"AT&F\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmOkRespCb,0},
	{"ATE0\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmOkRespCb,0},
	{"AT+CMEE=2\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmOkRespCb,0},
        {"AT+IPR=115200\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmOkRespCb,0},
	{"AT+CFUN=1\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmOkRespCb,0},
	{"AT+QINISTAT\r",GSM_QUERY_INIT_NUM_RETRY,GSM_QUERY_INIT_TIMEOUT,GSM_QUERY_INIT_RETRY_WAIT_MS,GsmQueryInitCb,0},
	{"AT+CREG=1\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmOkRespCb,0},
	{"AT+CGREG=1\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmOkRespCb,0},
        {"AT+CEREG=1\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmOkRespCb,0},  //EPS Network Registration Status
	{"AT+GSN\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmCcidImeiRespCb,0},
	{"AT+QCCID\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmCcidImeiRespCb,0},
        {"AT+GMR\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmOkRespCb,0},      //EC25EFAR06A06M4G
        {"AT+QURCCFG=\"urcport\",\"uart1\"\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmOkRespCb,0},
        {"AT+QURCCFG=\"urcport\"\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmOkRespCb,0},
        {"AT+QCFG=\"urc/cache\",1\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT+200,GSM_INIT_RETRY_WAIT_MS,GsmOkRespCb,0},
        {"AT+COPS=0,1\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmOkRespCb,0},
//	{"AT+CTZU=1\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmOkRespCb,0},
	{"AT+CMGF=1\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmOkRespCb},
	{"AT+CSCS=\"IRA\"\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT+200,GSM_INIT_RETRY_WAIT_MS,GsmOkRespCb,0},
	//{"AT+CSMP=17,167,0,241\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmOkRespCb,0},																													
        {"AT&W\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmOkRespCb,0},
	{"AT+CPIN?\r",GSM_SIM_STAT_NUM_RETRY,GSM_SIM_STAT_TIMEOUT,GSM_SIM_STAT_RETRY_WAIT_MS,GsmSimStatusRespCb},
//	{"AT+QSPN\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmOperatorNameRespCb},
//      {"AT+CCLK?\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_CMD_DEFAULT_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmDateTimeRespCb},
        {"AT+CMGD=1,4\r",GSM_INIT_CMD_NUM_RETRY,GSM_INIT_DEL_ALL_TIMEOUT,GSM_INIT_RETRY_WAIT_MS,GsmOkRespCb}
};
#endif

/**
*  @brief  :This GsmQSPNHandlerCb() callback function for the AT+QSPN? commands expecting +QSPN:,OK or ERROR.
 *   @Inputs:  gsmResp of type enum  gsmResPonse_et
 *             lineBuff - Pointer to the buffer in which modem response(processlinebuffer - Extended Response or SerialLineBuff)is received.
 *             len      - Length of the data received.
 *  @param     [out] gsmRespCb_et.
 *  @return    1 value of enum @ref gsmRespCb_et        
 *             :RESP_OK --> GSM_RESP_CB_COMPLETE if OK received or 
 *             :RESP_ERROR --> GSM_RESP_CB_ERROR_COMPLETE if ERROR is received.
*              :RESP_CME_ERROR --> GSM_RESP_CB_ERROR_COMPLETE if cme error is received.
*              :RESP_SERVICE_PROVIDER_NAME --> GSM_RESP_CB_WAIT when QSPN prefix is received wait to receive OK.
*/
//gsmRespCb_et GsmQSPNHandlerCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len)
//{
//    gsmRespCb_et gsmRespRetVal = GSM_RESP_CB_WAIT;
//    char *p = NULL;
//    char *q = NULL; 
//    char *r = NULL;
//    switch(gsmResp)
//    {
//        case  RESP_OK: 
//            gsmRespRetVal = GSM_RESP_CB_OK_COMPLETE;
//            gsmInitAtCmdState = AT_CMD_SUCCESS;
//        break;
//        
//        case RESP_ERROR:   
//        case RESP_CME_ERROR:
//        case RESP_TIMEOUT:
//            gsmRespRetVal = GSM_RESP_CB_ERROR_COMPLETE;
//            gsmInitAtCmdState = AT_CMD_FAILURE;
//        break;
//           
//        case RESP_IGNORE:
//            gsmRespRetVal = GSM_RESP_CB_WAIT;
//        break;
//        
//        case RESP_SERVICE_PROVIDER_NAME:    
//            memset(OperatorSelection,0x00,sizeof(OperatorSelection));
//            p = (char *)lineBuff + 6;
//            p++;
//            q = strchr(p,'"');
//            q++;
//            r = strchr(q,'"');
//            if(q && r)
//            {
//                strncpy((char*)OperatorSelection,q,r-q);
//                flagOperatorNameUpdated = 1;
//            }
//            gsmRespRetVal = GSM_RESP_CB_WAIT;   
//        break;
//          
//        default:
//        break;
//    }
//    return gsmRespRetVal;
//}

/**
*  @brief  :This GsmCPINHandlerCb() callback function for the AT+CPIN? commands expecting +CPIN:,OK or ERROR.
 *   @Inputs:  gsmResp of type enum  gsmResPonse_et
 *             lineBuff - Pointer to the buffer in which modem response(processlinebuffer - Extended Response or SerialLineBuff)is received.
 *             len      - Length of the data received.
 *  @param     [out] gsmRespCb_et.
 *  @return    1 value of enum @ref gsmRespCb_et        
 *             :RESP_OK --> GSM_RESP_CB_COMPLETE if OK received or 
 *             :RESP_ERROR --> GSM_RESP_CB_ERROR_COMPLETE if ERROR is received.
*              :RESP_CME_ERROR --> GSM_RESP_CB_ERROR_COMPLETE if cme error is received.
*              :RESP_SIM_STATUS --> GSM_RESP_CB_WAIT when CPIN prefix is received wait to receive OK.
*/
gsmRespCb_et GsmSimStatusRespCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len)
{
    gsmRespCb_et gsmRespRetVal = GSM_RESP_CB_WAIT;
    char *head = NULL;
    char *p = NULL;
    char *q = NULL; 
    
    switch(gsmResp)
    {
        case  RESP_OK:
          gsmRespRetVal = GSM_RESP_CB_OK_COMPLETE;
          // ok received now check whether simState is
          if(simState == SIM_STAT_READY)
          {
            gsmInitAtCmdState = AT_CMD_SUCCESS;
          }
          else
          {
            gsmInitAtCmdState = AT_CMD_FAILURE;
          }
        break;
        
        case RESP_ERROR:   
        case RESP_TIMEOUT:
            LOG_ERR(CH_GSM,"Init Cmd:%d errcode :%d",gsmInitCurrentCmdIndex,gsmResp);
            gsmRespRetVal = GSM_RESP_CB_ERROR_COMPLETE;
            gsmInitAtCmdState = AT_CMD_FAILURE;
        break;
        
        case RESP_CME_ERROR:
          head = GsmFindString((char *)lineBuff, strlen((char *)lineBuff), "+CME ERROR:");
          if(head)
          { 
                p = head + strlen("+CME ERROR:");
                p++;          //space
                q = strchr(p ,'\0');
                if (p && q)
                {
                    memset(strTmp,0x00,sizeof(strTmp));
                    strncpy(strTmp, p,q-p);
                }         
               simState =  GsmGetSimStateByErrCode(strTmp,strlen(strTmp));   
          }
           gsmRespRetVal = GSM_RESP_CB_ERROR_COMPLETE;
           gsmInitAtCmdState = AT_CMD_FAILURE;
          break;
           
        case RESP_IGNORE:
          gsmRespRetVal = GSM_RESP_CB_WAIT;
        break;
        
        case RESP_SIM_STATUS:            
             head  = GsmFindString((char *)lineBuff, strlen((char *)lineBuff),"+CPIN:"); 
             if(head)
             {
                p = head + strlen("+CPIN:");
                p++;
                q = strchr(p ,'\0');
                if (p && q)
                {
                    memset(strTmp,0x00,sizeof(strTmp));
                    strncpy(strTmp, p,q-p);
                }
                simState = GsmGetSimStateByName(strTmp,strlen(strTmp));   
                gsmRespRetVal = GSM_RESP_CB_WAIT;
             }
             else
             {
                 gsmRespRetVal = GSM_RESP_CB_ERROR_COMPLETE;
                 gsmInitAtCmdState = AT_CMD_FAILURE;
             }
        break;
        
         default:
        break;
    }
    return gsmRespRetVal;
}


/**
 *  @brief     :This GsmQueryInitCmdReplyCb() defualt callback for the AT+QINISTAT command.
 *  @Inputs:   gsmResp of type enum  gsmResPonse_et
 *             lineBuff - Pointer to the buffer in which modem response(processlinebuffer - Extended Response or SerialLineBuff)is received.
 *             len      - Length of the data received.
 *  @param     [out] gsmRespCb_et.
 *  @return    1 value of enum @ref gsmRespCb_et        
 *             :GSM_RESP_CB_COMPLETE if +QINISTAT: and OK received or 
 *             :GSM_RESP_CB_ERROR_COMPLETE if ERROR is received. */
gsmRespCb_et GsmQueryInitCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len)
{
   //what we want to return from the calllback depending on the reponse received
    gsmRespCb_et gsmRespRetVal = GSM_RESP_CB_WAIT;
    char *head = NULL;
     //here received the response from processlinebuffer
    
    switch(gsmResp)
    {
        case RESP_OK: 
          gsmRespRetVal = GSM_RESP_CB_OK_COMPLETE;
          if((gsmInitQueryState == SYSTEM_STATE_SMSOK) || (gsmInitQueryState == SYSTEM_STATE_ALL_READY))
          {
            gsmInitAtCmdState = AT_CMD_SUCCESS;
          }
          else
          {
            gsmInitAtCmdState = AT_CMD_FAILURE;
          }
          break;
        
        case RESP_ERROR:
        case RESP_TIMEOUT:
            LOG_ERR(CH_GSM,"Init Cmd:%d errcode :%d",gsmInitCurrentCmdIndex,gsmResp);
            gsmRespRetVal = GSM_RESP_CB_ERROR_COMPLETE;
            gsmInitAtCmdState = AT_CMD_FAILURE;
        break;
           
        case RESP_IGNORE:
          gsmRespRetVal = GSM_RESP_CB_WAIT;
        break;
        
        case RESP_OTHER:
            head = GsmFindString((char *)lineBuff, strlen((char *)lineBuff),"+QINISTAT: ");
            if(head != 0)
            {
                head = head + 11;
                memset(strTmp,0,sizeof(strTmp));
                strncpy((char*)strTmp , (const char*)head, 1);
                gsmInitQueryState = (sysInitState_et)atoi((const char*)strTmp); 
            }
            gsmRespRetVal = GSM_RESP_CB_WAIT;   
        break;
        
        default:
        break;
        
    }
    return gsmRespRetVal;
}

/**
 *  @brief  :This GsmCCIDResponseCb() callback for the CCID Handler routine.
 *   @Inputs:  gsmResp of type enum  gsmResPonse_et
 *             lineBuff - Pointer to the buffer in which modem response(processlinebuffer - Extended Response or SerialLineBuff)is received.
 *             len      - Length of the data received.
 *  @return    1 value of enum @ref gsmRespCb_et        
 *             :GSM_RESP_CB_COMPLETE if OK received or 
 *             :GSM_RESP_CB_ERROR_COMPLETE if ERROR is received. 
 *             : RESP_OTHER means CCID data is received in serial buff fetch it wait to process OK 
 *              on next line.
 */
gsmRespCb_et GsmCcidImeiRespCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len)
{
    gsmRespCb_et gsmRespRetVal = GSM_RESP_CB_WAIT;
    char *p = NULL;
    char *q = NULL;
    
	switch(gsmResp)
	{
        case RESP_OK: 
            //  the resp ret val is used by the call back function 
            gsmRespRetVal  = GSM_RESP_CB_OK_COMPLETE;
            // this is done to inform the init handler that the cmd response is received as it is 
            // waiting for the reply
            gsmInitAtCmdState = AT_CMD_SUCCESS;
        break;
        
        case RESP_ERROR:
        case RESP_CME_ERROR:
        case RESP_TIMEOUT:
            LOG_ERR(CH_GSM,"Init Cmd:%d errcode :%d",gsmInitCurrentCmdIndex,gsmResp);
            gsmRespRetVal   = GSM_RESP_CB_ERROR_COMPLETE;   
            // this is done to inform the init handler that the cmd response is received as it is 
            // waiting for the reply
            gsmInitAtCmdState = AT_CMD_FAILURE;
        break;
        
        case RESP_IGNORE:
            gsmRespRetVal = GSM_RESP_CB_WAIT;    //+QNITZ/+CCLK URC is rcvd then ignore it.
        break;
        
        case RESP_OTHER: 
            gsmRespRetVal = GSM_RESP_CB_WAIT; 
            memset(ccidBuff,0,sizeof(ccidBuff));
            p = strstr((char *)lineBuff,"+QCCID: ");  
            if(p)
            {
                q = strchr((char *)lineBuff, '\0');
                if(p && q)
                {
                    strncpy((char *)ccidBuff, p+8, (q-p-8));
                    flagCcidUpdated = 1;
                    LOG_INFO(CH_GSM,"CCID:%s",ccidBuff);
									  PacketGetImeiCcid();
                }
            }
            else
            {
                if(len >= 15)
                {
                    q = strchr((char *)lineBuff, '\0');
                    if(q)
                    {
                        strncpy((char *)imeiBuff, (const char*)lineBuff, len);
                        flagImeiUpdated = 1;
                        LOG_INFO(CH_GSM,"IMEI:%s",imeiBuff);
											  PacketGetImeiCcid();
                    }
                }
            }
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
gsmRespCb_et GsmOkRespCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len)
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
            gsmInitAtCmdState = AT_CMD_SUCCESS;
        break;
        
        case RESP_CME_ERROR:
            LOG_ERR(CH_GSM,"Init Cmd:%d errcode :%d",gsmInitCurrentCmdIndex,gsmResp);
            gsmRespRetVal   = GSM_RESP_CB_ERROR_COMPLETE;   
            // this is done to inform the init handler that the cmd response is received as it is 
            // waiting for the reply
            gsmInitAtCmdState = AT_CMD_FAILURE;
        break;
        
        case RESP_ERROR:
        case RESP_TIMEOUT:
            LOG_ERR(CH_GSM,"Init Cmd:%d errcode :%d",gsmInitCurrentCmdIndex,gsmResp);
            gsmRespRetVal   = GSM_RESP_CB_ERROR_COMPLETE;   
            // this is done to inform the init handler that the cmd response is received as it is 
            // waiting for the reply
            gsmInitAtCmdState = AT_CMD_FAILURE;
        break;
        
        case RESP_IGNORE:
            gsmRespRetVal = GSM_RESP_CB_WAIT;    //+QNITZ/+CCLK URC is rcvd then ignore it.
        break;
        
        case RESP_OTHER: 
            gsmRespRetVal = GSM_RESP_CB_WAIT;    
        break;
        
        default:
        break;
    }
    
    return gsmRespRetVal;
}

void GsmInitClearCmdFlag(void)
{
    uint8_t i;
    for(i = 0; i < gsmInitTotalCmds; i++)
    {
        gsmInitCmdList[i].flagCmdSuccess = 0;
    }
}

void GsmInitClearInitBuff(void)
{
   memset(imeiBuff,0,sizeof(imeiBuff));
   memset(ccidBuff,0,sizeof(ccidBuff));
}

/**
 *  @brief It is called before calling the init handler it sets the state and initialises all the init cmd variables
 *  @return none
 */
void GsmInitStart(void)
{
    gsmInitTotalCmds = NUM_ELEMS(gsmInitCmdList);
    gsmInitAtCmdState = AT_CMD_SEND;
    gsmInitCurrentCmdIndex = AT_CMD;
    gsmInitState = GSM_INIT_CMD_IN_PROGRESS;
    gsmInitCmdRetryCount = 0;
    flagCcidUpdated = 0;
    flagImeiUpdated = 0;
    //flagOperatorNameUpdated = 0;
    GetDataFirstTime = 1;
    GsmInitClearCmdFlag();
	  GsmResetGprsActTime();
    GsmSocketStateReset();
    //GsmReset10MinTick();
	  PacketHandlerStatesInit(); //packet handler state will be changed ti idle when modem restarts and socket open cmd will also be reset
    GsmInitClearInitBuff();
	  GsmInitClearCellDataBuff();
    LOG_DBGS(CH_GSM,"Init Cmd Start");
}

uint8_t* getIMEI(void)
{
  return(imeiBuff);
}

uint8_t* getCCID(void)
{
    return(ccidBuff);
}

/**
 *  @brief All the init commands are sent and acknowledge is checked in this function
 *  @return will return the state of the init command
 */
gsmInitCmdState_et GsmInitHandler(void)
{
      switch(gsmInitState)
      {
        case GSM_INIT_CMD_WAIT:
            
        break;
        
        case GSM_INIT_CMD_IN_PROGRESS:
            switch(gsmInitAtCmdState)
            {
                case AT_CMD_SEND:
                    // check whether is at command is idle
                    if(GsmIsAtIdle())
                    {
                        if(gsmInitCmdList[gsmInitCurrentCmdIndex].flagCmdSuccess == 0)
                        {
                            // send the at command
                            GsmSendAtCmd(gsmInitCmdList[gsmInitCurrentCmdIndex].cmd,strlen((const char *)gsmInitCmdList[gsmInitCurrentCmdIndex].cmd),
                                    gsmInitCmdList[gsmInitCurrentCmdIndex].cmdTimeoutInMs,gsmInitCmdList[gsmInitCurrentCmdIndex].respCb);
                            gsmInitAtCmdState = AT_WAIT_REPLY;
                        }
                        else
                        {
                            gsmInitAtCmdState = AT_CMD_SUCCESS;
                        }
                    }
                break;
                
                case AT_WAIT_REPLY:
                    // the callback function will change the state of the initAtState
                break;
                
                case AT_CMD_SUCCESS:
                    LOG_DBG(CH_GSM,"Init Cmd %d Success",gsmInitCurrentCmdIndex);
                    gsmInitCmdList[gsmInitCurrentCmdIndex].flagCmdSuccess = 1;
                    //(gsmInitCMD_et)gsmInitCurrentCmdIndex++;
								     gsmInitCurrentCmdIndex++;
                    gsmInitCmdRetryCount = 0;
                    if(gsmInitCurrentCmdIndex >= gsmInitTotalCmds)
                    {
                        gsmInitState = GSM_INIT_CMD_SUCCESS;
                    }
                    else
                    {
                        gsmInitAtCmdState = AT_CMD_SEND;
                    }
                break;
                
                case AT_CMD_FAILURE:
                    gsmInitCmdRetryCount++;
                    if(gsmInitCmdRetryCount < gsmInitCmdList[gsmInitCurrentCmdIndex].numRetries)
                    {
                        gsmInitRetryCmdTick = GetStartTime();
                        gsmInitAtCmdState = AT_CMD_RETRY_WAIT; 
                    }
                    else
                    {
                        // maximum retry is done so set the cmd to failure
                        gsmInitState = GSM_INIT_CMD_FAIL;
                    }
                break;
                
                case AT_CMD_RETRY_WAIT:
                    if(TimeSpent(gsmInitRetryCmdTick,gsmInitCmdList[gsmInitCurrentCmdIndex].retryWaitIntervalInIms))
                    {
                        gsmInitAtCmdState = AT_CMD_SEND;
                    }
                break;
            }
        break;
        
        case GSM_INIT_CMD_FAIL:
            
        break;
        
        case GSM_INIT_CMD_SUCCESS:
            
        break;
    }
    return gsmInitState;
}

void GsmInitUpdateModuleReady(void)
{
    gsmInitCmdList[QINISTAT_CMD].flagCmdSuccess = 1;
}

void GsmInitUpdateSimReady(void)
{
    gsmInitCmdList[CPIN_CMD].flagCmdSuccess = 1;
}

void GsmInitUpdateSimNotReady(void)
{
    gsmInitCmdList[CPIN_CMD].flagCmdSuccess = 0;
}

void GsmInitFunctionalityReady(void)
{
    gsmInitCmdList[CFUN_CMD].flagCmdSuccess = 1;
}

void GsmInitFunctionalityNotReady(void)
{
    gsmInitCmdList[CFUN_CMD].flagCmdSuccess = 0;
}


void GetIMEINumber(uint8_t* imeiDataArray)
{
      if(flagImeiUpdated == 1)
      {
         strncpy((char*)imeiDataArray,(const char*)imeiBuff,IMEI_BUFF_SIZE);
      }
      else
      {
         snprintf((char*)imeiDataArray,sizeof(imeiDataArray),"%s","0");
      }
//    return(imeiBuff);
}

void GetCCIDNumber(uint8_t* ccidDataArray)
{
     if(flagCcidUpdated == 1)
     {
       strncpy((char*)ccidDataArray,(const char*)ccidBuff,CCID_BUFF_SIZE);
     }
     else
     {
       snprintf((char*)ccidDataArray,sizeof(ccidDataArray),"%s","0");
     }
//  return(ccidBuff);
}