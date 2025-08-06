/**
 *  \file debug_log.h
 *  \brief Includes macros for printing various log messages along with typedefs for log levels and channels.
 *  		Also public function declarations for initializing and printing logs are included. 
 */

#ifndef __APP_DEBUG_H__
#define __APP_DEBUG_H__


 /*----includes-----*/
#include "debug_log_port.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
/******************************************************************************/
/* Configuration to enable log messages */
/* set the values to 1 if it is to be enbaled */
#define LOG_LEVEL_CRIT_ENABLE               1
#define LOG_LEVEL_ERR_ENABLE                1
#define LOG_LEVEL_WARN_ENABLE               1
#define LOG_LEVEL_INFO_ENABLE               1
#define LOG_LEVEL_DBG_ENABLE                1
/******************************************************************************/


/* set the values to 1 if it is to be enbaled */

/*----typedefs-----*/
typedef enum
{
	LOG_LEVEL_CRIT,
	LOG_LEVEL_ERR,
	LOG_LEVEL_WARN,
	LOG_LEVEL_INFO,
	LOG_LEVEL_DBG,
    LOG_LEVEL_RFID,
	LOG_NUM_LEVELS
}loglevel_t;

typedef enum
{
    CH_GSM,
    CH_GPRS,
    CH_SMS,
    CH_SOCK,
    CH_POLL,
    CH_GPS,
    CH_PACKET,
    CH_MEMORY,
    CH_FTP,
    CH_FOTA,
    CH_UFS,
    CH_BOOT,
    CH_SERIAL,
    CH_RFID,
		CH_RTCM,
	LOG_NUM_CHANNELS
}logchannel_t;


 /*----constants-----*/
#define DBG_BUF_LEN   		1024
#define LOG_LEVEL_NUM		LOG_NUM_LEVELS
#define LOG_CHANNEL_NUM 	LOG_NUM_CHANNELS

 /*----variables-----*/
extern char debugBuffer[DBG_BUF_LEN];
extern const char *const level2str[LOG_LEVEL_NUM];
extern const char *const channel2str[LOG_CHANNEL_NUM];
extern uint16_t msgLen;

void DebugLogPrintString(loglevel_t logLevel,logchannel_t chVal, char* msgPtr);
void DebugLogPrintStringN(loglevel_t logLevel,logchannel_t chVal, char* msgPtr,uint16_t msgLength);

void LogCritS(logchannel_t chVal, char* msgPtr);
void LogCritN(logchannel_t chVal, char* msgPtr, uint16_t msgLength);
void LogErrS(logchannel_t chVal, char* msgPtr);
void LogErrN(logchannel_t chVal, char* msgPtr, uint16_t msgLength);
void LogWarnS(logchannel_t chVal, char* msgPtr);
void LogWarnN(logchannel_t chVal, char* msgPtr, uint16_t msgLength);
void LogInfoS(logchannel_t chVal, char* msgPtr);
void LogInfoS(logchannel_t chVal, char* msgPtr);
void LogInfoN(logchannel_t chVal, char* msgPtr, uint16_t msgLength);
void LogDebugN(logchannel_t chVal, char* msgPtr, uint16_t msgLength);
void LogDebugS(logchannel_t chVal, char* msgPtr);

#if LOG_LEVEL_CRIT_ENABLE
#define LOG_CRITS(_ch, _msg)            DebugLogPrintString(LOG_LEVEL_CRIT,_ch, _msg)
#define LOG_CRITSN(_ch, _msg, _len)     DebugLogPrintStringN(LOG_LEVEL_CRIT,_ch, _msg, _len)
#define LOG_CRIT(_ch, _msg, ...){\
    msgLen = snprintf((char *)debugBuffer, sizeof(debugBuffer), "%s,%s,", (const char *)level2str[LOG_LEVEL_CRIT], (const char *)channel2str[_ch]); \
	DebugLogSend((uint8_t*)debugBuffer,msgLen);\
    msgLen = snprintf((char *)debugBuffer, sizeof(debugBuffer),_msg,##__VA_ARGS__);\
    DebugLogSend((uint8_t*)debugBuffer,msgLen);\
    DebugLogSend((uint8_t*)"\r\n",2);\
}

#else
#define LOG_CRITS(_ch, _msg)
#define LOG_CRITSN(_ch, _msg, _len)
#define LOG_CRIT(_ch, _msg, ...)
#endif

#if LOG_LEVEL_ERR_ENABLE

#define LOG_ERRS(_ch, _msg)             DebugLogPrintString(LOG_LEVEL_ERR,_ch, _msg)
#define LOG_ERRSN(_ch, _msg, _len)      DebugLogPrintStringN(LOG_LEVEL_ERR,_ch, _msg, _len)
#define LOG_ERR(_ch, _msg, ...){\
    msgLen = snprintf((char *)debugBuffer, sizeof(debugBuffer), "%s,%s,", (const char *)level2str[LOG_LEVEL_ERR], (const char *)channel2str[_ch]); \
	DebugLogSend((uint8_t*)debugBuffer,msgLen);\
    msgLen = snprintf((char *)debugBuffer, sizeof(debugBuffer),_msg,##__VA_ARGS__);\
    DebugLogSend((uint8_t*)debugBuffer,msgLen);\
    DebugLogSend((uint8_t*)"\r\n",2);\
}
#else
#define LOG_ERRS(_ch, _msg)
#define LOG_ERRSN(_ch, _msg, _len)
#define LOG_ERR(_ch, _msg, ...)
#endif

#if LOG_LEVEL_WARN_ENABLE
#define LOG_WARNS(_ch, _msg)            DebugLogPrintString(LOG_LEVEL_WARN,_ch, _msg)
#define LOG_WARNSN(_ch, _msg, _len)     DebugLogPrintStringN(LOG_LEVEL_WARN,_ch, _msg, _len)
#define LOG_WARN(_ch, _msg, ...){\
    msgLen = snprintf((char *)debugBuffer, sizeof(debugBuffer), "%s,%s,", (const char *)level2str[LOG_LEVEL_WARN], (const char *)channel2str[_ch]); \
	DebugLogSend((uint8_t*)debugBuffer,msgLen);\
    msgLen = snprintf((char *)debugBuffer, sizeof(debugBuffer),_msg,##__VA_ARGS__);\
    DebugLogSend((uint8_t*)debugBuffer,msgLen);\
    DebugLogSend((uint8_t*)"\r\n",2);\
}
#else
#define LOG_WARNS(_ch, _msg)
#define LOG_WARNSN(_ch, _msg, _len)
#define LOG_WARN(_ch, _msg, ...)
#endif

#if LOG_LEVEL_INFO_ENABLE
#define LOG_INFOS(_ch, _msg)            DebugLogPrintString(LOG_LEVEL_INFO,_ch, _msg)
#define LOG_INFOSN(_ch, _msg, _len)     DebugLogPrintStringN(LOG_LEVEL_INFO,_ch, _msg, _len)
#define LOG_INFO(_ch, _msg, ...){\
    msgLen = snprintf((char *)debugBuffer, sizeof(debugBuffer), "%s,%s,", (const char *)level2str[LOG_LEVEL_INFO], (const char *)channel2str[_ch]); \
	DebugLogSend((uint8_t*)debugBuffer,msgLen);\
    msgLen = snprintf((char *)debugBuffer, sizeof(debugBuffer),_msg,##__VA_ARGS__);\
    DebugLogSend((uint8_t*)debugBuffer,msgLen);\
    DebugLogSend((uint8_t*)"\r\n",2);\
}
#else
#define LOG_INFOS(_ch, _msg)
#define LOG_INFOSN(_ch, _msg, _len)
#define LOG_INFO(_ch, _msg, ...)
#endif 

#if LOG_LEVEL_DBG_ENABLE
#define LOG_DBGS(_ch, _msg)             DebugLogPrintString(LOG_LEVEL_DBG,_ch, _msg)
#define LOG_DBGSN(_ch, _msg, _len)      DebugLogPrintStringN(LOG_LEVEL_DBG,_ch, _msg, _len)
#define LOG_DBG(_ch, _msg, ...){\
    msgLen = snprintf((char *)debugBuffer, sizeof(debugBuffer), "%s,%s,", (const char *)level2str[LOG_LEVEL_DBG], (const char *)channel2str[_ch]); \
	DebugLogSend((uint8_t*)debugBuffer,msgLen);\
    msgLen = snprintf((char *)debugBuffer, sizeof(debugBuffer),_msg,##__VA_ARGS__);\
    DebugLogSend((uint8_t*)debugBuffer,msgLen);\
    DebugLogSend((uint8_t*)"\r\n",2);\
}
#else
#define LOG_DBGS(_ch, _msg)
#define LOG_DBGSN(_ch, _msg, _len)
#define LOG_DBG(_ch, _msg, ...)

#endif

 /*----public function declarations-----*/
void DebugLogInit();

#endif