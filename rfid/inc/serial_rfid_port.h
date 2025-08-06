#ifndef __SERIAL_RFID_PORT_H
#define __SERIAL_RFID_PORT_H

#include "gsm_utility.h"
#include "rfid_data_handler.h"
#include "digio_port.h"
#include "ring_buffer.h" 
#include "qtimespent.h"

#define RFID_DATA_UART        USART3
//#define RFID_DATA_UART_IRQ    UART5_IRQn


#define SERIAL_DISABLE_Pin       GPIO_PIN_14
#define SERIAL_DISABLE_GPIO_Port GPIOB  


//#define BUZZER_ENABLE_Pin        GPIO_PIN_6
//#define BUZZER_ENABLE_GPIO_Port  GPIOC

typedef void (*RfidReceiveDataCbFnPtr_t)(uint8_t rfReceiveDat); 
void InitRfidSerialPort(RfidReceiveDataCbFnPtr_t rfFnPtr);
void RfidUartSendData(uint8_t *sendData, uint16_t sendLen);
void SerialControlConfig(uint8_t disableFlag);    //inverted logic
void RfidUartMspPortInit(void);
void RfidMspUartPortDeInit(void);
uint8_t GetSerialPwrKeyState(void);  //if pin level is 0 output vtg is generated



























#endif