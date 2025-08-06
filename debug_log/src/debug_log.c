/**
 *  @file          :  debug_log.c
 *  @author        :  Aakash/Ratna
 *  @date          :  06/05/2017
 *  @brief         :  Log message classification and printing on the UART.
 *  				  Includes handles seperately for different log levels & port initialization.
 *  @filerevision  :  1.0
 */

 /*----includes-----*/
#include "debug_log.h"
#include "ais_app.h"

#define DEBUG_UNIT_TEST     0

 /*----variables-----*/
char debugBuffer[DBG_BUF_LEN];
uint16_t msgLen;


// variable below is the type of 
const char *const level2str[LOG_LEVEL_NUM] = {
    "CRT", "ERR", "WAR", "INF", "DBG"
};
const char *const channel2str[LOG_CHANNEL_NUM] = {
    "GSM","GPR","SMS","SOC","POL","GPS","PAC","MEM","FTP","FOT","UFS","BOOT","FUEL","RDR","RTCM"
};
 /*----public functions-----*/
void DebugLogPrintString(loglevel_t logLevel,logchannel_t chVal, char* msgPtr);
void DebugLogPrintStringN(loglevel_t logLevel,logchannel_t chVal, char* msgPtr,uint16_t msgLength);
// this function is called by DEBUG UART reception in ISR
/**
 *  @brief This function initializes both the serial port by registering the callback function for the 
 *  specified serial port & It opens a specified UART port with the specified flow control mode. 
 *  Which task call this function, which task will own the specified UART port.
 *  @return void
 */
 
void DebugLogInit(void)
{
	DebugLogInitPort(serialDataRcvCb);
    
#if DEBUG_UNIT_TEST
//    LOG_CRITS(CH_GSM,"Hello world");
//    LOG_ERRS(CH_GSM,"Hello world");
//    LOG_WARNS(CH_GSM,"Hello world");
//    LOG_INFOS(CH_GSM,"Hello world");
//    LOG_DBGS(CH_GSM,"Hello world");
//    
//    LOG_CRITSN(CH_GSM,"Hello world",7);
//    LOG_ERRSN(CH_GSM,"Hello world",7);
//    LOG_WARNSN(CH_GSM,"Hello world",7);
//    LOG_INFOSN(CH_GSM,"Hello world",7);
//    LOG_DBGSN(CH_GSM,"Hello world",7);
//    
//    LOG_CRIT(CH_GSM,"Temp : %0.1f setpoint : %d",1.52,64);
//    LOG_ERR(CH_GSM,"Temp : %0.1f setpoint : %d",1.52,64);
//    LOG_WARN(CH_GSM,"Temp : %0.1f setpoint : %d",1.52,64);
//    LOG_INFO(CH_GSM,"Temp : %0.1f setpoint : %d",1.52,64);
//    LOG_DBG(CH_GSM,"Temp : %0.1f setpoint : %d",1.52,64);
    
#endif
}

void DebugLogPrintString(loglevel_t logLevel,logchannel_t chVal, char* msgPtr)
{
    msgLen = snprintf((char *)debugBuffer, sizeof(debugBuffer), "%s,%s,%s\r\n",(const char *)level2str[logLevel], (const char *)channel2str[chVal],msgPtr);
	DebugLogSend((uint8_t*)debugBuffer,msgLen);
}

void DebugLogPrintStringN(loglevel_t logLevel,logchannel_t chVal, char* msgPtr,uint16_t msgLength)
{
    msgLen = snprintf((char *)debugBuffer, sizeof(debugBuffer), "%s,%s,", (const char *)level2str[logLevel], (const char *)channel2str[chVal]);
	DebugLogSend((uint8_t*)debugBuffer,msgLen);
    DebugLogSend((uint8_t*)msgPtr,msgLength);
    DebugLogSend((uint8_t*)"\r\n",2);
}