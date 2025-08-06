/**
 *  @file          :  mem_port.c
 *  @author        :  Vishnu
 *  @date          :
 *  @brief         :  Brief description
 *  @filerevision  :  2.0
 *  
 */
 
 #ifndef __MEMORYPORT_H
#define __MEMORYPORT_H

#include <stdint.h>
#include "stm32g0xx_hal.h"

#define SPI_MEM_CS_Pin        GPIO_PIN_5
#define SPI_MEM_CS_GPIO_Port  GPIOB

#define SET_CS()	HAL_GPIO_WritePin(SPI_MEM_CS_GPIO_Port, SPI_MEM_CS_Pin, GPIO_PIN_SET)
#define CLR_CS() 	HAL_GPIO_WritePin(SPI_MEM_CS_GPIO_Port, SPI_MEM_CS_Pin, GPIO_PIN_RESET)


void SpiTransmitByte(uint8_t writeData);
uint8_t SpiReceiveByte(void);
void FlashWriteArray(uint8_t *writeData, uint16_t count);
void FlashReadArray(uint8_t *readData, uint16_t count);
void Delay(uint32_t delay);
void InitiliaiseSPIMemoryPorts(void);

#endif