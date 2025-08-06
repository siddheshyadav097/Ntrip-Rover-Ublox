/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "adc.h"
#include "iwdg.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"
#include "ais_app.h"
#include "mem_packet_api.h"
#include "led_api.h"
#include "fota_api.h"
#include "dig_ip_handler.h"
#include "serial_rfid_api.h"
#include "buzzer_app.h"
#include "digio_api.h"
#include "digio_port.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE BEGIN PFP */
#define APPLICATION_ADDRESS     (uint32_t)0x08004000
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
//#if   (defined ( __CC_ARM ))
//__IO uint32_t VectorTable[48] __attribute__((at(0x20000000)));
//#elif (defined (__ICCARM__))
//#pragma location = 0x20000000
//__no_init __IO uint32_t VectorTable[48];
//#elif defined   (  __GNUC__  )
//__IO uint32_t VectorTable[48] __attribute__((section(".RAMVectorTable")));
//#endif
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
//void AppCopyVecTableAddress(void)
//{
//   uint32_t i = 0;
//   
//   /* Relocate by software the vector table to the internal SRAM at 0x20000000 ***/  

//  /* Copy the vector table from the Flash (mapped at the base of the application
//     load address 0x08004000) to the base address of the SRAM at 0x20000000. */
//  for(i = 0; i < 48; i++)
//  {
//    VectorTable[i] = *(__IO uint32_t*)(APPLICATION_ADDRESS + (i<<2));
//  }

//  /* Enable the SYSCFG peripheral clock*/
//  __HAL_RCC_SYSCFG_CLK_ENABLE(); 
//  /* Remap SRAM at 0x00000000 */
//  __HAL_SYSCFG_REMAPMEMORY_SRAM();

//}
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
golfCpuState_et golfCpuState =CPU_ACTIVE_STATE;//CPU_IDLE_STATE;//CPU_ACTIVE_STATE;//CPU_IDLE_STATE;//CPU_ACTIVE_STATE;//CPU_IDLE_STATE;//CPU_ACTIVE_STATE
uint32_t cpuActiveStateTime = 0;


void CPUSetState(golfCpuState_et setState)
{
  if(golfCpuState == setState)
  {
    return;
  }
  golfCpuState = setState;
  
  switch(golfCpuState)
  {
     case  CPU_ACTIVE_STATE:
          // InitMemory();
					// AisAppInit();     //init all the peripherals			 
           LOG_INFO(CH_GSM,"CPU STATE - ACTIVE STATE");
		       cpuActiveStateTime  = GetStartTime();
       break;
       
     case CPU_IDLE_STATE:
//           vtgCount = 0;
           LOG_INFO(CH_GSM,"CPU STATE - IDLE STATE");
       break;
  
  }

}

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
//void StartWatchdog(void)
//{
//    /*## Configure & Start the IWDG peripheral ##*/
//  /* Set counter reload value to obtain 10sec(10000msec) IWDG TimeOut.
//     Counter Reload Value = (LsiFreq(Hz) * Timeout(ms)) / (prescaler * 1000)                         
//  */
//    hiwdg.Instance = IWDG;
//    hiwdg.Init.Prescaler = IWDG_PRESCALER_128;
//    hiwdg.Init.Reload = 3125;
//    hiwdg.Init.Window = IWDG_WINDOW_DISABLE;
//    HAL_IWDG_Init(&hiwdg);
//    
////__HAL_DBGMCU_FREEZE_IWDG();  //freeze the IWDG Clock when MCU is in Debug Mode
////  __HAL_DBGMCU_UNFREEZE_IWDG();
//}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
//uint8_t writeBuff[8] = {0x12,0x34,0x56,0x78,0x12,0x34,0x56,0x78,};
//unsigned long long int doubleWord = 0;
//uint32_t bankSize = 0;

//uint8_t writeBuff[8] = {0x12,0x34,0x56,0x78,0x98,0x88,0x78,0x68};
//uint32_t crcsize1 = 0, backCrc1 =0;
/* USER CODE END 0 */
DigitalInput_t*  switchStatusStruct= NULL;

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
 __enable_irq();  /* enable global interuppts after jumping */
    /*Reset of all peripherals, Initializes the Flash interface and the Systick.*/
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/
  
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  /* Configure the system clock */
  SystemClock_Config();
  /* USER CODE END Init */
  /* Configure the system clock */
  /* USER CODE BEGIN SysInit */
  /* USER CODE BEGIN SysInit */
  //AppCopyVecTableAddress();
  /* USER CODE END SysInit */
	
  /* Initialize all configured peripherals */
   MX_GPIO_Init();
	 AisDebugInit();
   AppVersionPrint();
	 InitDigitalIOConfig();
	
	 InitMemory();
	 AisAppInit();     //init all the peripherals			 
	// LOG_INFO(CH_GSM,"CPU STATE - ACTIVE STATE");

/* USER CODE BEGIN 2 */
  if(WATCHDOG_ENABLE)
  {
  MX_IWDG_Init();
  }
  //	QbootFlashBackupOpen();		 
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		  if(WATCHDOG_ENABLE)
    {
        FeedWatchdog();
    }
		//InputHandler();
    /* USER CODE END WHILE */
//		 switch(golfCpuState)
//    {
//       case CPU_ACTIVE_STATE:
		
						Led_State_Handler();
						GsmHandler();
						GpsNmeaResponseHandler();
						BatteryHandler();
						PacketHandler();
						PvtSendHandler();
						FotaHandler();
		        BuzzerHandler();
	          StartRelayHandler();
						
						//////
						NtripPacketHandler();
						NtripSendHandler();
						rtcm_header_Parse();
						//////
						//serialCmdHandler();
			 
//			       if(TimeSpent(cpuActiveStateTime,CPU_PUT_OFF_TIME_MS))
//						 {
//									//after 5 hours put off everything cpuActiveStateTime
//									TurnOffGsmPowerRegulatorSwitch();
//									PutOffGpsSupply();
//									PutOffCpuSupply();
//						 }
//			 break;
//			 
//			case  CPU_IDLE_STATE:
////					 switchStatusStruct  = GetDigitalInputStatus();
////					 if(switchStatusStruct->switchIpState == INPUT_ON)
////					 {
////							ChangeSwitchPinToOutPut();
////							CPUSetState(CPU_ACTIVE_STATE);
////					 }
//			 break;
    //}
			 
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 8;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
	
//	 HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

//    /**Configure the Systick 
//    */
//  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

//  /* SysTick_IRQn interrupt configuration */
//  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USER CODE BEGIN 4 */
/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{
  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}


void FeedWatchdog(void)
{
    HAL_IWDG_Refresh(&hiwdg);
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */


/********* To test the memory
 SystemClock_Config();
	
	QbootFlashBackupOpen();
	
	//QbootFlashWrite(BACKUP_START_ADDR,writeBuff,8);
	
	//bankSize = FLASH_BANK_SIZE;
	//crcsize = fotaReadFileSize - sizeof(appHeader_st);
//backCrc = QbootBackupGetCrc(crcsize);
*///////////
