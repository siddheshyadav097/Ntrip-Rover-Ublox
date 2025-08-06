#include "debug_log_port.h"
#include "ais_app.h"

UART_HandleTypeDef huartDebug;

GetSerialFnPtr GetSerialCommand = NULL;

void DebugLogInitPort(GetSerialFnPtr serialDataCallBack)
{
	/* USER CODE BEGIN USART6_Init 0 */

  /* USER CODE END USART6_Init 0 */

  /* USER CODE BEGIN USART6_Init 1 */

  /* USER CODE END USART6_Init 1 */
  huartDebug.Instance = USART6;
  huartDebug.Init.BaudRate = 115200;
  huartDebug.Init.WordLength = UART_WORDLENGTH_8B;
  huartDebug.Init.StopBits = UART_STOPBITS_1;
  huartDebug.Init.Parity = UART_PARITY_NONE;
  huartDebug.Init.Mode = UART_MODE_TX_RX;
  huartDebug.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huartDebug.Init.OverSampling = UART_OVERSAMPLING_16;
  huartDebug.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huartDebug.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huartDebug.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huartDebug) != HAL_OK)
  {
    //Error_Handler();
  }
	
//	if (HAL_UARTEx_SetTxFifoThreshold(&huartDebug, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
//  {
//    //Error_Handler();
//  }
//  if (HAL_UARTEx_SetRxFifoThreshold(&huartDebug, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
//  {
//    //Error_Handler();
//  }
//  if (HAL_UARTEx_DisableFifoMode(&huartDebug) != HAL_OK)
//  {
//    //Error_Handler();
//  }
//  /* USER CODE BEGIN USART3_Init 2 */
//   __HAL_UNLOCK(&huartDebug);
//	__HAL_UART_ENABLE_IT(&huartDebug, UART_IT_RXNE);
//  /* USER CODE END USART3_Init 2 */
//	
//	GetSerialCommand = serialDataCallBack;
  /* USER CODE BEGIN USART6_Init 2 */

  /* USER CODE END USART6_Init 2 */
}

void DebugLogSend(uint8_t *sendData,uint16_t sendLen)
{
	HAL_UART_Transmit(&huartDebug,sendData,sendLen,100);
}

/**
* @brief This function handles GSM - USART1 global interrupt.
*/
//void DebugIRQHandler(void)  
//{ 
//    uint8_t debugCmd;
//    
//    if(__HAL_UART_GET_IT_SOURCE(&huartDebug, UART_IT_RXNE)!= RESET)
//    {
//        //debugCmd = DEBUG_UART->RDR & 0xff; // the character from the debug uart register is saved in at ch
//       // GetSerialCommand(&debugCmd, 1);
//      
//      GetSerialCommand((DEBUG_UART->RDR & 0xff));
//    }
//    if(__HAL_UART_GET_FLAG(&huartDebug, UART_FLAG_ORE)!= RESET) 
//    {
//        __HAL_UART_CLEAR_OREFLAG(&huartDebug);
//    }
//    if(__HAL_UART_GET_FLAG(&huartDebug, UART_FLAG_PE)!= RESET) 
//    {
//        __HAL_UART_CLEAR_PEFLAG(&huartDebug);
//    }
//    if(__HAL_UART_GET_FLAG(&huartDebug, UART_FLAG_FE)!= RESET) 
//    {
//        __HAL_UART_CLEAR_FEFLAG(&huartDebug);
//    }
//    if(__HAL_UART_GET_FLAG(&huartDebug, UART_FLAG_NE)!= RESET)
//    {
//        __HAL_UART_CLEAR_NEFLAG(&huartDebug);
//    }
//}


