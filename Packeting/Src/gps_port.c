/**
 *  @file          :  gps_port.c
 *  @author        :  Aakash/Ratna
 *  @date          :  02/04/2017
 *  @brief         :  GPS Serial initialization and nmea data acquisition routines are defined.
 *  				  The GPS data acquired is passed onto the application.
 *  @filerevision  :  1.0
 *  
 */

 
 /*----includes-----*/
#include "debug_log.h"
#include "gps_port.h"
#include "gps_api.h"


 /*----variables-----*/
//uint8_t cpy_gpsdata_buff[2048];
uint8_t chksum_buff[150]= {0};
UART_HandleTypeDef huart_gps;

// function pointer of the receive callback function this function should be called in the receive
// data ISR
GpsReceiveDataCbFnPtr_t GPSReceiveDataFnPtr = NULL;

static void GPS_UART_Init(GpsReceiveDataCbFnPtr_t fnPtr)
{
	/* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart_gps.Instance = USART2;
  huart_gps.Init.BaudRate = 38400;//115200;
  huart_gps.Init.WordLength = UART_WORDLENGTH_8B;
  huart_gps.Init.StopBits = UART_STOPBITS_1;
  huart_gps.Init.Parity = UART_PARITY_NONE;
  huart_gps.Init.Mode = UART_MODE_TX_RX;
  huart_gps.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart_gps.Init.OverSampling = UART_OVERSAMPLING_16;
  huart_gps.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart_gps.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart_gps.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart_gps) != HAL_OK)
  {
    //Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart_gps, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    //Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart_gps, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    //Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart_gps) != HAL_OK)
  {
    //Error_Handler();
  }
	
	 /* USER CODE BEGIN USART3_Init 2 */
   __HAL_UNLOCK(&huart_gps);
	__HAL_UART_ENABLE_IT(&huart_gps, UART_IT_RXNE);
  /* USER CODE END USART3_Init 2 */
  /* USER CODE BEGIN USART2_Init 2 */
        GPSReceiveDataFnPtr = fnPtr;
  /* USER CODE END USART2_Init 2 */
}

 /*----public functions-----*/
 
/**
 *  @brief Initializes GPS module by Registering the specified serial port, in this case we are using UART_PORT3
 *  UART callback function is used to receive the UART notification from core system. After registering 
 *  the specified port, This function opens a specified UART port with the specified flow control mode. 
 *  It also has to initiates the gpio to enable the GPS module(Quectel L80)
 *  @return void
 */
void InitGPS(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    GpsRingBuffInit();
    GPS_UART_Init(GpsUartReceiveDataCb);
    
	  //PB9
    GPIO_InitStruct.Pin = GPS_ENABLE_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPS_ENABLE_GPIO_Port, &GPIO_InitStruct);
	
	  //PD4
	  GPIO_InitStruct.Pin = GPS_RESET_CTRL_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPS_RESET_CTRL_Port, &GPIO_InitStruct);
	
    HAL_GPIO_WritePin(GPS_ENABLE_GPIO_Port, GPS_ENABLE_Pin, GPIO_PIN_SET);
	
	 // HAL_GPIO_WritePin(GPS_RESET_CTRL_Port, GPS_RESET_CTRL_Pin, GPIO_PIN_RESET);
}

void PutOffGpsSupply(void)
{
    HAL_GPIO_WritePin(GPS_ENABLE_GPIO_Port, GPS_ENABLE_Pin, GPIO_PIN_RESET);
}
/**
 *  @brief This function is used to verify the calculated checksum with available one in GPRMC string
 *  For calculating the checksum, xoring each character of string with previous character. then the final
 *  checksum value is compared with available checksum value. if it's equal then return 1 else 0
 *  @param [in] strBuf pointer for strBuf
 *  @param [in] str_len length of string buffer
 *  @param [in] chksum value received in GPRMC string
 *  @return Boolean 1 or 0
 */
BOOL Verify_Checksum(uint8_t *strBuf, uint16_t str_len, uint8_t chksum)
{
		uint8_t cal_chksum = 0;
		uint8_t i;
		
		memset(chksum_buff,0,sizeof(chksum_buff));
		memcpy(chksum_buff, strBuf, sizeof(chksum_buff));
		
		for(i=0; i < str_len; i++)
		{
			cal_chksum = chksum_buff[i] ^ cal_chksum;
		}
		if(cal_chksum == chksum)
		{
			return True;
		}
		return False;
}
/**
  * @brief This function handles USART2 + LPUART2 Interrupt.
  */

void USART2_LPUART2_IRQHandler(void)
{
    if(__HAL_UART_GET_IT_SOURCE(&huart_gps, UART_IT_RXNE)!= RESET)
	  {
		GPSReceiveDataFnPtr((GPS_UART->RDR & 0xff));
	  }  
	  if(__HAL_UART_GET_FLAG(&huart_gps, UART_FLAG_ORE)!= RESET) 
	  {
		__HAL_UART_CLEAR_OREFLAG(&huart_gps);
	  }
	  if(__HAL_UART_GET_FLAG(&huart_gps, UART_FLAG_PE)!= RESET) 
	  {
		__HAL_UART_CLEAR_PEFLAG(&huart_gps);
	  }
	  if(__HAL_UART_GET_FLAG(&huart_gps, UART_FLAG_FE)!= RESET) 
	  {
		 __HAL_UART_CLEAR_FEFLAG(&huart_gps);
	  }
	  if(__HAL_UART_GET_FLAG(&huart_gps, UART_FLAG_NE)!= RESET)
	  {
		__HAL_UART_CLEAR_NEFLAG(&huart_gps);
	  }
	  
	  /* USER CODE BEGIN USART3_IRQn 0 */

	  /* USER CODE END USART3_IRQn 0 */
	//  HAL_UART_IRQHandler(&huart_gsm);
	  /* USER CODE BEGIN USART3_IRQn 1 */

	  /* USER CODE END USART3_IRQn 1 */
}

void SendRtcm(uint8_t *sendData,uint16_t sendLen)
{
	HAL_UART_Transmit(&huart_gps,sendData,sendLen,100);
}