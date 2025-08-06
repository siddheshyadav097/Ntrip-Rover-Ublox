#ifndef __DEBUG_LOG_PORT_H
#define __DEBUG_LOG_PORT_H
#include "stm32g0xx_hal.h"

#define DEBUG_UART        USART2
extern UART_HandleTypeDef huartDebug;

//void DebugLogInitPort(void);
//void DebugLogInitPort(void);
typedef void (*GetSerialFnPtr)(uint8_t receiveGpsData);  
void DebugLogInitPort(GetSerialFnPtr serialDataCallBack);
//void DebugLogMspInit(void);
void DebugLogSend(uint8_t *sendData,uint16_t sendLen);

 /*----public function declarations-----*/


void serialDataRcvCb(uint8_t receiveData);
void DebugIRQHandler(void) ; 

#endif