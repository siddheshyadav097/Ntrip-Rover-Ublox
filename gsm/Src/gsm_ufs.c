#include "gsm_ufs.h"
#include <string.h>
#include <stdlib.h>

// common variables used by gsm ufs 
gsmCmdState_et gsmFileAtCmdState = AT_CMD_SEND;
char gsmFileCmdBuff[GSM_FILE_CMD_BUFF_SIZE];
uint32_t gsmFileCmdTimeout = 0;

/******* GSM FILE OPEN CMD variables ****************/
const uint8_t gsmFileOpenCmdPrefix[] = "AT+QFOPEN=";
int gsmFileHandle = 0;
gsmFileOpenState_et gsmFileOpenState = GSM_FILE_OPEN_WAIT_FOR_START;

/******* GSM FILE READ CMD variables ****************/
const uint8_t gsmFileReadCmdPrefix[] = "AT+QFREAD=";
GsmFileReadByteCbFnPtr FileReadCb = NULL;
gsmFileReadState_et gsmFileReadState = GSM_FILE_READ_WAIT_FOR_START;
uint16_t gsmFileReadLen = 0;

/******* GSM FILE CLOSE CMD variables ****************/
const uint8_t gsmFileCloseCmdPrefix[] = "AT+QFCLOSE=";
gsmFileCloseState_et gsmFileCloseState = GSM_FILE_CLOSE_WAIT_FOR_START;

gsmRespCb_et GsmFileRespCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len)
{
    //what we want to return from the calllback depending on the reponse received
    gsmRespCb_et gsmRespRetVal = GSM_RESP_CB_WAIT;
    switch(gsmResp)
    {
        case RESP_OK:
            // check whether the current command is 
            gsmRespRetVal = GSM_RESP_CB_OK_COMPLETE;
            gsmFileAtCmdState = AT_CMD_SUCCESS;
        break;
        
        // all response of used by file handler
        case RESP_FILE_OPEN:
            // "+QFOPEN: "offset is 9
            gsmFileHandle =  atoi((const char *)lineBuff + 9);
            if(gsmFileHandle < 0)
            {
                LOG_DBG(CH_UFS,"File open errorcode : %d",gsmFileHandle);
                //FtpGetErrorString((FTPCmdReply_et)ftpRetCode);
            }
            gsmRespRetVal = GSM_RESP_CB_WAIT;
        break;
        
        case RESP_FILE_READ:
            //"CONNECT "  offset is 8
            gsmFileReadLen =  atoi((const char *)lineBuff + 8);
            if(gsmFileReadLen > 0)
            {
                // set the state to fixed length reception mode
                //GsmReadFixedLengthData(FileReadCb,gsmFileReadLen);
								GsmReadFixedLengthDataIndexed(0, FileReadCb, gsmFileReadLen);
            }
            else
            {
                LOG_DBG(CH_UFS,"File read error Length : %d",gsmFileReadLen);
            }
            gsmRespRetVal = GSM_RESP_CB_WAIT;
        break;
        

        case RESP_ERROR:
        case RESP_CME_ERROR:
        case RESP_TIMEOUT:
        case RESP_OTHER: 
            LOG_ERR(CH_UFS,"%s failure errcode :%d",gsmFileCmdBuff, gsmResp);
            gsmRespRetVal = GSM_RESP_CB_ERROR_COMPLETE;
            gsmFileAtCmdState = AT_CMD_FAILURE;
        break;
        
        case RESP_IGNORE: 
            gsmRespRetVal = GSM_RESP_CB_WAIT;
        break;
    }
    return gsmRespRetVal;
}

// will return 1 when the file open command is sent successfully
uint8_t GsmFileOpenStart(char *fileName, fileMode_et mode)
{
    // check whether the gsm state is in idle
    if(GsmStateIsIdle() || GsmStateIsFileProcessing())
    {
        //create the command buff 
        snprintf((char *)gsmFileCmdBuff, sizeof(gsmFileCmdBuff),"%s\"%s\",%d\r",gsmFileOpenCmdPrefix,fileName,mode);
        gsmFileOpenState = GSM_FILE_OPEN_IN_PROGRESS;
        gsmFileAtCmdState = AT_CMD_SEND;
        GsmSetState(GSM_FILE_PROCESSING);
        gsmFileCmdTimeout = GSM_FILE_OPEN_CMD_TIMEOUT;
        return 1;
    }
    return 0;
}

void GsmFileAtCmdHandler(void)
{
    switch(gsmFileAtCmdState)
    {
        case AT_CMD_SEND:
            // check whether is at command is idle
            if(GsmIsAtIdle())
            {
                // send the at command
                // depending upon the state of the
                GsmSendAtCmd(gsmFileCmdBuff,strlen((const char *)gsmFileCmdBuff),
                        gsmFileCmdTimeout,GsmFileRespCb);
                gsmFileAtCmdState = AT_WAIT_REPLY;
            }
        break;
        
        case AT_WAIT_REPLY:
            // the callback function will change the state of the initAtState
        break;
        
        case AT_CMD_SUCCESS:
            //GsmSetState(GSM_MODEM_IDLE);
        break;
        
        case AT_CMD_FAILURE:
            //GsmSetState(GSM_MODEM_IDLE);
        break;
        
        case AT_CMD_RETRY_WAIT:
            
        break;
    }
}

gsmFileOpenState_et GsmFileOpenHandler(void)
{
    switch(gsmFileOpenState)
    {
        case GSM_FILE_OPEN_WAIT_FOR_START:
        break;
        
        case GSM_FILE_OPEN_IN_PROGRESS:
            GsmFileAtCmdHandler();
            if(gsmFileAtCmdState == AT_CMD_SUCCESS)
            {
                gsmFileOpenState = GSM_FILE_OPEN_SUCCESS;
            }
            else if(gsmFileAtCmdState == AT_CMD_FAILURE)
            {
                gsmFileOpenState = GSM_FILE_OPEN_FAIL;
            }
        break;
        
        case GSM_FILE_OPEN_SUCCESS:
        break;
        
        case GSM_FILE_OPEN_FAIL:
        break;
    }
    return gsmFileOpenState;
}

int GsmGetFileHandle(void)
{
    return gsmFileHandle;
}

uint8_t GsmFileReadStart(GsmFileReadByteCbFnPtr fileReadByteCb,uint16_t readLen)
{
    //check the file open state if it is success and in file processing and the file is opened successfully 
    if(gsmFileOpenState == GSM_FILE_OPEN_SUCCESS)
    {
        //create the command buff 
        snprintf((char *)gsmFileCmdBuff, sizeof(gsmFileCmdBuff),"%s%d,%d\r",gsmFileReadCmdPrefix,gsmFileHandle,readLen);
        FileReadCb = fileReadByteCb;
        gsmFileReadState = GSM_FILE_READ_IN_PROGRESS;
        gsmFileAtCmdState = AT_CMD_SEND;
        gsmFileCmdTimeout = GSM_FILE_READ_CMD_TIMEOUT;
        return 1;
    }
    return 0;
}

gsmFileReadState_et GsmFileReadHandler(void)
{
    switch(gsmFileReadState)
    {
        case GSM_FILE_READ_WAIT_FOR_START:
        break;
        
        case GSM_FILE_READ_IN_PROGRESS:
            GsmFileAtCmdHandler();
            if(gsmFileAtCmdState == AT_CMD_SUCCESS)
            {
                gsmFileReadState = GSM_FILE_READ_SUCCESS;
            }
            else if(gsmFileAtCmdState == AT_CMD_FAILURE)
            {
                gsmFileReadState = GSM_FILE_READ_FAIL;
            }
        break;
        
        case GSM_FILE_READ_SUCCESS:
        break;
        
        case GSM_FILE_READ_FAIL:
        break;
    }
    return gsmFileReadState;
}


uint8_t GsmFileCloseStart(void)
{
    //check the file open state if it is success and in file processing and the file is opened successfully 
    if(gsmFileOpenState == GSM_FILE_OPEN_SUCCESS)
    {
        //create the command buff 
        snprintf((char *)gsmFileCmdBuff, sizeof(gsmFileCmdBuff),"%s%d\r",gsmFileCloseCmdPrefix,gsmFileHandle);
        gsmFileCloseState = GSM_FILE_CLOSE_IN_PROGRESS;
        gsmFileAtCmdState = AT_CMD_SEND;
        gsmFileCmdTimeout = GSM_FILE_CLOSE_CMD_TIMEOUT;
        return 1;
    }
    return 0;
}

gsmFileCloseState_et GsmFileCloseHandler(void)
{
    switch(gsmFileCloseState)
    {
        case GSM_FILE_CLOSE_WAIT_FOR_START:
            
        break;
        
        case GSM_FILE_CLOSE_IN_PROGRESS:
            GsmFileAtCmdHandler();
            if(gsmFileAtCmdState == AT_CMD_SUCCESS)
            {
                gsmFileCloseState = GSM_FILE_CLOSE_SUCCESS;
                gsmFileOpenState = GSM_FILE_OPEN_WAIT_FOR_START;
                //GsmSetState(GSM_MODEM_IDLE);
            }
            else if(gsmFileAtCmdState == AT_CMD_FAILURE)
            {
                gsmFileCloseState = GSM_FILE_CLOSE_FAIL;
                gsmFileOpenState = GSM_FILE_OPEN_WAIT_FOR_START;
                //GsmSetState(GSM_MODEM_IDLE);
            }
        break;
        
        case GSM_FILE_CLOSE_SUCCESS:
            
        break;
        
        case GSM_FILE_CLOSE_FAIL:
        break;
    }
    return gsmFileCloseState;
}

