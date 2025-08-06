#ifndef __GSM_FTP_API_H
#define __GSM_FTP_API_H

#include "gsm_port.h"
#include "gsm_statemachine.h"
#include "gsm_at_handler.h"
#include "gsm_gprs_api.h"
#include "ftp_url_decode.h"

// FTP command timeout
#define GSM_FTP_DEFAULT_CMD_TIMEOUT     300
#define GSM_FTP_CMD_TIMEOUT             1000
#define GSM_FTP_CLOSE_CMD_TIMEOUT       5000
#define GSM_FTP_OPEN_CMD_TIMEOUT        120000
#define GSM_FTP_FILE_DW_TIMEOUT         300000

// ftp command buffer size
#define FTP_CMD_BUFF_SIZE               512

typedef enum
{
    FTP_STATUS_DOWNLOAD_IN_PROG,
    FTP_STATUS_DOWNLOAD_SUCCESS,
    FTP_STATUS_DOWNLOAD_FAIL
}ftpDownloadStatus_et;

typedef enum
{
    DELETE_UFS_FILES = 0,
    GET_UFS_FREE_SPACE,
    SET_FTP_CONTEXTID,
    SET_FTP_ACCOUNT,
    SET_FTP_FILETYPE,
    SET_FTP_TRANSMODE,
    SET_FTP_RESPTIMOUT,
    //SET_FTP_USERNAME,
    //SET_FTP_PASSWORD,
    //SET_FTP_CONFIG,
    OPEN_FTP_PORT,
    SET_FILE_PATH,
    GET_FILE_SIZE,
    GET_FTP_FILE,
    CLOSE_FTP_PORT,
    NUM_FTP_CMDS
}ftpCmd_et; 

typedef enum
{
    FTP_SUCCESS = 0,
    FTP_UNKNOWN_ERROR = -1,
    FTP_SERVICE_BUSY = -3,
    FAIL_TO_GET_IP = -4,
    FAIL_TO_SEND_CMD = -5,
    SESSION_CLOSED_BY_SERVER = -6,
    DATA_CONN_CLOSED_BY_SERVER = -7,
    GPRS_CONTEXT_DEACTIVATED = -8,
    TIMEOUT = -9,
    INPUT_PARA_ILLEGAL = -10,
    FILE_NOT_FOUND = -11,
    FAIL_TO_GET_FILE = -12,
    NO_ENOUGH_MEMORY_ON_GSM = -13,
    FTP_SERVICE_NOT_SUPPORTED = -421,
    FAIL_TO_OPEN_DATA_CONN = -425,
    CONN_CLOSED_STOPPED_TRANSFER = -426,
    FILE_REQUEST_NOT_OPERATED = -450,
    NO_ENOUGH_MEMORY_ON_SERVER = -452,
    WRONG_FTP_CMD_FORMAT = -500,
    WRONG_FTP_CMD_PARAMETER = -501,
    FTP_CMD_NOT_OPERATED_BY_SERVER = -502,
    LOGIN_ERROR_ON_SERVER = -530,
    NEED_ACCOUNT_INFO = -532,
    REQUEST_NOT_OPERATED = -550,
    REQUEST_STOPPED = -551,
    FILE_REQUEST_STOPPED = -552,
    ILLEGAL_FILENAME = -553
}FTPCmdReply_et;

typedef void (*FtpDownloadStatusCbFnPtr)(ftpDownloadStatus_et status, uint32_t downloadSize);
typedef struct
{
	char *cmd;
	uint32_t cmdTimeoutInMs;
	GsmCmdResponseCbFnPtr_t respCb;
}gsmFtpCmd_st;

typedef enum
{
    FTP_WAIT_FOR_START,
    FTP_DOWNLOAD_IN_PROGRESS,
    FTP_DOWNLOAD_SUCCESS,
    FTP_DOWNLOAD_FAIL
}gsmFtpState_et;

void GsmFtpHandler(void);
uint8_t FtpStartDownload(uint8_t *urlString, FtpDownloadStatusCbFnPtr statusCb);
FTPCmdReply_et ftpGetFileDwnldErrorCode(void);
#endif