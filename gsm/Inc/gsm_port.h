
/**
 *  @file          :gsm_port.h
 *  @author        :
 *  @date          :
 *  @brief         :
 *  @filerevision  :
 *  
 */
#ifndef __GSM_PORT_H
#define __GSM_PORT_H

#include "stm32g0xx_hal.h"
#include "qtimespent.h"
#include "debug_log.h"

//TPS ENABLE PIN
#define GSM_PWR_EN_GPIO_PIN         GPIO_PIN_2  //PD2
#define GSM_PWR_EN_GPIO_PORT        GPIOD

//EC25 PWR KEY
#define GSM_PWR_KEY_GPIO_PIN         GPIO_PIN_8 //PC3
#define GSM_PWR_KEY_GPIO_PORT        GPIOD

//EC25 RESET KEY
#define GSM_RESET_KEY_GPIO_PIN        GPIO_PIN_2//PC2
#define GSM_RESET_KEY_GPIO_PORT       GPIOC

#define GSM_UART_SEND_TIMEOUT       100

#define CURRENT_DEFAULT_YEAR        23

typedef enum
{
    GSM_SEND_FAILURE,
    GSM_SEND_SUCCESS
}gsmUartSendRet_et;

typedef enum
{
  TURN_POWER_OFF=0,
  TURN_POWER_ON,
  WAIT
}Power_States;


typedef struct 
{
    int32_t year;    
    int32_t month;
    int32_t day;
    int32_t hour;
    int32_t minute;
    int32_t second;
    int32_t timezone;  
}gsmTimePara_t;

typedef void (*GsmReceiveDataCbFnPtr_t)(uint8_t receiveDat);  

/**
 *  @brief  : Initialise the uart port
 *  @param  :[in] fnPtr Pointer of call back function
 *              This function is called when a byte is received and the received byte is passed to this function
 *  @return : none
 */
void GsmUartPortInit(GsmReceiveDataCbFnPtr_t fnPtr);

/**
 *  @brief  : The port pins and clock of UART is initialised here
 *  @return : none
 */
void GsmUartMspPortInit(void);

/**
 *  @brief  : Deinit the UART port
 *  @return :Return_Description
 */
void GsmMspUartPortDeInit(void);
/**
 *  @brief  : Initialise uart power enable port pin
 *  @return : none
 */
void GsmPwrEnableInit(void);
void GsmPwrKeyInit(void);

/**
 *  @brief  :Switch on the gsm power
 *  @return :none
 */
void TurnOnGsmPowerRegulatorSwitch(void);
void GsmPowerkeyPinOn(void);

/**
 *  @brief  :Switch OFF gsm power
 *  @return :none
 */
void TurnOffGsmPowerRegulatorSwitch(void);
void GsmPowerkeyPinOff(void);


//void Toggle_GPRS_LED(void);
//void GPRS_LED_KEEP_ON(void);
//void PACKET_UPLOAD_LED_KEEP_ON(void);
//void Toggle_PACKET_UPLOAD_LED(void);
/**
 *  @brief  : This function sends the data to UART
 *  @param  :[in] sendData - array byte
 *  @param  :[in] sendLen  number of bytes to be sent
 *  @return :Return_Description
 */
gsmUartSendRet_et GsmUartSendData(char *sendData, uint16_t sendLen);

gsmUartSendRet_et GsmUartSendByte(uint8_t sendData);

#endif