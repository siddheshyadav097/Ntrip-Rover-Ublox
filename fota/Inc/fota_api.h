#ifndef __FOTA_API_H
#define __FOTA_API_H

#include "gsm_api.h"
#include "gsm_ufs.h"
#include "gsm_ftp_api.h"
#include "qboot.h"
#include "crc16.h"
#include "debug_log.h"
	
#include <stdio.h>

#define FOTA_URL_SIZE           256
#define FOTA_FILE_READ_SIZE     2000
#define FOTA_MAX_TIMEOUT        (10UL*60UL*1000UL)  // 10 minutes

typedef enum
{
    FOTA_WAIT_FOR_START,    // waiting for SMS or command from server with details of the ftp link to download
    FOTA_FTP_DOWNLOAD_START,
    FOTA_FTP_DOWNLOAD_COMPLETE,
    FOTA_FTP_FILE_OPEN_START,
    FOTA_FTP_FILE_OPEN_COMPLETE,
    FOTA_FTP_FILE_READ_HEADER_START,
    FOTA_FTP_FILE_READ_HEADER_COMPLETE,
    FOTA_FTP_FILE_READ_START,
    FOTA_FTP_FILE_READ_COMPLETE,
    FOTA_FTP_FILE_CLOSE_START,
    FOTA_FTP_FILE_CLOSE_COMPLETE,
    FOTA_UPGRADE_VERIFY,
    FOTA_SEND_REPLY,
    FOTA_CHECK_FOTA_SUCCESS
}fotaState_et;

typedef enum
{
    FOTA_FAIL,
    FOTA_FAIL_TIMEOUT,
    FOTA_FAIL_FTP_DOWNLOAD,
    FOTA_FAIL_FILE_OPEN,
    FOTA_FAIL_READ_HEADER,
    FOTA_FAIL_INVALID_HEADER,
    FOTA_FAIL_READ_FILE,
    FOTA_FAIL_FILE_CHECKSUM,
    FOTA_FAIL_FLASH_VERIFY,
    FOTA_SUCCESS,
    FOTA_APP_VER_IS_LOWER,
//    HEADER_OK,   //1
    FOTA_INVALID_PRODUCT_ID,
    FOTA_APP_HEADER_CRC_ERROR,
    FOTA_VER_NO_CRC_ERROR,
    FOTA_FAIL_GSM_IS_NOT_IDLE,
    FOTA_FAIL_DECODE_URL_ERROR
    
}fotaStatus_et;

void FotaStartFromFtp(uint8_t *urlString, uint8_t *phoneNum);
void FotaSetState(fotaState_et state);
void FotaHandler(void);
fotaState_et FotaGetState(void);

//typedef uint8_t (*GsmSendSmsCallbackFnPtr_t)(uint8_t* smsData, uint16_t smsLen,fotaStatus_et fotaRespEnum);
//void GsmFotaResponseInit(GsmSendSmsCallbackFnPtr_t fnPtr);
#endif