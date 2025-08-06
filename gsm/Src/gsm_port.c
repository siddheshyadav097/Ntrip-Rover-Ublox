/**
 *  @file          :gsm_port.c
 *  @author        :
 *  @date          :
 *  @brief         :gsm uart port initialise and gsm power enable pin initialise is done here
 *  @filerevision  :1.0
 */
#include "gsm_port.h"
#include "port.h"
 
// UART handle 
UART_HandleTypeDef huartGsm;
//UART_HandleTypeDef huartGsm;

// function pointer of the receive callback function this function should be called in the receive
// data ISR
GsmReceiveDataCbFnPtr_t ReceiveDataFnPtr = NULL;

//RfidReceiveDataCbFnPtr_t RfReceiveDataFnPtr;

/**
 *  @brief  : Initialise the uart port
 *  @param  :[in] fnPtr Pointer of call back function
 *              This function is called when a byte is received and the received byte is passed to this function
 *  @return : none
 */
void GsmUartPortInit(GsmReceiveDataCbFnPtr_t fnPtr)
{
   /* USER CODE BEGIN USART4_Init 0 */

  /* USER CODE END USART4_Init 0 */

  /* USER CODE BEGIN USART4_Init 1 */

  /* USER CODE END USART4_Init 1 */
  huartGsm.Instance = USART4;
  huartGsm.Init.BaudRate = 115200;
  huartGsm.Init.WordLength = UART_WORDLENGTH_8B;
  huartGsm.Init.StopBits = UART_STOPBITS_1;
  huartGsm.Init.Parity = UART_PARITY_NONE;
  huartGsm.Init.Mode = UART_MODE_TX_RX;
  huartGsm.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huartGsm.Init.OverSampling = UART_OVERSAMPLING_16;
  huartGsm.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huartGsm.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huartGsm.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huartGsm) != HAL_OK)
  {
    //Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huartGsm, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    //Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huartGsm, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    //Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huartGsm) != HAL_OK)
  {
    //Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */
   __HAL_UNLOCK(&huartGsm);
	__HAL_UART_ENABLE_IT(&huartGsm, UART_IT_RXNE);
  /* USER CODE END USART3_Init 2 */
	
    ReceiveDataFnPtr = fnPtr;
}

/**
 *  @brief  : The port pins and clock of UART is initialised here
 *  @return : none
 */
void GsmUartMspPortInit(void)
{
    /* USER CODE BEGIN USART3_MspInit 0 */
 GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  /* USER CODE END USART3_MspInit 0 */

  /** Initializes the peripherals clocks
  */
   // PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART3;
    //PeriphClkInit.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
//    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
//    {
//      //Error_Handler();
//    }

    /* USART4 clock enable */
    __HAL_RCC_USART4_CLK_ENABLE();

    __HAL_RCC_GPIOC_CLK_ENABLE();
    /**USART4 GPIO Configuration
    PC11     ------> USART4_RX
    PC10     ------> USART4_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF1_USART4;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* USART4 interrupt Init */
    HAL_NVIC_SetPriority(USART3_4_5_6_LPUART1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART3_4_5_6_LPUART1_IRQn);
  /* USER CODE BEGIN USART3_MspInit 1 */

  /* USER CODE END USART3_MspInit 1 */
}

void GsmMspUartPortDeInit(void)
{
  /* USER CODE BEGIN USART3_MspDeInit 0 */

  /* USER CODE END USART3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART4_CLK_DISABLE();

    /**USART4 GPIO Configuration
    PC11     ------> USART4_RX
    PC10     ------> USART4_TX
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_11|GPIO_PIN_10);

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
 *  @brief  : Initialise uart power enable port pin
 *  @return : none
 */
void GsmPwrEnableInit(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
	
    GPIO_InitStruct.Pin = GSM_PWR_EN_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	
    HAL_GPIO_Init(GSM_PWR_EN_GPIO_PORT,&GPIO_InitStruct);
    HAL_GPIO_WritePin(GSM_PWR_EN_GPIO_PORT, GSM_PWR_EN_GPIO_PIN, GPIO_PIN_RESET);   //Reset the gsm Enable pin 
	
}

void GsmPwrKeyInit(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin = GSM_PWR_KEY_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	
    HAL_GPIO_Init(GSM_PWR_KEY_GPIO_PORT,&GPIO_InitStruct);
    HAL_GPIO_WritePin(GSM_PWR_KEY_GPIO_PORT, GSM_PWR_KEY_GPIO_PIN, GPIO_PIN_RESET);   //Reset the gsm Powerkey pin 
}

void GsmResetKeyInit(void)
{
//    GPIO_InitTypeDef GPIO_InitStruct;
//    GPIO_InitStruct.Pin = GSM_RESET_KEY_GPIO_PIN;
//    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//	  GPIO_InitStruct.Pull = GPIO_NOPULL;
//    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//	
//    HAL_GPIO_Init(GSM_RESET_KEY_GPIO_PORT,&GPIO_InitStruct);
    
}


  
/**
 *  @brief  :Switch on the gsm power
 *  @return :none
 */
    
void TurnOnGsmPowerRegulatorSwitch(void)
{
    HAL_GPIO_WritePin (GSM_PWR_EN_GPIO_PORT, GSM_PWR_EN_GPIO_PIN,GPIO_PIN_SET);
}

void GsmPowerkeyPinOn(void)
{
    HAL_GPIO_WritePin (GSM_PWR_KEY_GPIO_PORT, GSM_PWR_KEY_GPIO_PIN,GPIO_PIN_SET);
}

/**
 *  @brief  :Switch OFF gsm power
 *  @return :none
 */
void TurnOffGsmPowerRegulatorSwitch(void)
{
    HAL_GPIO_WritePin (GSM_PWR_EN_GPIO_PORT, GSM_PWR_EN_GPIO_PIN,GPIO_PIN_RESET);
}

void GsmPowerkeyPinOff(void)
{
    HAL_GPIO_WritePin (GSM_PWR_KEY_GPIO_PORT, GSM_PWR_KEY_GPIO_PIN,GPIO_PIN_RESET);
}

//void Toggle_GPRS_LED(void)
//{
//  HAL_GPIO_TogglePin(GPRS_LED_PORT,GPRS_LED_PIN);
//}

//void GPRS_LED_KEEP_ON(void)
//{
//  HAL_GPIO_WritePin(GPRS_LED_PORT,GPRS_LED_PIN,GPIO_PIN_RESET);
//}

//void Toggle_PACKET_UPLOAD_LED(void)
//{
//  HAL_GPIO_TogglePin(PACKET_UPLOAD_LED_PORT,PACKET_UPLOAD_LED_PIN);
//}

//void PACKET_UPLOAD_LED_KEEP_ON(void)
//{
//  HAL_GPIO_WritePin(PACKET_UPLOAD_LED_PORT,PACKET_UPLOAD_LED_PIN,GPIO_PIN_RESET);
//}


/**
 *  @brief  : This function sends the data to UART
 *  @param  :[in] sendData - array byte
 *  @param  :[in] sendLen  number of bytes to be sent
 *  @return :Return_Description
 */
gsmUartSendRet_et GsmUartSendData(char *sendData, uint16_t sendLen)
{
    gsmUartSendRet_et TxAck = GSM_SEND_FAILURE;
  
    if(HAL_UART_Transmit(&huartGsm, (uint8_t *)sendData, sendLen, GSM_UART_SEND_TIMEOUT) == HAL_OK)
    {
      TxAck = GSM_SEND_SUCCESS;
    }
    return TxAck;
}

/**
 *  @brief  : This function sends the data to UART
 *  @param  :[in] sendData - array byte
 *  @param  :[in] sendLen  number of bytes to be sent
 *  @return :Return_Description
 */
gsmUartSendRet_et GsmUartSendByte(uint8_t sendData)
{
    gsmUartSendRet_et TxAck = GSM_SEND_FAILURE;
  
    if(HAL_UART_Transmit(&huartGsm, (uint8_t *)&sendData, 1, GSM_UART_SEND_TIMEOUT) == HAL_OK)
    {
      TxAck = GSM_SEND_SUCCESS;
    }
    return TxAck;
}

//void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) 
//{
// 
//   if (huart->Instance == USART3) 
//	{
//    // USART3
//		RfReceiveDataFnPtr(RFID_DATA_UART->RDR & 0xff);
//  }
//	else if (huart->Instance == USART4) 
//	{
//    // USART4
//		ReceiveDataFnPtr((USART3->RDR & 0xff));
//  }
//}

/**
* @brief This function handles GSM - USART6 global interrupt.
*/
//void USART3_4_5_6_LPUART1_IRQHandler(void)  //USART6_IRQHandler
//{
//	
//    if(__HAL_UART_GET_IT_SOURCE(&huartGsm, UART_IT_RXNE)!= RESET)
//    {
//        ReceiveDataFnPtr((USART3->RDR & 0xff));
//    }
//    if(__HAL_UART_GET_FLAG(&huartGsm, UART_FLAG_ORE)!= RESET) 
//    {
//        __HAL_UART_CLEAR_OREFLAG(&huartGsm);
//    }
//    if(__HAL_UART_GET_FLAG(&huartGsm, UART_FLAG_PE)!= RESET) 
//    {
//        __HAL_UART_CLEAR_PEFLAG(&huartGsm);
//    }
//    if(__HAL_UART_GET_FLAG(&huartGsm, UART_FLAG_FE)!= RESET) 
//    {
//        __HAL_UART_CLEAR_FEFLAG(&huartGsm);
//    }
//    if(__HAL_UART_GET_FLAG(&huartGsm, UART_FLAG_NE)!= RESET)
//    {
//        __HAL_UART_CLEAR_NEFLAG(&huartGsm);
//    }
//		
//		//DebugIRQHandler();
//    
//}


/**
  * @brief This function handles USART3, USART4, USART5, USART6, LPUART1 globlal Interrupts (combined with EXTI 28).
  */
//void USART3_4_5_6_LPUART1_IRQHandler(void)
//{
//  /* USER CODE BEGIN USART3_4_5_6_LPUART1_IRQn 0 */

//  /* USER CODE END USART3_4_5_6_LPUART1_IRQn 0 */
//  HAL_UART_IRQHandler(&huartGsm);
//  HAL_UART_IRQHandler(&huart6);
//  /* USER CODE BEGIN USART3_4_5_6_LPUART1_IRQn 1 */

//  /* USER CODE END USART3_4_5_6_LPUART1_IRQn 1 */
//}
