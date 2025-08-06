#include "gsm_ftp_api.h"
#include <string.h>
#include <stdlib.h>


// the current ftp command is in progress
ftpCmd_et ftpCurCmd = DELETE_UFS_FILES;
// FTP at command state
gsmCmdState_et ftpAtCmdState = AT_CMD_SEND;
// ftp handler state
gsmFtpState_et ftpState = FTP_WAIT_FOR_START;

// // buffer to store FTP user name
// uint8_t ftpUserNameCmdBuf[FTP_USER_NAME_CMD_SIZE];
// // buffer to store FTP password command
// uint8_t ftpPasswordCmdBuf[FTP_PASSWORD_CMD_SIZE];
// // buffer to store FTP open command 
// uint8_t ftpOpenCmdBuff[FTP_OPEN_CMD_SIZE];
// // buffer to store FTP path command
// uint8_t ftpPathCmdBuff[FTP_PATH_CMD_SIZE];
// // buffer to store FTP size command
// uint8_t ftpFileSizeCmdBuff[FTP_FILE_CMD_SIZE];
// // buffer to store FTP size command
// uint8_t ftpGetCmdBuff[FTP_GET_CMD_SIZE];

uint8_t ftpCmdBuff[FTP_CMD_BUFF_SIZE];
// ufs free size
int ufsFreeSize = 0;
// file downloaded size
int ftpDownloadedFileSize = 0;

// ftp file struct holding all the file details
ftpFile_st ftpFile;

FtpDownloadStatusCbFnPtr FtpStatusCb = NULL;
// const char ftpUserNameCmdPrefix[] = "AT+QFTPUSER=";
// const char ftpPasswordCmdPrefix[] = "AT+QFTPPASS=";
// const char ftpOpenCmdPrefix[] = "AT+QFTPOPEN=";
// const char ftpPathCmdPrefix[] = "AT+QFTPPATH=";
// const char ftpFileSizeCmdPrefix[] = "AT+QFTPSIZE=";

/**
 *  @brief     :This AtCmdReplyCb() defualt callback for the commands expecting OK or ERROR.
 *  @Inputs:   gsmResp of type enum  gsmResPonse_et
 *             lineBuff - Pointer to the buffer in which modem response(processlinebuffer - Extended Response or SerialLineBuff)is received.
 *             len      - Length of the data received.
 *  @param     [out] gsmRespCb_et.
 *  @return    1 value of enum @ref gsmRespCb_et        
 *             :GSM_RESP_CB_COMPLETE if OK received or 
 *             GSM_RESP_CB_ERROR_COMPLETE if ERROR is received. */
gsmRespCb_et FtpOkRespCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len)
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
            ftpAtCmdState = AT_CMD_SUCCESS;
        break;
        
        case RESP_ERROR:
        case RESP_CME_ERROR:
        case RESP_TIMEOUT:
            LOG_ERR(CH_FTP,"FTP Cmd:%d errcode :%d",ftpCurCmd,gsmResp);
            gsmRespRetVal = GSM_RESP_CB_ERROR_COMPLETE;   
            // this is done to inform the init handler that the cmd response is received as it is 
            // waiting for the reply
            ftpAtCmdState = AT_CMD_FAILURE;
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

void FtpGetErrorString(FTPCmdReply_et errVal)
{
    switch(errVal)
    {
        case 0:
          LOG_ERRS(CH_FTP,"FTP Result = FTP_SUCCESS\r\n");
          break;
        case -1:
          LOG_ERRS(CH_FTP,"FTP Result = UNKNOWN_ERROR\r\n");
          break;
        case -3:
          LOG_ERRS(CH_FTP,"FTP Result = FTP_SERVICE_BUSY\r\n");
          break;
        case -4:
          LOG_ERRS(CH_FTP,"FTP Result = FAIL_TO_GET_IP\r\n");
          break;  
        case -5:
          LOG_ERRS(CH_FTP,"FTP Result = FAIL_TO_SEND_CMD\r\n");
          break;
        case -6:
          LOG_ERRS(CH_FTP,"FTP Result = SESSION_CLOSED_BY_SERVER\r\n");
          break;
        case -7:
          LOG_ERRS(CH_FTP,"FTP Result = DATA_CONN_CLOSED_BY_SERVER\r\n");
          break;   
        case -8:
          LOG_ERRS(CH_FTP,"FTP Result = GPRS_CONTEXT_DEACTIVATED\r\n");
          break;
        case -9:
          LOG_ERRS(CH_FTP,"FTP Result = TIMEOUT\r\n");
          break;
        case -10:
          LOG_ERRS(CH_FTP,"FTP Result = INPUT_PARA_ILLEGAL\r\n");
          break;  
        case -11:
          LOG_ERRS(CH_FTP,"FTP Result = FILE_NOT_FOUND\r\n");
          break;
        case -12:
          LOG_ERRS(CH_FTP,"FTP Result = FAIL_TO_GET_FILE\r\n");
          break;
        case -13:
          LOG_ERRS(CH_FTP,"FTP Result = NO_ENOUGH_MEMORY_ON_GSM\r\n");
          break;     
        case -421:
          LOG_ERRS(CH_FTP,"FTP Result = FTP_SERVICE_NOT_SUPPORTED\r\n");
          break;
        case -425:
          LOG_ERRS(CH_FTP,"FTP Result = FAIL_TO_OPEN_DATA_CONN\r\n");
          break;
        case -426:
          LOG_ERRS(CH_FTP,"FTP Result = CONN_CLOSED_STOPPED_TRANSFER\r\n");
          break;  
        case -450:
          LOG_ERRS(CH_FTP,"FTP Result = FILE_REQUEST_NOT_OPERATED\r\n");
          break;
        case -452:
          LOG_ERRS(CH_FTP,"FTP Result = NO_ENOUGH_MEMORY_ON_SERVER\r\n");
          break;
        case -500:
          LOG_ERRS(CH_FTP,"FTP Result = WRONG_FTP_CMD_FORMAT\r\n");
          break;   
        case -501:
          LOG_ERRS(CH_FTP,"FTP Result = WRONG_FTP_CMD_PARAMETER\r\n");
          break;
        case -502:
            LOG_ERRS(CH_FTP,"FTP Result = FTP_CMD_NOT_OPERATED_BY_SERVER\r\n");
        break;
        case -530:
            LOG_ERRS(CH_FTP,"FTP Result = LOGIN_ERROR_ON_SERVER\r\n");
        break;
        
        case -532:
            LOG_ERRS(CH_FTP,"FTP Result = NEED_ACCOUNT_INFO\r\n");
        break;
        
        case -550:
            LOG_ERRS(CH_FTP,"FTP Result = REQUEST_NOT_OPERATED\r\n");
        break;
          
        case -551:
            LOG_ERRS(CH_FTP,"FTP Result = REQUEST_STOPPED\r\n");
        break;  
        
        case -552:
            LOG_ERRS(CH_FTP,"FTP Result = FILE_REQUEST_STOPPED\r\n");
        break;
        
        case -553:
            LOG_ERRS(CH_FTP,"FTP Result = ILLEGAL_FILENAME\r\n");
        break;    
    }
}

gsmRespCb_et FtpProcessRespCb(gsmCmdResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len)
{
    int ftpRetCode;
    char *p = NULL;
    //what we want to return from the calllback depending on the reponse received
    gsmRespCb_et gsmRespRetVal = GSM_RESP_CB_WAIT;
    switch(gsmResp)
    {
        case RESP_OK:
            // check whether the current command is 
            if(ftpCurCmd == GET_UFS_FREE_SPACE)
            {
                gsmRespRetVal = GSM_RESP_CB_OK_COMPLETE;
                ftpAtCmdState = AT_CMD_SUCCESS;
            }
            else
            {
                gsmRespRetVal = GSM_RESP_CB_WAIT;
            }
        break;
        
        case RESP_FTP_CONFIG:
            // "+QFTPCFG:" - offset is 9
            ftpRetCode = atoi((const char *)(lineBuff + 9));
            if(ftpRetCode == FTP_SUCCESS)
            {
                gsmRespRetVal = GSM_RESP_CB_OK_COMPLETE;
                ftpAtCmdState = AT_CMD_SUCCESS;
            }
            else
            {
                FtpGetErrorString((FTPCmdReply_et)ftpRetCode);
                gsmRespRetVal = GSM_RESP_CB_ERROR_COMPLETE; 
                ftpAtCmdState = AT_CMD_FAILURE;
            }
        break;
        
        case RESP_FTP_OPEN:
            // "+QFTPOPEN:" offset is 10
            ftpRetCode = atoi((const char *)(lineBuff + 10));
            if(ftpRetCode == FTP_SUCCESS)
            {
                gsmRespRetVal = GSM_RESP_CB_OK_COMPLETE;
                ftpAtCmdState = AT_CMD_SUCCESS;
            }
            else
            {
                FtpGetErrorString((FTPCmdReply_et)ftpRetCode);
                gsmRespRetVal = GSM_RESP_CB_ERROR_COMPLETE; 
                ftpAtCmdState = AT_CMD_FAILURE;
            }
        break;
        
        case RESP_FTP_PATH:
            // "+QFTPPATH:" offset is 10
            ftpRetCode = atoi((const char *)(lineBuff + 10));
            if(ftpRetCode == FTP_SUCCESS)
            {
                gsmRespRetVal = GSM_RESP_CB_OK_COMPLETE;
                ftpAtCmdState = AT_CMD_SUCCESS;
            }
            else
            {
                FtpGetErrorString((FTPCmdReply_et)ftpRetCode);
                gsmRespRetVal = GSM_RESP_CB_ERROR_COMPLETE; 
                ftpAtCmdState = AT_CMD_FAILURE;
            }
        break;
        
        case RESP_FTP_SIZE:
            //  "+QFTPSIZE:" offset is 10
            ftpRetCode = atoi((const char *)(lineBuff + 10));
            if(ftpRetCode >= 0)
            {
                ftpFile.fileSize = ftpRetCode;
                gsmRespRetVal = GSM_RESP_CB_OK_COMPLETE;
                ftpAtCmdState = AT_CMD_SUCCESS;
            }
            else
            {
                FtpGetErrorString((FTPCmdReply_et)ftpRetCode);
                gsmRespRetVal = GSM_RESP_CB_ERROR_COMPLETE; 
                ftpAtCmdState = AT_CMD_FAILURE;
            }
        break;
        
        case RESP_FTP_GETFILE:
            //  "+QFTPGET:" offset is 9
            ftpRetCode = atoi((const char *)(lineBuff + 9));
            if(ftpRetCode >= 0)
            {
                ftpDownloadedFileSize = ftpRetCode;
                //TODO check whether the downloaded file size is same as the file size
                gsmRespRetVal = GSM_RESP_CB_OK_COMPLETE;
                ftpAtCmdState = AT_CMD_SUCCESS;
            }
            else
            {
                FtpGetErrorString((FTPCmdReply_et)ftpRetCode);
                gsmRespRetVal = GSM_RESP_CB_ERROR_COMPLETE; 
                ftpAtCmdState = AT_CMD_FAILURE;
            }
        break;
        
        case RESP_FTP_CLOSE:
            // "+QFTPCLOSE:" offset is 11
            ftpRetCode = atoi((const char *)(lineBuff + 11));
            if(ftpRetCode == FTP_SUCCESS)
            {
                gsmRespRetVal = GSM_RESP_CB_OK_COMPLETE;
                ftpAtCmdState = AT_CMD_SUCCESS;
            }
            else
            {
                FtpGetErrorString((FTPCmdReply_et)ftpRetCode);
                gsmRespRetVal = GSM_RESP_CB_ERROR_COMPLETE; 
                ftpAtCmdState = AT_CMD_FAILURE;
            }
        break;
        
        // all response of used by file handler
        case RESP_UFS_SIZE:
            // "+QFLDS: " offset is 8
            // parse the ufs free size by checking for ,
            p = strchr((const char *)lineBuff, ',');
            // replace the , with null character
            if(p)
            {
                *p = '\0';
                ufsFreeSize =  atoi((const char *)lineBuff + 8);
                if(ufsFreeSize < 0)
                {
                    FtpGetErrorString((FTPCmdReply_et)ftpRetCode);
                }
            }
            gsmRespRetVal = GSM_RESP_CB_WAIT;
        break;

        case RESP_ERROR:
        case RESP_CME_ERROR:
        case RESP_TIMEOUT:
        case RESP_OTHER: 
            LOG_ERR(CH_FTP,"FTP Cmd:%d errcode :%d",ftpCurCmd,gsmResp);
            gsmRespRetVal = GSM_RESP_CB_ERROR_COMPLETE;
            ftpAtCmdState = AT_CMD_FAILURE;
        break;
        
        case RESP_IGNORE: 
            gsmRespRetVal = GSM_RESP_CB_WAIT;
        break;
    }
    return gsmRespRetVal;
}


gsmFtpCmd_st ftpCmdList[]= {
    {"AT+QFDEL=\"*\"\r",GSM_FTP_CMD_TIMEOUT,FtpOkRespCb},
    {"AT+QFLDS=\"UFS\"\r",GSM_FTP_CMD_TIMEOUT,FtpProcessRespCb},
    {"AT+QFTPUSER=",GSM_FTP_DEFAULT_CMD_TIMEOUT,FtpOkRespCb},
    {"AT+QFTPPASS=",GSM_FTP_DEFAULT_CMD_TIMEOUT,FtpOkRespCb},
    {"AT+QFTPCFG=4,\"/UFS/firmware.txt\"\r",GSM_FTP_CMD_TIMEOUT,FtpProcessRespCb},
    {"AT+QFTPOPEN=",GSM_FTP_OPEN_CMD_TIMEOUT,FtpProcessRespCb},
    {"AT+QFTPPATH=",GSM_FTP_CMD_TIMEOUT,FtpProcessRespCb},
    {"AT+QFTPSIZE=",GSM_FTP_OPEN_CMD_TIMEOUT,FtpProcessRespCb},
    {"AT+QFTPGET=",GSM_FTP_FILE_DW_TIMEOUT,FtpProcessRespCb},
    {"AT+QFTPCLOSE\r",GSM_FTP_CLOSE_CMD_TIMEOUT,FtpProcessRespCb},
};

// the ftp start download to be called only if the gsm state is idle
uint8_t FtpStartDownload(uint8_t *urlString, FtpDownloadStatusCbFnPtr statusCb)
{
    if(GsmStateIsIdle())
    {
        if(FtpUrlDecode(urlString,&ftpFile))
        {
            // change the state of the gsm
            // update all the command buff
            ftpCurCmd = DELETE_UFS_FILES;
            ftpDownloadedFileSize = 0;
            FtpStatusCb = statusCb;
            GsmSetState(GSM_FTP_PROCESSING);
            ftpAtCmdState = AT_CMD_SEND;
            ftpState = FTP_DOWNLOAD_IN_PROGRESS;
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
}

uint8_t* FtpGetCurrentCmd(void)
{
    uint8_t* cmd = ftpCmdList[ftpCurCmd].cmd;
    switch(ftpCurCmd)
    {
        case DELETE_UFS_FILES:
        break;

        case GET_UFS_FREE_SPACE:
        break;

        case SET_FTP_USERNAME:
            sprintf((char *)ftpCmdBuff, "%s\"%s\"\r",ftpCmdList[SET_FTP_USERNAME].cmd,ftpFile.userName);
            cmd = ftpCmdBuff;
        break;

        case SET_FTP_PASSWORD:
            sprintf((char *)ftpCmdBuff, "%s\"%s\"\r",ftpCmdList[SET_FTP_PASSWORD].cmd,ftpFile.password);
            cmd = ftpCmdBuff;
        break;

        case SET_FTP_CONFIG:
            
        break;
        
        case OPEN_FTP_PORT:
            sprintf((char *)ftpCmdBuff, "%s\"%s\",%d\r",ftpCmdList[OPEN_FTP_PORT].cmd,ftpFile.host, ftpFile.portNum);
            cmd = ftpCmdBuff;
        break;
        
        case SET_FILE_PATH:
            sprintf((char *)ftpCmdBuff, "%s\"%s\"\r",ftpCmdList[SET_FILE_PATH].cmd,ftpFile.filePath);
            cmd = ftpCmdBuff;
        break;
        
        case GET_FILE_SIZE:
            sprintf((char *)ftpCmdBuff, "%s\"%s\"\r",ftpCmdList[GET_FILE_SIZE].cmd,ftpFile.fileName);
            cmd = ftpCmdBuff;
        break;
        
        case GET_FTP_FILE:
            sprintf((char *)ftpCmdBuff, "%s\"%s\",%d\r",ftpCmdList[GET_FTP_FILE].cmd,ftpFile.fileName, ftpFile.fileSize);
            cmd = ftpCmdBuff;
        break;
    }
    return cmd;
}

void GsmFtpHandler(void)
{
    uint8_t *cmd;
    switch(ftpState)
    {
        case FTP_WAIT_FOR_START:
        break;
        
        case FTP_DOWNLOAD_IN_PROGRESS:
            switch(ftpAtCmdState)
            {
                case AT_CMD_SEND:
                    // check whether is at command is idle
                    if(GsmIsAtIdle())
                    {
                        // send the at command
                        // check whether if the command is 
                        cmd = FtpGetCurrentCmd();
                        GsmSendAtCmd(cmd,strlen((const char *)cmd),
                                ftpCmdList[ftpCurCmd].cmdTimeoutInMs,ftpCmdList[ftpCurCmd].respCb);
                        ftpAtCmdState = AT_WAIT_REPLY;
                    }
                break;
                
                case AT_WAIT_REPLY:
                    // the callback function will change the state of the initAtState
                break;
                
                case AT_CMD_SUCCESS:
                     LOG_DBG(CH_FTP,"FTP Cmd %d success",ftpCurCmd);
                    (ftpCmd_et)ftpCurCmd++;
                    if(ftpCurCmd >= NUM_FTP_CMDS)
                    {
                        ftpState = FTP_DOWNLOAD_SUCCESS;
                        GsmSetState(GSM_MODEM_IDLE);
                        ftpAtCmdState = AT_CMD_SEND; 
                        if(ftpDownloadedFileSize == ftpFile.fileSize)
                        {
                            FtpStatusCb(FTP_STATUS_DOWNLOAD_SUCCESS,ftpDownloadedFileSize);
                        }
                        else
                        {
                            FtpStatusCb(FTP_STATUS_DOWNLOAD_FAIL,ftpDownloadedFileSize);
                        }
                    }
                    else
                    {
                        ftpAtCmdState = AT_CMD_SEND;
                    }
                break;
                
                case AT_CMD_FAILURE:
                    // check whether it is a delete command 
                    // if delete command then proceed to next command
                    LOG_DBG(CH_FTP,"FTP Cmd %d failure",ftpCurCmd);
                    if(ftpCurCmd != DELETE_UFS_FILES)
                    {
                        //check whether if it the last command
                        if(ftpCurCmd == SET_FILE_PATH || ftpCurCmd == GET_FILE_SIZE || ftpCurCmd == GET_FTP_FILE)
                        {
                            ftpCurCmd = CLOSE_FTP_PORT;
                            ftpAtCmdState = AT_CMD_SEND;
                        }
                        else if(ftpCurCmd == CLOSE_FTP_PORT)
                        {
                            if(ftpDownloadedFileSize == ftpFile.fileSize)
                            {
                                FtpStatusCb(FTP_STATUS_DOWNLOAD_SUCCESS,ftpDownloadedFileSize);
                            }
                            else
                            {
                                FtpStatusCb(FTP_STATUS_DOWNLOAD_FAIL,ftpDownloadedFileSize);
                            }
                            ftpState = FTP_DOWNLOAD_FAIL;
                            GsmSetState(GSM_MODEM_IDLE);
                        }
                        else
                        {
                            ftpState = FTP_DOWNLOAD_FAIL;
                            FtpStatusCb(FTP_STATUS_DOWNLOAD_FAIL,ftpDownloadedFileSize);
                            GsmSetState(GSM_MODEM_IDLE);
                        }
                        
                    }
                    else
                    {
                        (ftpCmd_et)ftpCurCmd++;
                        ftpAtCmdState = AT_CMD_SEND;
                    }
                break;
                
                case AT_CMD_RETRY_WAIT:
                    
                break;
            }
        break;
        
        case FTP_DOWNLOAD_SUCCESS:
            
        break;
        
        case FTP_DOWNLOAD_FAIL:
            
        break;
    }
}





