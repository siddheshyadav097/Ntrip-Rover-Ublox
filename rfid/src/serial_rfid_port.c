
 
/*----includes-----*/
#include "debug_log.h"
#include "serial_rfid_port.h"
#include <math.h>


UART_HandleTypeDef huart3_rfidSerial;

// function pointer of the receive callback function this function should be called in the receive
// data ISR
RfidReceiveDataCbFnPtr_t RfReceiveDataFnPtr = NULL;


void InitSerialPwrkey(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  
  GPIO_InitStruct.Pin = SERIAL_DISABLE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SERIAL_DISABLE_GPIO_Port, &GPIO_InitStruct);
    
  HAL_GPIO_WritePin(SERIAL_DISABLE_GPIO_Port, SERIAL_DISABLE_Pin, GPIO_PIN_SET);
	
	/****************************BUZZER EN PIN****************/
	

	
}



/*----public functions-----*/ 
void InitRfidSerialPort(RfidReceiveDataCbFnPtr_t rfFnPtr)
{
  __HAL_RCC_GPIOB_CLK_ENABLE();
  
  __HAL_RCC_GPIOD_CLK_ENABLE();
	
	InitSerialPwrkey();
  
  huart3_rfidSerial.Instance       = RFID_DATA_UART;
   huart3_rfidSerial.Init.BaudRate = 115200;
  huart3_rfidSerial.Init.WordLength = UART_WORDLENGTH_8B;
  huart3_rfidSerial.Init.StopBits = UART_STOPBITS_1;
  huart3_rfidSerial.Init.Parity = UART_PARITY_NONE;
  huart3_rfidSerial.Init.Mode = UART_MODE_TX_RX;
  huart3_rfidSerial.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3_rfidSerial.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3_rfidSerial.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3_rfidSerial.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart3_rfidSerial.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	
	  RfReceiveDataFnPtr                            = rfFnPtr;

	
  if (HAL_UART_Init(&huart3_rfidSerial) != HAL_OK)
  {
    //Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart3_rfidSerial, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    //Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart3_rfidSerial, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    //Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart3_rfidSerial) != HAL_OK)
  {
    //Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */
   __HAL_UNLOCK(&huart3_rfidSerial);
	__HAL_UART_ENABLE_IT(&huart3_rfidSerial, UART_IT_RXNE);
	
}


/**
 *  @brief  : The port pins and clock of UART is initialised here
 *  @return : none
 */
void RfidUartMspPortInit(void)
{
    /* USER CODE BEGIN USART3_MspInit 0 */
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  /* USER CODE END USART3_MspInit 0 */

  /** Initializes the peripherals clocks
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART3;
    PeriphClkInit.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
	
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      //Error_Handler();
    }

    /* USART3 clock enable */
    __HAL_RCC_USART3_CLK_ENABLE();

    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**USART3 GPIO Configuration
    PD8     ------> USART3_TX
    PD9     ------> USART3_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP; //GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL; //GPIO_PULLDOWN;//
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF0_USART3;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* USART4 interrupt Init */
    HAL_NVIC_SetPriority(USART3_4_5_6_LPUART1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART3_4_5_6_LPUART1_IRQn);
  /* USER CODE BEGIN USART3_MspInit 1 */

  /* USER CODE END USART3_MspInit 1 */
}

void RfidMspUartPortDeInit(void)
{
  /* USER CODE BEGIN USART3_MspDeInit 0 */

  /* USER CODE END USART3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART3_CLK_DISABLE();

     /**USART3 GPIO Configuration
    PD8     ------> USART3_TX
    PD9     ------> USART3_RX
    */
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_8|GPIO_PIN_9);

    /* USART3 interrupt Deinit */
  /* USER CODE BEGIN USART3:USART3_4_5_6_LPUART1_IRQn disable */
    /**
    * Uncomment the line below to disable the "USART3_4_5_6_LPUART1_IRQn" interrupt
    * Be aware, disabling shared interrupt may affect other IPs
    */
    /* HAL_NVIC_DisableIRQ(USART3_4_5_6_LPUART1_IRQn); */
  /* USER CODE END USART3:USART3_4_5_6_LPUART1_IRQn disable */

  /* USER CODE BEGIN USART3_MspDeInit 1 */

  /* USER CODE END USART3_MspDeInit 1 */
}

/**
 * Get the status of the serial pwr SHDN Pin
 */
uint8_t GetSerialPwrKeyState(void)  //if pin level is 0 output vtg is generated
{
    if(HAL_GPIO_ReadPin(SERIAL_DISABLE_GPIO_Port, SERIAL_DISABLE_Pin) == GPIO_PIN_SET)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}  

void SerialControlConfig(uint8_t disableFlag)    //inverted logic
{
    if(disableFlag == 1)
    {
        HAL_GPIO_WritePin(SERIAL_DISABLE_GPIO_Port, SERIAL_DISABLE_Pin, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(SERIAL_DISABLE_GPIO_Port, SERIAL_DISABLE_Pin, GPIO_PIN_RESET);
    }
}
/**
 *  @brief  : This function sends the data to UART
 *  @param  :[in] sendData - array byte
 *  @param  :[in] sendLen  number of bytes to be sent
 *  @return :Return_Description
 */
void RfidUartSendData(uint8_t *sendData, uint16_t sendLen)
{
    HAL_UART_Transmit(&huart3_rfidSerial, (uint8_t *)sendData, sendLen, 500);
}

/**
* @brief This function handles GSM - USART1 global interrupt.
*/
//void UART3_IRQHandler(void)  
//{
//    if(__HAL_UART_GET_IT_SOURCE(&huart5_rfidSerial, UART_IT_RXNE)!= RESET)
//    {
//        RfReceiveDataFnPtr(RFID_DATA_UART->RDR & 0xff);
//    }
//    if(__HAL_UART_GET_FLAG(&huart5_rfidSerial, UART_FLAG_ORE)!= RESET) 
//    {
//        __HAL_UART_CLEAR_OREFLAG(&huart5_rfidSerial);
//    }
//    if(__HAL_UART_GET_FLAG(&huart5_rfidSerial, UART_FLAG_PE)!= RESET) 
//    {
//        __HAL_UART_CLEAR_PEFLAG(&huart5_rfidSerial);
//    }
//    if(__HAL_UART_GET_FLAG(&huart5_rfidSerial, UART_FLAG_FE)!= RESET) 
//    {
//        __HAL_UART_CLEAR_FEFLAG(&huart5_rfidSerial);
//    }
//    if(__HAL_UART_GET_FLAG(&huart5_rfidSerial, UART_FLAG_NE)!= RESET)
//    {
//        __HAL_UART_CLEAR_NEFLAG(&huart5_rfidSerial);
//    }
//}
