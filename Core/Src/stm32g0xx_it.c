/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32g0xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32g0xx_it.h"
#include "gsm_port.h"
#include "serial_rfid_port.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huartGsm;
extern UART_HandleTypeDef huart6;

extern DMA_HandleTypeDef hdma_adc1;
extern ADC_HandleTypeDef hadc_supplysense;
/* USER CODE BEGIN EV */
extern UART_HandleTypeDef huartGsm;

extern UART_HandleTypeDef huart3_rfidSerial;

// function pointer of the receive callback function this function should be called in the receive
// data ISR
extern RfidReceiveDataCbFnPtr_t RfReceiveDataFnPtr;

/* USER CODE END EV */
extern GsmReceiveDataCbFnPtr_t ReceiveDataFnPtr;

/******************************************************************************/
/*           Cortex-M0+ Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
  while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVC_IRQn 0 */

  /* USER CODE END SVC_IRQn 0 */
  /* USER CODE BEGIN SVC_IRQn 1 */

  /* USER CODE END SVC_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
/**
* @brief This function handles System tick timer.
*/
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32G0xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32g0xx.s).                    */
/******************************************************************************/
/**
  * @brief This function handles DMA1 channel 1 interrupt.
  */
void DMA1_Channel1_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel1_IRQn 0 */

  /* USER CODE END DMA1_Channel1_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_adc1);
  /* USER CODE BEGIN DMA1_Channel1_IRQn 1 */

  /* USER CODE END DMA1_Channel1_IRQn 1 */
}

/**
  * @brief This function handles ADC1 interrupt.
  */
void ADC1_IRQHandler(void)
{
  /* USER CODE BEGIN ADC1_IRQn 0 */

  /* USER CODE END ADC1_IRQn 0 */
  HAL_ADC_IRQHandler(&hadc_supplysense);
  /* USER CODE BEGIN ADC1_IRQn 1 */

  /* USER CODE END ADC1_IRQn 1 */
}

/**
  * @brief  This function handles ADC interrupt request.
  * @param  None
  * @retval None
  */
void ADCx_IRQHandler(void)
{
  HAL_ADC_IRQHandler(&hadc_supplysense);
}

/**
  * @brief This function handles USART3, USART4, USART5, USART6, LPUART1 globlal Interrupts (combined with EXTI 28).
  */
void USART3_4_5_6_LPUART1_IRQHandler(void)
{
  /* USER CODE BEGIN USART3_4_5_6_LPUART1_IRQn 0 */

  /* USER CODE END USART3_4_5_6_LPUART1_IRQn 0 */
//  HAL_UART_IRQHandler(&huart3_rfidSerial);
//  HAL_UART_IRQHandler(&huartGsm);
	///////////////////////////IRQ FOR GSM/////////////////////////////
	if (__HAL_GET_PENDING_IT(HAL_ITLINE_USART4)!= RESET)
   {
	   if(__HAL_UART_GET_IT_SOURCE(&huartGsm, UART_IT_RXNE)!= RESET)
    {
        ReceiveDataFnPtr((USART4->RDR & 0xff));
    }
    if(__HAL_UART_GET_FLAG(&huartGsm, UART_FLAG_ORE)!= RESET) 
    {
        __HAL_UART_CLEAR_OREFLAG(&huartGsm);
    }
    if(__HAL_UART_GET_FLAG(&huartGsm, UART_FLAG_PE)!= RESET) 
    {
        __HAL_UART_CLEAR_PEFLAG(&huartGsm);
    }
    if(__HAL_UART_GET_FLAG(&huartGsm, UART_FLAG_FE)!= RESET) 
    {
        __HAL_UART_CLEAR_FEFLAG(&huartGsm);
    }
    if(__HAL_UART_GET_FLAG(&huartGsm, UART_FLAG_NE)!= RESET)
    {
        __HAL_UART_CLEAR_NEFLAG(&huartGsm);
    }
	}
	 
	if (__HAL_GET_PENDING_IT(HAL_ITLINE_USART3)!= RESET)
   {
	  if(__HAL_UART_GET_IT_SOURCE(&huart3_rfidSerial, UART_IT_RXNE)!= RESET)
    {
        RfReceiveDataFnPtr(USART3->RDR & 0xff);
    }
    if(__HAL_UART_GET_FLAG(&huart3_rfidSerial, UART_FLAG_ORE)!= RESET) 
    {
        __HAL_UART_CLEAR_OREFLAG(&huart3_rfidSerial);
    }
    if(__HAL_UART_GET_FLAG(&huart3_rfidSerial, UART_FLAG_PE)!= RESET) 
    {
        __HAL_UART_CLEAR_PEFLAG(&huart3_rfidSerial);
    }
    if(__HAL_UART_GET_FLAG(&huart3_rfidSerial, UART_FLAG_FE)!= RESET) 
    {
        __HAL_UART_CLEAR_FEFLAG(&huart3_rfidSerial);
    }
    if(__HAL_UART_GET_FLAG(&huart3_rfidSerial, UART_FLAG_NE)!= RESET)
    {
        __HAL_UART_CLEAR_NEFLAG(&huart3_rfidSerial);
    }
	}
	 
  /* USER CODE BEGIN USART3_4_5_6_LPUART1_IRQn 1 */

  /* USER CODE END USART3_4_5_6_LPUART1_IRQn 1 */
}


/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
