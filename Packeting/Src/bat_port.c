/**
 *  @file          :  bat_port.c
 *  @author        :  Aakash/Ratna
 *  @date          :  01/04/2017
 *  @brief         :  Monitoring the hardware to sense the battery voltage and the charging status of the battery.
 *  				  The battery level is measured using ADC and provided to the application for monitoring along with the status showing,
 *  				  if charging is ongoing or discontinued. 
 *  @filerevision  :  1.0
 *  
 */

 
  /*----includes-----*/
#include "debug_log.h"
#include "bat_port.h"


 /*----variables-----*/
//uint32_t battVoltage = 0;
//uint16_t adcValBuff[5] = {0,0,0,0,0};
ADC_HandleTypeDef hadc_supplysense;
DMA_HandleTypeDef hdma_adc2;

__IO   uint16_t   aADCxConvertedData[ADC_CONVERTED_DATA_BUFFER_SIZE]; /* ADC group regular conversion data (array of data) */
extern __IO   uint8_t ubDmaTransferStatus; /* Variable set into DMA interruption callback */
uint32_t tmp_index_adc_converted_data = 0;


/*----public functions-----*/
//void ReadSupplyChannel(void)
//{
//   if(HAL_ADC_Start_DMA(&hadc_supplysense,(uint32_t *)adcValBuff,3) != HAL_OK)
//   {
////      _Error_Handler(__FILE__, __LINE__);
//   }
//}

void InitBattChargePin(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
  
  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  
  /*Configure Digital Input Ignition pin */
  GPIO_InitStruct.Pin = BATTERY_PGOOD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;   
  HAL_GPIO_Init(BATTERY_CHG_GPIO_Port, &GPIO_InitStruct);
}

/**
  * Enable DMA controller clock
  */
void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/**
 *  @brief Initialize the battery sense port by Registering an ADC sampling function to register a 
 *  callback function which will be invoked after ADC has sampled count times. ADC sampling parameter 
 *  initialization, Call Ql_ADC_Init function to set the sample counts and the interval of each sample
 *  Start/stop ADC sampling, Use Ql_ADC_Sampling function with an enable parameter to start ADC sampling, 
 *  and then ADC callback function will be invoked cyclically to report the ADC value. Again call this 
 *  API function with a disable parameter may stop the ADC sampling.
 *  @return void
 */

void InitSupplySenseADCPorts(void)
{
	
	 /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc_supplysense.Instance 							    = ADC1;
  hadc_supplysense.Init.ClockPrescaler 		    = ADC_CLOCK_ASYNC_DIV1;
  hadc_supplysense.Init.Resolution				    = ADC_RESOLUTION_12B;
  hadc_supplysense.Init.DataAlign 				    = ADC_DATAALIGN_RIGHT;
  hadc_supplysense.Init.ScanConvMode          = ADC_SCAN_ENABLE;
  hadc_supplysense.Init.EOCSelection          = ADC_EOC_SINGLE_CONV;
  hadc_supplysense.Init.LowPowerAutoWait      = DISABLE;
  hadc_supplysense.Init.LowPowerAutoPowerOff  = DISABLE;
  hadc_supplysense.Init.ContinuousConvMode    = DISABLE;
  hadc_supplysense.Init.NbrOfConversion       = 4;
  hadc_supplysense.Init.DiscontinuousConvMode = ENABLE;
  hadc_supplysense.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
  hadc_supplysense.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc_supplysense.Init.DMAContinuousRequests = DISABLE;
  hadc_supplysense.Init.Overrun 							= ADC_OVR_DATA_OVERWRITTEN;
  hadc_supplysense.Init.SamplingTimeCommon1   = ADC_SAMPLETIME_1CYCLE_5;
  hadc_supplysense.Init.SamplingTimeCommon2   = ADC_SAMPLETIME_160CYCLES_5;
  hadc_supplysense.Init.OversamplingMode      = DISABLE;
  hadc_supplysense.Init.TriggerFrequencyMode  = ADC_TRIGGER_FREQ_HIGH;
	
  if (HAL_ADC_Init(&hadc_supplysense) != HAL_OK)
  {
    //Error_Handler();
  }

  /** Configure Regular Channel
  */
	//VSUPPLY SENSE
  sConfig.Channel         = ADC_CHANNEL_0;
  sConfig.Rank            = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime    = ADC_SAMPLINGTIME_COMMON_2;
  if (HAL_ADC_ConfigChannel(&hadc_supplysense, &sConfig) != HAL_OK)
  {
    //Error_Handler();
  }
	
	//VBAT SENSE
  sConfig.Channel         = ADC_CHANNEL_1;
  sConfig.Rank            = ADC_REGULAR_RANK_2;
  sConfig.SamplingTime    = ADC_SAMPLINGTIME_COMMON_2;
  if (HAL_ADC_ConfigChannel(&hadc_supplysense, &sConfig) != HAL_OK)
  {
    //Error_Handler();
  }
	
	//MCU_TEMP
  sConfig.Channel         = ADC_CHANNEL_2;
  sConfig.Rank            = ADC_REGULAR_RANK_3;
  sConfig.SamplingTime    = ADC_SAMPLINGTIME_COMMON_2;
  if (HAL_ADC_ConfigChannel(&hadc_supplysense, &sConfig) != HAL_OK)
  {
    //Error_Handler();
  }
//	
//	
//	//ANALOG IP2
//  sConfig.Channel         = ADC_CHANNEL_3;
//  sConfig.Rank            = ADC_REGULAR_RANK_4;
//  sConfig.SamplingTime    = ADC_SAMPLINGTIME_COMMON_2;
//  if (HAL_ADC_ConfigChannel(&hadc_supplysense, &sConfig) != HAL_OK)
//  {
//    //Error_Handler();
//  }

	
	//VREFINT SENSE
  sConfig.Channel         = ADC_CHANNEL_VREFINT;
  sConfig.Rank            = ADC_REGULAR_RANK_4;
  sConfig.SamplingTime    = ADC_SAMPLINGTIME_COMMON_2;
  if (HAL_ADC_ConfigChannel(&hadc_supplysense, &sConfig) != HAL_OK)
  {
    //Error_Handler();
  }
	
  /* USER CODE BEGIN ADC1_Init 2 */
 /* Run the ADC calibration */
  if (HAL_ADCEx_Calibration_Start(&hadc_supplysense) != HAL_OK)
  {
    /* Calibration Error */
    //Error_Handler();
  }
  /* USER CODE END ADC1_Init 2 */
	
	/*## Start ADC conversions ###############################################*/
  /* Start ADC group regular conversion with DMA */
  if (HAL_ADC_Start_DMA(&hadc_supplysense,
                        (uint32_t *)aADCxConvertedData,
                        ADC_CONVERTED_DATA_BUFFER_SIZE
                       ) != HAL_OK)
  {
    /* ADC conversion start error */
    //Error_Handler();
  }  
	  
	
	/* USER CODE BEGIN 2 */
//  for (tmp_index_adc_converted_data = 0; tmp_index_adc_converted_data < ADC_CONVERTED_DATA_BUFFER_SIZE; tmp_index_adc_converted_data++)
//  {
//    aADCxConvertedData[tmp_index_adc_converted_data] = VAR_CONVERTED_DATA_INIT_VALUE;
//  }
    /* Run the ADC calibration in single-ended mode */  
//  if (HAL_ADCEx_Calibration_Start(&hadc_supplysense, ADC_SINGLE_ENDED) != HAL_OK)
//  {
//    /* Calibration Error */
////    _Error_Handler(__FILE__, __LINE__);
//  }
      
   // ReadSupplyChannel();
}

void TriggerSupplyADC(void)
{
/* Start ADC conversion */
    /* Since sequencer is enabled in discontinuous mode, this will perform    */
    /* the conversion of the next rank in sequencer.                          */
    /* Note: For this example, conversion is triggered by software start,     */
    /*       therefore "HAL_ADC_Start()" must be called for each conversion.  */
    /*       Since DMA transfer has been initiated previously by function     */
    /*       "HAL_ADC_Start_DMA()", this function will keep DMA transfer      */
    /*       active.                                                          */
    if (HAL_ADC_Start(&hadc_supplysense) != HAL_OK)
    {
     // Error_Handler(); 
    }
}




uint16_t ReadAdcSupplyValue(void)
{
  return (uint16_t)aADCxConvertedData[0];
}

uint16_t ReadAdcBatValue(void)
{
  return (uint16_t)aADCxConvertedData[1];
}

uint16_t ReadTempSensorValue(void)
{
  return (uint16_t)aADCxConvertedData[2];
}

//uint16_t ReadAnalogIp2Value(void)
//{
//  return (uint16_t)aADCxConvertedData[3];
//}

uint16_t ReadAdcVrefIntValue(void)
{
  return (uint16_t)aADCxConvertedData[4];
}

/**
 *  @brief This function checks whether Charger is connected by monitoring the adc_pinlevel, 
 *  if it is high it returns 1 else it returns 0
 *  @return Boolean 1 or 0
 */
BOOL IsChargerConnected(void)
{
    if(HAL_GPIO_ReadPin(BATTERY_CHG_GPIO_Port,BATTERY_PGOOD_Pin) == GPIO_PIN_RESET)
//	if(battVoltage > 4200)
	{
		return True;
	}

        return False;
}
/**
 *  @brief Brief
 *  @return Boolean 1 or 0
 */
BOOL IsChargingFull(void)
{
	 return False;
}

/**
  * @brief  Conversion complete callback in non blocking mode 
  * @param  hadc: ADC handle
  * @note   This example shows a simple way to report end of conversion
  *         and get conversion result. You can add your own implementation.
  * @retval None
  */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
  /* Update status variable of DMA transfer */
  ubDmaTransferStatus = 1;  

  /* Set LED depending on DMA transfer status */
  /* - Turn-on if DMA transfer is completed */
  /* - Turn-off if DMA transfer is not completed */
  //BSP_LED_On(LED4);
}

/**
  * @brief  Conversion DMA half-transfer callback in non blocking mode 
  * @note   This example shows a simple way to report end of conversion
  *         and get conversion result. You can add your own implementation.
  * @retval None
  */
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{
  /* Set LED depending on DMA transfer status */
  /* - Turn-on if DMA transfer is completed */
  /* - Turn-off if DMA transfer is not completed */
  //BSP_LED_Off(LED4);
}

/**
  * @brief  ADC error callback in non blocking mode
  *        (ADC conversion with interruption or transfer by DMA)
  * @param  hadc: ADC handle
  * @retval None
  */
void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
  /* In case of ADC error, call main error handler */
  //Error_Handler();
}