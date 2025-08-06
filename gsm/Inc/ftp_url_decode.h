#ifndef __FTP_URL_DECODE_H
#define __FTP_URL_DECODE_H

#include "gsm_port.h"

#define FTP_SERVERADD_LEN       40
#define FTP_FILEPATH_LEN        60
#define FTP_USERNAME_LEN        50
#define FTP_PASSWORD_LEN        50
#define FTP_FILENAME_LEN        50
#define FTP_SERVICE_PORT        21

typedef struct
{
    uint8_t host[FTP_SERVERADD_LEN];
    uint16_t portNum;
    uint8_t filePath[FTP_FILEPATH_LEN];
    uint8_t userName[FTP_USERNAME_LEN];
    uint8_t password[FTP_PASSWORD_LEN];
    uint8_t fileName[FTP_FILENAME_LEN];
    uint32_t fileSize;
}ftpFile_st;

typedef enum
{
    FTP_URL_DECODE_FAIL,
    FTP_URL_DECODE_SUCCESS,
}ftpUrlDecodeRet_t;

// this function will return whether the decode is successful or failure
// on success it will copy all the details in the ftpFile structure
// url should be a string terminated with NULL character
ftpUrlDecodeRet_t FtpUrlDecode(uint8_t *urlString, ftpFile_st *ftpResult);

#endif
