/**
 *  @file          :  mem_port.c
 *  @author        :  Vishnu
 *  @date          :
 *  @brief         :  Brief description
 *  @filerevision  :  2.0
 *  
 */
 
#include "memoryport.h"

// spi handle 
SPI_HandleTypeDef hspi_mem;


void InitSPIChipSelectPin(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin = SPI_MEM_CS_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(SPI_MEM_CS_GPIO_Port,&GPIO_InitStruct);
    HAL_GPIO_WritePin(SPI_MEM_CS_GPIO_Port, SPI_MEM_CS_Pin, GPIO_PIN_SET);   //Reset the gsm Enable pin

}
void InitiliaiseSPIMemoryPorts(void)
{
  InitSPIChipSelectPin();
	
	 /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  hspi_mem.Instance = SPI2;
  hspi_mem.Init.Mode = SPI_MODE_MASTER;
  hspi_mem.Init.Direction = SPI_DIRECTION_2LINES;
  hspi_mem.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi_mem.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi_mem.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi_mem.Init.NSS = SPI_NSS_SOFT;
  hspi_mem.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi_mem.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi_mem.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi_mem.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi_mem.Init.CRCPolynomial = 7;
  hspi_mem.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi_mem.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi_mem) != HAL_OK)
  {
    //Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

//  hspi_mem.Instance = SPI1;
//  hspi_mem.Init.Mode = SPI_MODE_MASTER;
//  hspi_mem.Init.Direction = SPI_DIRECTION_2LINES;
//  hspi_mem.Init.DataSize = SPI_DATASIZE_8BIT;
//  hspi_mem.Init.CLKPolarity = SPI_POLARITY_LOW;
//  hspi_mem.Init.CLKPhase = SPI_PHASE_1EDGE;
//  hspi_mem.Init.NSS = SPI_NSS_SOFT;
//  hspi_mem.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
//  hspi_mem.Init.FirstBit = SPI_FIRSTBIT_MSB;
//  hspi_mem.Init.TIMode = SPI_TIMODE_DISABLE;
//  hspi_mem.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
//  hspi_mem.Init.CRCPolynomial = 10;
//  if (HAL_SPI_Init(&hspi_mem) != HAL_OK)
//  {
//    _Error_Handler(__FILE__, __LINE__);
//  }

    SET_CS();
    Delay(10);
}



void SpiTransmitByte(uint8_t writeData)
{	
    HAL_SPI_Transmit(&hspi_mem,&writeData,1,500);
}


uint8_t SpiReceiveByte(void)
{
    uint8_t data = 0;														
    HAL_SPI_Receive(&hspi_mem, &data, 1, 500);
    return data;
}


void FlashWriteArray(uint8_t *writeData, uint16_t count)
{
    HAL_SPI_Transmit(&hspi_mem,writeData,count,500);
}


void FlashReadArray(uint8_t *readData, uint16_t count)
{
    HAL_SPI_Receive(&hspi_mem, readData, count, 500);
}


void Delay(uint32_t delay)
{
    HAL_Delay(delay);
}

