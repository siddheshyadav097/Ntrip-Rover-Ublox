/**
 *  \file gps_port.h
 *  \brief GPS port initialization and serial data read routine. 
 */

#ifndef __GPS_PORT_H__
#define __GPS_PORT_H__


 /*----includes-----*/
#include "lib_port.h"

#define GPS_UART             USART2
#define GPS_UART_IRQ         USART2_LPUART2_IRQn   
   
#define GPS_ENABLE_Pin       GPIO_PIN_9
#define GPS_ENABLE_GPIO_Port GPIOB


#define GPS_RESET_CTRL_Pin       GPIO_PIN_4
#define GPS_RESET_CTRL_Port      GPIOD

 /*----public function declarations-----*/
typedef void (*GpsReceiveDataCbFnPtr_t)(uint8_t receiveGpsData);  


void InitGPS(void);
//void CallBack_ReadGps_Data(UART_HandleTypeDef *huart);
BOOL Verify_Checksum(uint8_t *strBuf, uint16_t str_len, uint8_t chksum);
void PutOffGpsSupply(void);
void SendRtcm(uint8_t *sendData,uint16_t sendLen);
#endif