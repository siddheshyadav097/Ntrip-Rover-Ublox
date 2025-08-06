#ifndef __GSM_UFS_H
#define __GSM_UFS_H

#include "gsm_port.h"
#include "gsm_statemachine.h"
#include "gsm_at_handler.h"

#define GSM_FILE_CMD_BUFF_SIZE      256
#define FOTA_FILE_READ_SIZE         2000

#define GSM_FILE_OPEN_CMD_TIMEOUT       1000
#define GSM_FILE_READ_CMD_TIMEOUT       1000
#define GSM_FILE_CLOSE_CMD_TIMEOUT      2000

typedef enum
{
    FILE_MODE_READ_WRITE = 0,      // if file exists 
    FILE_MODE_WRITE      = 1,      
    FILE_TYPE_READ_ONLY  = 2
}fileMode_et;

typedef enum
{
    GSM_FILE_OPEN_WAIT_FOR_START,
    GSM_FILE_OPEN_IN_PROGRESS,
    GSM_FILE_OPEN_SUCCESS,
    GSM_FILE_OPEN_FAIL
}gsmFileOpenState_et;

// will return 1 when the file open command is sent successfully
uint8_t GsmFileOpenStart(char *fileName, fileMode_et mode);
gsmFileOpenState_et GsmFileOpenHandler(void);
int GsmGetFileHandle(void);


typedef void (*GsmFileReadByteCbFnPtr)(uint8_t byte);

typedef enum
{
    GSM_FILE_READ_WAIT_FOR_START,
    GSM_FILE_READ_IN_PROGRESS,
    GSM_FILE_READ_SUCCESS,
    GSM_FILE_READ_FAIL
}gsmFileReadState_et;
uint8_t GsmFileReadStart(GsmFileReadByteCbFnPtr fileReadByteCb,uint16_t readLen);
gsmFileReadState_et GsmFileReadHandler(void);


typedef enum
{
    GSM_FILE_CLOSE_WAIT_FOR_START,
    GSM_FILE_CLOSE_IN_PROGRESS,
    GSM_FILE_CLOSE_SUCCESS,
    GSM_FILE_CLOSE_FAIL
}gsmFileCloseState_et;
uint8_t GsmFileCloseStart(void);
gsmFileCloseState_et GsmFileCloseHandler(void);

#endif