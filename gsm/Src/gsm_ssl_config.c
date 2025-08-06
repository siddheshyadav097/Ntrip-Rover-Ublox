#include "gsm_ssl_config.h"

sslConfigState_et sslConfigState = SSL_CONFIG_WAIT_FOR_START;
gsmCmdState_et sslConfigAtCmdState = AT_CMD_SEND;
sslConfigCmd_et sslConfigCmdIndex = CONFIG_SSL_VERSION;

gsmRespCb_et SslOkRespCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len);

sslConfigCmd_st sslConfigCmdList[]= {
    {"AT+QSSLCFG=\"sslversion\",0,3\r",SSL_CONFIG_CMD_DEFAULT_TIMEOUT,SslOkRespCb},
    {"AT+QSSLCFG=\"seclevel\",0,0\r",SSL_CONFIG_CMD_DEFAULT_TIMEOUT,SslOkRespCb},
    {"AT+QSSLCFG=\"ciphersuite\",0,\"0XFFFF\"\r",SSL_CONFIG_CMD_DEFAULT_TIMEOUT,SslOkRespCb},
    {"AT+QSSLCFG=\"ignorertctime\",1\r",SSL_CONFIG_CMD_DEFAULT_TIMEOUT,SslOkRespCb},
};

void GsmSslConfigStart(void)
{
    //TODO check whether ssl configuration is required
    sslConfigState = SSL_CONFIG_IN_PROGRESS;
    sslConfigAtCmdState = AT_CMD_SEND;
    sslConfigCmdIndex = CONFIG_SSL_VERSION;
    LOG_DBGS(CH_GPRS,"SSL config start");
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
gsmRespCb_et SslOkRespCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len)
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
            sslConfigAtCmdState = AT_CMD_SUCCESS;
        break;
        
        case RESP_ERROR:
        case RESP_CME_ERROR:
        case RESP_TIMEOUT:
            LOG_ERR(CH_GPRS,"SSL config cmd :%d errcode :%d",sslConfigCmdIndex,gsmResp);
            gsmRespRetVal   = GSM_RESP_CB_ERROR_COMPLETE;   
            // this is done to inform the init handler that the cmd response is received as it is 
            // waiting for the reply
            sslConfigAtCmdState = AT_CMD_FAILURE;
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

sslConfigState_et GsmSslConfigHandler(void)
{
    switch(sslConfigState)
    {
        case SSL_CONFIG_WAIT_FOR_START:
            
        break;
        
        case SSL_CONFIG_IN_PROGRESS:
            switch(sslConfigAtCmdState)
            {
                case AT_CMD_SEND:
                    // check whether is at command is idle
                    if(GsmIsAtIdle())
                    {
                        // send the at command
                        // check whether if the command is 
                        GsmSendAtCmd(sslConfigCmdList[sslConfigCmdIndex].cmd,strlen((const char *)sslConfigCmdList[sslConfigCmdIndex].cmd),
                                sslConfigCmdList[sslConfigCmdIndex].cmdTimeoutInMs,sslConfigCmdList[sslConfigCmdIndex].respCb);
                        sslConfigAtCmdState = AT_WAIT_REPLY;
                    }
                break;
                
                case AT_WAIT_REPLY:
                    // the callback function will change the state of the initAtState
                break;
                
                case AT_CMD_SUCCESS:
                    LOG_DBG(CH_GPRS,"SSL config %d Success",sslConfigCmdIndex);
                    //(sslConfigCmd_et)sslConfigCmdIndex++;
								    sslConfigCmdIndex++;
                    if(sslConfigCmdIndex >= SSL_NUM_CMDS)
                    {
                        sslConfigState = SSL_CONFIG_SUCCESS;
                    }
                    else
                    {
                        sslConfigAtCmdState = AT_CMD_SEND;
                    }
                break;
                
                case AT_CMD_FAILURE:
                    sslConfigState = SSL_CONFIG_FAIL;
                break;
                
                case AT_CMD_RETRY_WAIT:
                    
                break;
            }
        break;
        
        case SSL_CONFIG_SUCCESS:
            
        break;

        case SSL_CONFIG_FAIL:
            
        break;
    }
    
    return sslConfigState;
}