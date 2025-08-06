/**
 *  @file          :  digio_port.c
 *  @author        :  Aakash/Ratna
 *  @date          :  18/4/2017
 *  @brief         :  Includes digital i/o port initialization and interface for reading digital input pins 
 *  				  and controlling digital output pins
 *  @filerevision  :  1.0
 *  
 */

 
 /*----includes-----*/
#include "digio_port.h"
   
 /*----variables-----*/
//uint8_t InputPinState = 0;

 /*----public functions-----*/
 
void InitializeDigitalIOPins(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;
 
  /*Configure Digital Input Ignition pin */
  GPIO_InitStruct.Pin = VEHICLE_IGNITION_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;   
  HAL_GPIO_Init(VEHICLE_IGNITION_PORT, &GPIO_InitStruct);
	
	
//	/*Configure Digital Input Ignition pin */
//  GPIO_InitStruct.Pin = SWITCH_ENABLE_PIN;
//  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
//  GPIO_InitStruct.Pull = GPIO_NOPULL;   
//  HAL_GPIO_Init(SWITCH_ENABLE_PORT, &GPIO_InitStruct);
}


void ChangeSwitchPinToOutPut(void)
{
//    GPIO_InitTypeDef GPIO_InitStruct;
//	
//	  GPIO_InitStruct.Pin = SWITCH_ENABLE_PIN;
//    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//    HAL_GPIO_Init(SWITCH_ENABLE_PORT, &GPIO_InitStruct);
//	
//    HAL_GPIO_WritePin(SWITCH_ENABLE_PORT, SWITCH_ENABLE_PIN, GPIO_PIN_SET);

}

void PutOffCpuSupply(void)
{
	  HAL_GPIO_WritePin(SWITCH_ENABLE_PORT, SWITCH_ENABLE_PIN, GPIO_PIN_RESET);
}

  
/**
 *  @brief getIgnitionPinState() - This Function Reads the status of the Ignition pin (PA1)
 *  If the Ignition is ON Mcu will sense it Low & if the Ignition is OFF Mcu will sense it high.
 *  @return - of type inputState_e if input is high - INPUT_ON
 *                                 if input is low  - INPUT_OFF
 */
inputState_e getIgnitionPinState(void)
{
    if(HAL_GPIO_ReadPin(VEHICLE_IGNITION_PORT, VEHICLE_IGNITION_PIN) == GPIO_PIN_RESET)
    {
        return INPUT_ON;
    }
    else
    {
        return INPUT_OFF;
    }
}


inputState_e getSwitchEnablePinState(void)
{
    if(HAL_GPIO_ReadPin(SWITCH_ENABLE_PORT, SWITCH_ENABLE_PIN) == GPIO_PIN_RESET)
    {
        return INPUT_OFF;
    }
    else
    {
        return INPUT_ON;
    }
}
