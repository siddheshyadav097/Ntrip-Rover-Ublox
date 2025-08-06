/**
 *  \file led_port.h
 *  \brief Includes for platform specific dependencies and public function declarations 
 *			for LED port initialization and control are provided.
 */

#ifndef __LED_PORT_H
#define __LED_PORT_H


 /*----includes-----*/
#include "lib_port.h"
//#include "mcu_port.h"

#define  LED_GSM_Pin              GPIO_PIN_0   //gsm PD0
#define  LED_GSM_GPIO_Port        GPIOD

#define LED_GPS_Pin               GPIO_PIN_1    //gps  PD1
#define LED_GPS_GPIO_Port         GPIOD


#define LED_PWRSTATUS_Pin         GPIO_PIN_9  //pwr status PC9
#define LED_PWRSTATUS_GPIO_Port   GPIOC


//#define TEST_TAG_DETECT_PIN1       GPIO_PIN_10     //PA10 LED_RFID
//#define TEST_TAG_DETECT_PORT       GPIOA


//#define LED_PWRSTATUS_Pin GPIO_PIN_9   //pwr
//#define LED_PWRSTATUS_GPIO_Port GPIOB
//
//#define LED_GPS_Pin GPIO_PIN_8  //gps
//#define LED_GPS_GPIO_Port GPIOB
//
//
//#define  GPIO_PIN_13  //gsm
//#define  GPIOC

 /*----typedefs-----*/
typedef enum
{
  PWR_LED=0,    //GPRS n/w registration
  GPS_LED,      //GPS data Valid/Invalid
  GPRS_LED      //Power/Packet transfer
  //RFID_LED
}Enum_PinName;

 /*----public function declarations-----*/
void Init_LED_port(void);
void Led_State_ON(Enum_PinName pinname);
void Led_State_OFF(Enum_PinName pinname);

#endif