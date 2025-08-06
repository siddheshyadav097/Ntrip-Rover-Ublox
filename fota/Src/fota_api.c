#include "fota_api.h"
#include "packet_api.h"

fotaState_et fotaState = FOTA_WAIT_FOR_START;
uint8_t fotaUrl[FOTA_URL_SIZE];
uint32_t ftpDownloadFileSize;
uint32_t fotaReadFileSize = 0;
uint16_t fotaReadBuffIndex = 0;
uint8_t ftpDownloadCbCalled = 0;
ftpDownloadStatus_et ftpStatus = FTP_STATUS_DOWNLOAD_IN_PROG;
uint8_t fotaReadBuff[FOTA_FILE_READ_SIZE];
uint8_t fotaUpgradeStart = 0;
uint8_t fotaFlashVerify = 0;
fotaStatus_et fotaStatus = FOTA_FAIL;
//uint8_t fotaHeader[FOTA_HEADER_SIZE];
appHeader_st header;
uint16_t fileCalculatedChecksum = 0xffff;
appHeader_st* BackUpVerPtr = NULL; 
//used for the sending the FOTA response SMS 
char fotaAckBuff[100];
uint16_t fotaRespLen = 0;
uint8_t *fotaRespPhoneNum = NULL;
extern uint8_t flagFtpDecodeUrl;
uint32_t backCrc = 0;
uint32_t crcsize = 0;
void FotaStartFromFtp(uint8_t *urlString, uint8_t *phoneNum)
{
    // save the url to fotaUrlString and start the download
    strcpy((char *)fotaUrl,(const char *)urlString);
    // change the state from 'wait for start' to 'FTP start download'
    FotaSetState(FOTA_FTP_DOWNLOAD_START);
    fotaRespPhoneNum = phoneNum;
		//GsmCloseNtripSocket();
}

void FtpDownloadCompleteCb(ftpDownloadStatus_et status, uint32_t fileSize)
{
    ftpStatus = status;
    ftpDownloadFileSize = fileSize;
}

void FtpFileReadByteCb(uint8_t byte)
{
    uint8_t *headerPtr = (uint8_t *)&header;
    if(fotaState == FOTA_FTP_FILE_READ_HEADER_COMPLETE)
    {
        if(fotaReadBuffIndex < sizeof(appHeader_st))
        {
            headerPtr[fotaReadBuffIndex] = byte;
            fotaReadBuffIndex++;
        }
    }
    else
    {
        if(fotaReadBuffIndex < FOTA_FILE_READ_SIZE)
        {
            fotaReadBuff[fotaReadBuffIndex] = byte;
            fotaReadBuffIndex++;
        }
    }
    
}

const char *fotaRespString[20] = 
{
    "\nFOTA FAIL", 
    "\nFOTA FAIL STATE - TIMEOUT",
    "\nFOTA FAIL STATE - FTP DOWNLOAD FAILURE",
    "\nFOTA FAIL STATE - FILE OPEN FAILURE",
    "\nFOTA FAIL STATE - FILE HEADER READ FAILURE",
    "\nFOTA FAIL STATE - FILE HEADER INVALID",
    "\nFOTA FAIL STATE - FILE READ FAILURE",
    "\nFOTA FAIL STATE - FILE CHECKSUM FAILURE",
    "\nFOTA FAIL STATE - FILE FLASH VERIFY FAILURE",
    "\nFOTA SUCCESS",
    "\nFOTA FAIL STATE -  FOTA APP VERSION IS LOWER",
    "\nFOTA FAIL STATE -  FOTA INVALID PRODUCT ID",
    "\nFOTA FAIL STATE -  FOTA APP HEADER CRC ERROR",
    "\nFOTA FAIL STATE -  FOTA VERSION NUMBER CRC ERROR",
    "\nFOTA FAIL STATE -  GSM IS NOT IDLE",
    "\nFOTA FAIL STATE -  FOTA DECODE URL ERROR",
};

 void FotaGetProcessResp(fotaStatus_et fotaResult)
 {
    memset(fotaAckBuff, 0, sizeof(fotaAckBuff));
    
    if(fotaResult == FOTA_FAIL){snprintf((char *)fotaAckBuff, sizeof(fotaAckBuff),"%s",fotaRespString[FOTA_FAIL]);}
    else if(fotaResult == FOTA_FAIL_TIMEOUT){snprintf((char *)fotaAckBuff, sizeof(fotaAckBuff),"%s",fotaRespString[FOTA_FAIL_TIMEOUT]);}
    else if(fotaResult == FOTA_FAIL_FTP_DOWNLOAD){snprintf((char *)fotaAckBuff, sizeof(fotaAckBuff),"%s,FTP ERROR : %d ",fotaRespString[FOTA_FAIL_FTP_DOWNLOAD],ftpGetFileDwnldErrorCode());}
    else if(fotaResult == FOTA_FAIL_FILE_OPEN){snprintf((char *)fotaAckBuff, sizeof(fotaAckBuff),"%s",fotaRespString[FOTA_FAIL_FILE_OPEN]);}
    else if(fotaResult == FOTA_FAIL_READ_HEADER){snprintf((char *)fotaAckBuff, sizeof(fotaAckBuff),"%s",fotaRespString[FOTA_FAIL_READ_HEADER]);}
    else if(fotaResult == FOTA_FAIL_INVALID_HEADER){snprintf((char *)fotaAckBuff, sizeof(fotaAckBuff),"%s",fotaRespString[FOTA_FAIL_INVALID_HEADER]);}
    else if(fotaResult == FOTA_FAIL_READ_FILE){snprintf((char *)fotaAckBuff, sizeof(fotaAckBuff),"%s",fotaRespString[FOTA_FAIL_READ_FILE]);}
    else if(fotaResult == FOTA_FAIL_FILE_CHECKSUM){snprintf((char *)fotaAckBuff, sizeof(fotaAckBuff),"%s",fotaRespString[FOTA_FAIL_FILE_CHECKSUM]);}
    else if(fotaResult == FOTA_FAIL_FLASH_VERIFY){snprintf((char *)fotaAckBuff, sizeof(fotaAckBuff),"%s",fotaRespString[FOTA_FAIL_FLASH_VERIFY]);}
    else if(fotaResult == FOTA_SUCCESS){snprintf((char *)fotaAckBuff, sizeof(fotaAckBuff),"%s",fotaRespString[FOTA_SUCCESS]);}
    else if(fotaResult == FOTA_FAIL_GSM_IS_NOT_IDLE){snprintf((char *)fotaAckBuff, sizeof(fotaAckBuff),"%s",fotaRespString[FOTA_FAIL_GSM_IS_NOT_IDLE]);}
    else if(fotaResult == FOTA_FAIL_DECODE_URL_ERROR){snprintf((char *)fotaAckBuff, sizeof(fotaAckBuff),"%s",fotaRespString[FOTA_FAIL_DECODE_URL_ERROR]);}
    
    //errors for header invalid
    else if(fotaResult == FOTA_APP_VER_IS_LOWER)
    {
      BackUpVerPtr= QbootGetBackUpAPPVer(); 
      //snprintf((char *)fotaAckBuff, sizeof(fotaAckBuff),"%s,APP1 Ver : V%d",fotaRespString[FOTA_APP_VER_IS_LOWER],CODE_VERSION);
      snprintf((char *)fotaAckBuff, sizeof(fotaAckBuff),"%s,APP1 Ver : V%d.%d.%d ,APP2 Ver: V%d.%d.%d",\
       fotaRespString[FOTA_APP_VER_IS_LOWER],MAJOR_SW_VER,MINOR_SW_VER,MICRO_SW_VER,BackUpVerPtr->verNumMajor,BackUpVerPtr->verNumMinor,BackUpVerPtr->verNumMicro); 
    }
    else if(fotaResult == FOTA_INVALID_PRODUCT_ID){snprintf((char *)fotaAckBuff, sizeof(fotaAckBuff),"%s,APP PID : %d",fotaRespString[FOTA_INVALID_PRODUCT_ID],PRODUCT_ID);}
    else if(fotaResult == FOTA_APP_HEADER_CRC_ERROR){snprintf((char *)fotaAckBuff, sizeof(fotaAckBuff),"%s",fotaRespString[FOTA_APP_HEADER_CRC_ERROR]);}
    else if(fotaResult == FOTA_VER_NO_CRC_ERROR){snprintf((char *)fotaAckBuff, sizeof(fotaAckBuff),"%s",fotaRespString[FOTA_VER_NO_CRC_ERROR]);}
    
    fotaRespLen = strlen((const char*)fotaAckBuff);
}

void FotaSetState(fotaState_et state)
{
    if(fotaState == state)
    {
        return;
    }
    fotaState = state;
    switch(fotaState)
    {
        case FOTA_WAIT_FOR_START:
            LOG_DBGS(CH_FOTA, "fotaState - FOTA_WAIT_FOR_START");
        break;
        
        case FOTA_FTP_DOWNLOAD_START:
            LOG_DBGS(CH_FOTA, "fotaState - FOTA_FTP_DOWNLOAD_START");
            ftpStatus = FTP_STATUS_DOWNLOAD_IN_PROG;
            fotaReadFileSize = 0;
            fotaUpgradeStart = 0;
            fileCalculatedChecksum = 0xFFFF;
            fotaFlashVerify = 0;
            fotaStatus = FOTA_FAIL;
        break;
        
        case FOTA_FTP_DOWNLOAD_COMPLETE:
            LOG_DBGS(CH_FOTA, "fotaState - FOTA_FTP_DOWNLOAD_COMPLETE");
        break;
        
        case FOTA_FTP_FILE_OPEN_START:
            LOG_DBGS(CH_FOTA, "fotaState - FOTA_FTP_FILE_OPEN_START");
        break;
        
        case FOTA_FTP_FILE_OPEN_COMPLETE:
            LOG_DBGS(CH_FOTA, "fotaState - FOTA_FTP_FILE_OPEN_COMPLETE");
        break;
        
        case FOTA_FTP_FILE_READ_HEADER_START:
            LOG_DBGS(CH_FOTA, "fotaState - FOTA_FTP_FILE_READ_HEADER_START");
            fotaReadBuffIndex = 0;
            fotaReadFileSize = 0;
        break;
        
        case FOTA_FTP_FILE_READ_HEADER_COMPLETE:
            LOG_DBGS(CH_FOTA, "fotaState - FOTA_FTP_FILE_READ_HEADER_COMPLETE");
        break;
        
        case FOTA_FTP_FILE_READ_START:
            LOG_DBGS(CH_FOTA, "fotaState - FOTA_FTP_FILE_READ_START");
            fotaReadBuffIndex = 0;
        break;
        
        case FOTA_FTP_FILE_READ_COMPLETE:
            LOG_DBGS(CH_FOTA, "fotaState - FOTA_FTP_FILE_READ_COMPLETE");
        break;
        
        case FOTA_FTP_FILE_CLOSE_START:
            LOG_DBGS(CH_FOTA, "fotaState - FOTA_FTP_FILE_CLOSE_START");
        break;
        
        case FOTA_FTP_FILE_CLOSE_COMPLETE:
            LOG_DBGS(CH_FOTA, "fotaState - FOTA_FTP_FILE_CLOSE_COMPLETE");
        break;
        
        case FOTA_UPGRADE_VERIFY:
            LOG_DBGS(CH_FOTA, "fotaState - FOTA_UPGRADE_VERIFY");
        break;
        
        case FOTA_SEND_REPLY:
            FotaGetProcessResp(fotaStatus);
            SendSMS(fotaRespPhoneNum,fotaAckBuff,fotaRespLen);
            GsmSetState(GSM_MODEM_IDLE);
            LOG_DBGS(CH_FOTA, "fotaState - FOTA_SEND_REPLY");
        break;
        
       case FOTA_CHECK_FOTA_SUCCESS:
            LOG_DBGS(CH_FOTA,"fotaState - FOTA_CHECK_FOTA_SUCCESS");
         break;
    }
}


fotaState_et FotaGetState(void)
{
  return fotaState;
}


void FotaHandler(void)
{
    static gsmFileOpenState_et openState   = GSM_FILE_OPEN_WAIT_FOR_START;
    static gsmFileReadState_et readState   = GSM_FILE_READ_WAIT_FOR_START;
    static gsmFileCloseState_et closeState = GSM_FILE_CLOSE_WAIT_FOR_START;
    fotaHeaderErr_et headerCheckVal;
    
    switch(fotaState)
    {
        case FOTA_WAIT_FOR_START:
        break;
        
        case FOTA_FTP_DOWNLOAD_START:
            if(FtpStartDownload(fotaUrl,FtpDownloadCompleteCb) == 1)
            {
                FotaSetState(FOTA_FTP_DOWNLOAD_COMPLETE);
            }
            else
            {
                 if(flagFtpDecodeUrl)
                 {
                    flagFtpDecodeUrl = 0;
                    fotaStatus = FOTA_FAIL_DECODE_URL_ERROR;
                    FotaSetState(FOTA_SEND_REPLY);
                 }
                 else
                 {
                   fotaStatus = FOTA_FAIL_GSM_IS_NOT_IDLE;
                   FotaSetState(FOTA_SEND_REPLY);
                 }
            }
        break;
        
        case FOTA_FTP_DOWNLOAD_COMPLETE:
            // wait for the callback to be called
            if(ftpStatus == FTP_STATUS_DOWNLOAD_SUCCESS)
            {
                LOG_DBGS(CH_FOTA,"FTP file download success");
								GsmCloseNtripSocket();
                FotaSetState(FOTA_FTP_FILE_OPEN_START);
            }
            else if(ftpStatus == FTP_STATUS_DOWNLOAD_FAIL)
            {
                LOG_ERR(CH_FOTA,"File downloading error");
                fotaStatus = FOTA_FAIL_FTP_DOWNLOAD;
                FotaSetState(FOTA_SEND_REPLY);
            }
        break;
        
        case FOTA_FTP_FILE_OPEN_START:
            if(GsmFileOpenStart("fotafile.bin",FILE_TYPE_READ_ONLY) == 1)
            {
                FotaSetState(FOTA_FTP_FILE_OPEN_COMPLETE);
            }
        break;
        
        case FOTA_FTP_FILE_OPEN_COMPLETE:
            openState = GsmFileOpenHandler();
            if(openState == GSM_FILE_OPEN_SUCCESS)
            {
                FotaSetState(FOTA_FTP_FILE_READ_HEADER_START);
            }
            else if(openState  == GSM_FILE_OPEN_FAIL)
            {
                LOG_ERR(CH_FOTA,"File opening error");
                fotaStatus = FOTA_FAIL_FILE_OPEN;
                FotaSetState(FOTA_SEND_REPLY);
            }
        break;
        
        case FOTA_FTP_FILE_READ_HEADER_START:
            if(GsmFileReadStart(FtpFileReadByteCb,sizeof(appHeader_st)) == 1)
            {
                FotaSetState(FOTA_FTP_FILE_READ_HEADER_COMPLETE);
            }
        break;
        
        case FOTA_FTP_FILE_READ_HEADER_COMPLETE:
            readState = GsmFileReadHandler();
            if(readState == GSM_FILE_READ_SUCCESS)
            {
                 headerCheckVal  = QbootCheckUpdateRequired(&header);
                //TODO check whether the header is correct and checksum is valid
//                if(QbootCheckUpdateRequired(&header) == HEADER_OK)
                 if(headerCheckVal == HEADER_OK)
                {
                    fotaReadFileSize += fotaReadBuffIndex;
                    fotaReadBuffIndex = 0;
                    FotaSetState(FOTA_FTP_FILE_READ_START);
                }
                else if(headerCheckVal == APP_VER_IS_LOWER)
                {
                  fotaStatus = FOTA_APP_VER_IS_LOWER;
                  FotaSetState(FOTA_FTP_FILE_CLOSE_START);
                }
                else if(headerCheckVal == INVALID_PRODUCT_ID)
                {
                  fotaStatus = FOTA_INVALID_PRODUCT_ID;
                  FotaSetState(FOTA_FTP_FILE_CLOSE_START);
                }
                else if(headerCheckVal == APP_HEADER_CRC_ERROR)
                {
                  fotaStatus = FOTA_APP_HEADER_CRC_ERROR;
                  FotaSetState(FOTA_FTP_FILE_CLOSE_START);
                }
                else
                {
                    fotaStatus = FOTA_FAIL_INVALID_HEADER;
                    FotaSetState(FOTA_FTP_FILE_CLOSE_START);
                }
            }
            else if(readState == GSM_FILE_READ_FAIL)
            {
                LOG_ERR(CH_FOTA,"File read header error");
                fotaStatus = FOTA_FAIL_READ_HEADER;
                FotaSetState(FOTA_FTP_FILE_CLOSE_START);
            }
        break;
        
        case FOTA_FTP_FILE_READ_START:
            if(GsmFileReadStart(FtpFileReadByteCb,FOTA_FILE_READ_SIZE) == 1)
            {
                FotaSetState(FOTA_FTP_FILE_READ_COMPLETE);
            }
        break;
        
        case FOTA_FTP_FILE_READ_COMPLETE:
            readState = GsmFileReadHandler();
            if(readState == GSM_FILE_READ_SUCCESS)
            {
                // check whether the 
                if(fotaUpgradeStart == 1)
                {
                    // write the data into the upgrade memory
                    QbootFlashBackupWrite(fotaReadBuff,fotaReadBuffIndex);
                }
                else
                {
                    fileCalculatedChecksum = Crc16GetFromSeed(fileCalculatedChecksum,fotaReadBuff,fotaReadBuffIndex);
                }
                fotaReadFileSize += fotaReadBuffIndex;
                fotaReadBuffIndex = 0;
                if(fotaReadFileSize >= ftpDownloadFileSize)
                {
                    // file read complete
                    LOG_DBGS(CH_FOTA, "file read complete");
                    if(fotaUpgradeStart == 0)
                    {
                        if(fileCalculatedChecksum == header.appCrc)
                        {
                            LOG_DBGS(CH_FOTA, "file check sum verified success");
                            fotaUpgradeStart = 1;
                            QbootFlashBackupOpen();
                        }
                        else
                        {
                            fotaStatus = FOTA_FAIL_FILE_CHECKSUM;
                            LOG_ERR(CH_FOTA, "file check sum verification fail FILE - %d CALC - %d",header.appCrc, fileCalculatedChecksum);
                        }
                    }
                    else
                    {
                        // now clear the fotaUpgradeStart so that it will not start again
                        fotaUpgradeStart = 0;
                        fotaFlashVerify = 1;
                    }
                    FotaSetState(FOTA_FTP_FILE_CLOSE_START);
                }
                else
                {
                    FotaSetState(FOTA_FTP_FILE_READ_START);
                }
            }
            else if(readState == GSM_FILE_READ_FAIL)
            {
                LOG_ERRS(CH_FOTA,"read file error");
                fotaStatus = FOTA_FAIL_READ_FILE;
                FotaSetState(FOTA_FTP_FILE_CLOSE_START);
            }
        break;
        
        case FOTA_FTP_FILE_CLOSE_START:
            if(GsmFileCloseStart())
            {
                FotaSetState(FOTA_FTP_FILE_CLOSE_COMPLETE);
            }
        break;
        
        case FOTA_FTP_FILE_CLOSE_COMPLETE:
            closeState = GsmFileCloseHandler();
            if(closeState == GSM_FILE_CLOSE_SUCCESS || closeState == GSM_FILE_CLOSE_FAIL)
            {
                 LOG_DBG(CH_FOTA,"FOTA readState :  %d" ,readState);
                if((fotaUpgradeStart == 1) && (readState == GSM_FILE_READ_SUCCESS))
                {
                    //AGAIN download the whole file and now load it in flash memory
                    FotaSetState(FOTA_FTP_FILE_OPEN_START);
                }
                else
                {
                    if(fotaFlashVerify == 1)
                    {
                        FotaSetState(FOTA_UPGRADE_VERIFY);
                    }
                    else
                    {
                        FotaSetState(FOTA_SEND_REPLY);
                    }
                }
            }
        break;
        
        case FOTA_UPGRADE_VERIFY:
					   crcsize = fotaReadFileSize - sizeof(appHeader_st);
					   backCrc = QbootBackupGetCrc(crcsize);
				     
				    LOG_DBG(CH_GSM, "header.appCrc = %x ,backCrc = %x",header.appCrc,backCrc);
				
            if(QbootBackupGetCrc(crcsize) == header.appCrc)
            {
                // upgrade verification successful
                // write the header now
                QbootWriteBackupHeader(&header);
                LOG_DBGS(CH_FOTA,"flash verification success");
                fotaStatus = FOTA_SUCCESS;
            }
            else
            {
                LOG_ERRS(CH_FOTA,"flash verification failure");
            }
            FotaSetState(FOTA_SEND_REPLY);
            
        break;
        
        case FOTA_SEND_REPLY:
              if(GsmGetSendState() == 1)
              {
                    FotaSetState(FOTA_CHECK_FOTA_SUCCESS);
              }
        break;
        
       case FOTA_CHECK_FOTA_SUCCESS:
            FotaSetState(FOTA_WAIT_FOR_START);
            if(fotaStatus == FOTA_SUCCESS)
            {
                HAL_NVIC_SystemReset();
            }
            else
            {
                GsmSetState(GSM_MODEM_IDLE);           //set gsm modem in the idle state if fota fail
            }
         break;
    }
}