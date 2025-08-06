/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f3xx_hal.h"
//#include "includefiles.h"

/* USER CODE BEGIN Includes */
#include "debug_log.h"
#include "sys_para.h"    
#include "port.h"
#include "gsm_api.h" 
#include "gsm_gprs_api.h"
#include "gps_api.h" 
#include "packet_api.h"
#include "mem_packet_api.h"
#include "mem_config_api.h"
#include "ais_app.h"
#include "sensor_port.h"
#include "bat_port.h"
#include "led_api.h"
#include "fota_api.h"
#include "serial_rfid_api.h"
#include "dht11_api.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
IWDG_HandleTypeDef hiwdg;
rfwmsAppState_et rfwmsCpuState = CPU_IDLE_STATE;
/* USER CODE BEGIN PV */
uint32_t cpuVtgMonitorTick =0;
uint8_t vtgCount = 0;
/* Private variables ---------------------------------------------------------*/
/* USER CODE END PV */
/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
/* USER CODE BEGIN 0 */

void StartWatchdog(void)
{
    /*## Configure & Start the IWDG peripheral ##*/
  /* Set counter reload value to obtain 10sec(10000msec) IWDG TimeOut.
     Counter Reload Value = (LsiFreq(Hz) * Timeout(ms)) / (prescaler * 1000)                         
  */
    hiwdg.Instance = IWDG;
    hiwdg.Init.Prescaler = IWDG_PRESCALER_128;
    hiwdg.Init.Reload = 3125;
    hiwdg.Init.Window = IWDG_WINDOW_DISABLE;
    HAL_IWDG_Init(&hiwdg);
    
    __HAL_DBGMCU_FREEZE_IWDG();  //freeze the IWDG Clock when MCU is in Debug Mode
//  __HAL_DBGMCU_UNFREEZE_IWDG();
}

void FeedWatchdog(void)
{
    HAL_IWDG_Refresh(&hiwdg);
}

void CPUSetState(rfwmsAppState_et setState)
{
  if(rfwmsCpuState == setState)
  {
    return;
  }
  rfwmsCpuState = setState;
  
  switch(rfwmsCpuState)
  {
     case  CPU_ACTIVE_STATE:
           InitMemory();
           AisAppInit();     //init all the peripherals
           LOG_INFO(CH_GSM,"CPU STATE - ACTIVE STATE");
       break;
       
     case CPU_IDLE_STATE:
//           vtgCount = 0;
//           LOG_INFO(CH_GSM,"CPU STATE - IDLE STATE");
       break;
  
  }

}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *V1.0.0 - 
  *V1.1.0 -  Gprs activation failed for 2 times reset the modem,failed to connect the socket for 5 times deact the gprs,
  *          AT+CMGF=1 AND AT+QENG=1,1 Added cmds when modem reads the sms / when modem is internally reset
  *
  * V1.1.1 - 1] Everytime before reading the SMS AT+CMGF=1 command is sent before AT+CMGR=X command.SMS mode of Modem in "Text mode" can not be set 
  *             in the modems NVRAM and Modem's Text Mode chnages to PDU Mode on HW Reset.

  * V1.1.2 - 1] If failed to Activate the gprs  for 5 Minutes then only RESET The MODEM which was previously on the 2 retries.
  *          2] If failed to Open the socket for 3 minutes  then only deactivate the Gprs which was previously on the 2 retries.
  *
  * V1.1.3 - 20th July 2018 - 1] Default Unit Data first time written in the memory is MH02CE1111 which will be replaced by MH02CE[Last 4 digits of IMEI Number].
  *          which can be further changed by **1234SET_VEHNO=Vehicle No## Sms Command.
  *          2] RFID Reader Changes: Reader Power threshold are by default saved in the memory for 12V which can be get by the Sms Command 
  *             **1234GET_READER_TH##  and can be set by the command **1234SET_READER_TH=Lower Threshold,Upper Threshold,##
  *            When a reader power goes above Upper rfid power Threshold Reader will start after 10 seconds.
  *            When a reader power goes below Lower rfid power Threshold Reader will stop after 2 Minutes.
  *            If user wants to switch on the reader Independant of External Vehicle Battery Supply threshold values then need to set as : 1,1
  *            If wants to switch off then:60,60
  *          3] In Debug Sms The Reader Power Status [Status of SHDN Pin],Main Ip Vtg , Internal Batt Vtg are added parameters.
  *          4] In gsm_gprs_api.c file for gprs activation check the gprsInitState is replaced by flagGprsActivated,
  *             which is used in GsmGprsIsActive and GprsDeactivationHandler too.
  *          5] In gsm_sms.c File when SMS is received by the +CMTI: URC gsmReadSmsFnState was changed which is replaced by setting the
  *             flag flagCmtiUrcRcvd. This flag is changed in the SMS handler.
  *          6] In fota Ftp download failure the error code is also sent in the response Sms.
  *             Lower App version fota sms command when given by User , backup Version and app Version both are Sent in the SMS.
  *          7]  If downloaded fota file size is greater than 0 is checked in the GsmFtpHandler() in gsm_ftp_api.c file.Note: Data Type of this 
  *              variable is also changed.
  *          8]  +CREG: and +CGREG: URC were not handled while waiting for response of the AT command as URC prefix and Command prefix are same for AT+CREG and AT+CGREG command.
  *              Now command and URC's are separated by finding for "," in the received matched prefix string.
  *          9] "+QNITZ:" ,RESP_IGNORE is removed from gsm_process_resp.c as rtc time is updated from this URC also at the start.
  *          10] flagRtcTimeUpdate is set to 1 when valid time is received from the GSM modem by +QNITZ URC , and it is set to 2  when valid time
  *              is received from the gsm modem by CCLK command.
  *              This flag is cleared when the RTC time is updated from the GSM time. Doing this avoids getting old time updated in the RTC register.
  * V1.1.4  - 28th July 2018 1] If failed to read the file header while doing the fota then that file is closed first and then change the modem state to idle.
  *         - 30th July 2018 If Gps data is not recieved for 5 seconds then clear the gps data structure.
  *         -  Clear the rssi , cell ids and reset the socket state when gsm gets deregistered from the network.
  * V1.1.5  - http code added , making the header and socket handler statmachine is chnaged.
  *         - When packet is ready at that time socket will be opened and data will be sent.
  *         - 8th August 2018 - When gsm modem restart /gprs activates/gprs deactivates packethandler state is changed to idle , and socket open command is also reset
  *         - flagGprsActivated is set to 0 when gsm state is chaged to GPRS_DEACTIVATE
  *           - URL Changed to : POST http://pkt.cleanupmumbai.com/pktDump/p2.pl?UN=1234567890&PN=9876543210 //11082018
  *         - HOST : pkt.cleanupmumbai.com
  * V1.1.6 - 17082018 - 1] After GSM Network Registration , Signal strength is queried first and then check for GPRS Network Registration.
  *         -           2] For the Current Upper and Lower threshold values for rfid default values assigned at the start which are of 12V battery.
  *         -           3] Gsm Modem State is added in the packet at the place of emergency status.
  * V1.1.7  - 23082018  - 1] packetReadyCntr is set 0 when live packet is ready so that history data will be sent for half time of live data.
  *                     - 2] GPS Status is updated on the display LED when GPS is valid 2 blinks , when GPS is not valid continuous on.
  *                     - 3] GSM Dual sim mode is changed to single sim now.
  *                     - 4] GSM power reset time is changed to 10 seconds 
  * v1.1.8  - 17092018  - 1] http packet statemachine was getting hung whwn gsm is getting reset/when gprs/gsm nw gets deregistered, that state change is only done at mcu reset.
  *                     - 2] imei number,ccid number,operator name are not stored in memory,.
  * v1.1.9  - 01102018  - 1] HTTP statemachine changed to the ais_app.c file. Pvthandler will handle packets and gsm socket.
  *                     - 2] http url was getting corrupt due  to long sms received in the system, sms buff len is increased to 512, and if any sms received above this len, it is deleted.
  * v1.1.10 - 16102018   - rfid tag read status and rfid state machine state replaced at mcc4 and mnc4.
  *                      - E2 51 44 6E 65 74 00 00 00 00 00 00
  * v1.1.11 - 26112018  - gprs act time reset removed
  *                     - rfid  led state is reset when reader is off
  * v1.1.12  -          - 1] Dht11 sensor handler added, RH sensitivity and temperature values calculated.
  * v1.1.13  - 26022019  -  1]Default Unit Id 7000 is not stored in the memory now,when data read from mem is 0xffff then loading 7000 unit id on the ram
  *                         2] #rd last param in packet analog data 1 is peplaced with the cpu vdda i.e. vrefInt
  *                         3]DHT11 data sensor values added in the packet i.e. relative sensitivity,temperature(LAC4 and CID4 is replaced accordingly.
  *                         4]Date corruption issue due to gps handled by checking the time difference between the gps time and gsm time
  *                         5] For checking the gps validity gps fix bit is also considered with gps validity bit 'A'
  *                         6] If no data activity for 20 min OR unable to open socket for 20 minutes then power reset the gsm module
  *                         7] On Unit power reset check the reference voltage is >3.0 for 3 seconds ,after that only cpu will run to main app state OR else cpu will be in a idle state.
  *          - 270619      - 1]Winbond memory added on new boards.                             
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();
  MX_GPIO_Init();
  AisDebugInit();
  AppVersionPrint();
  InitBatteryConfig();     /*Init Battery Monitor channels*/
  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  
  /* USER CODE BEGIN 2 */
  if(WATCHDOG_ENABLE)
  {
     StartWatchdog();
  }
  
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while(1)
  {
   if(WATCHDOG_ENABLE)
    {
        FeedWatchdog();
    }
    BatteryHandler();            /*Battery Handler will run independantly to get the vtg value*/
    
    switch(rfwmsCpuState)
    {
       case CPU_ACTIVE_STATE:
            Led_State_Handler();
            GsmHandler();
            GpsNmeaResponseHandler();
            InputHandler();
            AnalogDataHandler();
            FuelDataHandler();
            RfidReaderHandler();
            PacketHandler();
            PvtSendHandler();
            FotaHandler();
            serialCmdHandler();
            Dht11DataHandler(); 
       break; 
        
       case  CPU_IDLE_STATE:
             if(TimeSpent(cpuVtgMonitorTick,MAX_VOLATGE_CHECK_COUNT))
             {
                  vtgCount++;
                  if(vtgCount >=  1)  //after 5 seconds check the analog reference vtg
                  {
                    if(MonitorUnitVoltage() == 1)
                    {
                        vtgCount = 0;
                        CPUSetState(CPU_ACTIVE_STATE);
                    }
                    else
                    {
                      vtgCount = 0;
                    }
                  }
                  cpuVtgMonitorTick = GetStartTime();
             }
         break;
    }
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

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;
  
  /**Configure LSE Drive Capability 
    */
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);
  
    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSE
                              |RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;  
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);
//  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
//  {
//    _Error_Handler(__FILE__, __LINE__);
//  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);

//  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
//  {
//    _Error_Handler(__FILE__, __LINE__);
//  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_USART2
                              |RCC_PERIPHCLK_USART3|RCC_PERIPHCLK_UART4
                              |RCC_PERIPHCLK_UART5|RCC_PERIPHCLK_I2C1
                              |RCC_PERIPHCLK_ADC12|RCC_PERIPHCLK_RTC;
  
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
  PeriphClkInit.Uart4ClockSelection = RCC_UART4CLKSOURCE_PCLK1;
  PeriphClkInit.Uart5ClockSelection = RCC_UART5CLKSOURCE_PCLK1;
  PeriphClkInit.Adc12ClockSelection = RCC_ADC12PLLCLK_DIV1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);
  
//  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
//  {
//    _Error_Handler(__FILE__, __LINE__);
//  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}
/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{
   
  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

//#ifdef  USE_FULL_ASSERT
///**
//  * @brief  Reports the name of the source file and the source line number
//  *         where the assert_param error has occurred.
//  * @param  file: pointer to the source file name
//  * @param  line: assert_param error line source number
//  * @retval None
//  */
//void assert_failed(uint8_t* file, uint32_t line)
//{ 
//  /* USER CODE BEGIN 6 */
//  /* User can add his own implementation to report the file name and line number,
//     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
//  /* USER CODE END 6 */
//}
//#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
