/**
 *  @file          :  led_port.c
 *  @author        :  Aakash/Ratna
 *  @date          :  18/4/2017
 *  @brief         :  Port initialization and hardware control of LED's
 *  				  Led_State_ON & Led_State_OFF are common control handles for all LED's,
 *  				  where the particular LED can be specified to be turned ON or OFF. 
 *  @filerevision  :  1.0
 *  
 */
 
 /*----includes-----*/
#include "led_port.h"

/*----public functions-----*/

// Initialize the GPIO pin (output high level, pull up)
/**
 *  @brief Initializes the gpio for 
 *  
 *  @return void
 */
void Init_LED_port(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
  
    GPIO_InitStruct.Pin = LED_GSM_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_GSM_GPIO_Port, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = LED_GPS_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_GPS_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LED_PWRSTATUS_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_PWRSTATUS_GPIO_Port, &GPIO_InitStruct);
	
//	  GPIO_InitStruct.Pin = TEST_TAG_DETECT_PIN1;
//    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//	  GPIO_InitStruct.Pull = GPIO_NOPULL;
//    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//    HAL_GPIO_Init(TEST_TAG_DETECT_PORT, &GPIO_InitStruct);
		
    
    HAL_GPIO_WritePin(LED_GSM_GPIO_Port,LED_GSM_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_GPS_GPIO_Port, LED_GPS_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_PWRSTATUS_GPIO_Port, LED_PWRSTATUS_Pin, GPIO_PIN_SET);
		//HAL_GPIO_WritePin(TEST_TAG_DETECT_PORT, TEST_TAG_DETECT_PIN1, GPIO_PIN_SET);
}

void Led_State_ON(Enum_PinName pinname)
{
	if(pinname == GPRS_LED)
	{
		HAL_GPIO_WritePin(LED_GSM_GPIO_Port,LED_GSM_Pin,GPIO_PIN_RESET);
	}
	else if(pinname == GPS_LED)
	{
		HAL_GPIO_WritePin(LED_GPS_GPIO_Port,LED_GPS_Pin,GPIO_PIN_RESET);
	}
	else if(pinname == PWR_LED)
	{
		HAL_GPIO_WritePin(LED_PWRSTATUS_GPIO_Port,LED_PWRSTATUS_Pin,GPIO_PIN_RESET);
	}
//    else if(pinname == RFID_LED)
//    {
//      HAL_GPIO_WritePin(TEST_TAG_DETECT_PORT,TEST_TAG_DETECT_PIN1,GPIO_PIN_RESET);
//    }
}

void Led_State_OFF(Enum_PinName pinname)
{
	if(pinname == GPRS_LED)
	{
		HAL_GPIO_WritePin(LED_GSM_GPIO_Port,LED_GSM_Pin,GPIO_PIN_SET);
	}
	else if(pinname == GPS_LED)
	{
		HAL_GPIO_WritePin(LED_GPS_GPIO_Port,LED_GPS_Pin,GPIO_PIN_SET);
	}
	else if(pinname == PWR_LED)
	{
		HAL_GPIO_WritePin(LED_PWRSTATUS_GPIO_Port,LED_PWRSTATUS_Pin,GPIO_PIN_SET);
	}
//    else if(pinname == RFID_LED)
//    {
//      HAL_GPIO_WritePin(TEST_TAG_DETECT_PORT,TEST_TAG_DETECT_PIN1,GPIO_PIN_SET);
//    }
}