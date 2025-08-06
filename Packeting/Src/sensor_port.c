/**
 *  @file          :  sensor_port.c
 *  @author        :  Aakash/Ratna
 *  @date          :  15/06/2017
 *  @brief         :  Includes APIs for serial port and analog sensor initialization and acquire sensor data.
 *  @filerevision  :  1.0
 *  
 */

 
 /*----includes-----*/
#include "debug_log.h"
#include "sensor_port.h"


 /*----variables-----*/
uint32_t sensorDataBuff[3] = {0,0,0};   //4

ADC_HandleTypeDef hadc_sensorsense;
DMA_HandleTypeDef hdma_adc1;

// /*----private functions-----*/
void ReadAnalogChannels(void)
{
  if(HAL_ADC_Start_DMA(&hadc_sensorsense, (uint32_t *)sensorDataBuff, 3) != HAL_OK)//4
  {
     //      _Error_Handler(__FILE__, __LINE__);
  }  
}

void InitAnalogADCPort(void)
{
    ADC_MultiModeTypeDef multimode;
    ADC_ChannelConfTypeDef sConfig;

    /**Common config    */
    hadc_sensorsense.Instance                   = ADC1;
    hadc_sensorsense.Init.ClockPrescaler        = ADC_CLOCK_ASYNC_DIV1;
    hadc_sensorsense.Init.Resolution            = ADC_RESOLUTION_12B;
    hadc_sensorsense.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
    hadc_sensorsense.Init.ScanConvMode          = ADC_SCAN_ENABLE;
    hadc_sensorsense.Init.EOCSelection          = ADC_EOC_SINGLE_CONV;
    hadc_sensorsense.Init.LowPowerAutoWait      = DISABLE;
    hadc_sensorsense.Init.ContinuousConvMode    = DISABLE;
    hadc_sensorsense.Init.NbrOfConversion       = 3;   //4
    hadc_sensorsense.Init.DiscontinuousConvMode = ENABLE;
    hadc_sensorsense.Init.NbrOfDiscConversion   = 1;
    hadc_sensorsense.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
    hadc_sensorsense.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc_sensorsense.Init.DMAContinuousRequests = ENABLE; 
    hadc_sensorsense.Init.Overrun               = ADC_OVR_DATA_OVERWRITTEN;
    
    if (HAL_ADC_Init(&hadc_sensorsense) != HAL_OK)
    {
//      _Error_Handler(__FILE__, __LINE__);
    }
    
     /**Configure the ADC multi-mode 
    */
      multimode.Mode = ADC_MODE_INDEPENDENT;
      if (HAL_ADCEx_MultiModeConfigChannel(&hadc_sensorsense, &multimode) != HAL_OK)
      {
//        _Error_Handler(__FILE__, __LINE__);
      }

      /**Configure Analog Channels       */
    sConfig.Channel      = ANALOG_INPUT_CH1;    //PC0     ------> ADC1_IN6
    sConfig.Rank         = ADC_REGULAR_RANK_1;
    sConfig.SingleDiff   = ADC_SINGLE_ENDED;
    sConfig.SamplingTime = ADC_SAMPLETIME_601CYCLES_5;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset = 0;
    
    if (HAL_ADC_ConfigChannel(&hadc_sensorsense, &sConfig) != HAL_OK)
    {
//      _Error_Handler(__FILE__, __LINE__);
    }
    
    sConfig.Channel      = ANALOG_INPUT_CH2;    //PC1     ------> ADC1_IN7
    sConfig.Rank         = ADC_REGULAR_RANK_2;
    sConfig.SingleDiff   = ADC_SINGLE_ENDED;
    sConfig.SamplingTime = ADC_SAMPLETIME_181CYCLES_5;
//    sConfig.OffsetNumber = ADC_OFFSET_NONE;
//    sConfig.Offset = 0;
    if (HAL_ADC_ConfigChannel(&hadc_sensorsense, &sConfig) != HAL_OK)
    {
//      _Error_Handler(__FILE__, __LINE__);
    }
    
    /**Configure Emergency Channels 
      */
    sConfig.Channel      = ANALOG_EMGR_CH1;    //PC2     ------> ADC1_IN8 - EMERGENCY 
    sConfig.Rank         = ADC_REGULAR_RANK_3;   //3
    sConfig.SingleDiff   = ADC_SINGLE_ENDED;
    sConfig.SamplingTime = ADC_SAMPLETIME_61CYCLES_5;
//    sConfig.OffsetNumber = ADC_OFFSET_NONE;
//    sConfig.Offset = 0;
    
    if (HAL_ADC_ConfigChannel(&hadc_sensorsense, &sConfig) != HAL_OK)
    {
//      _Error_Handler(__FILE__, __LINE__);
    }
    
      /* Run the ADC calibration in single-ended mode */  
  if (HAL_ADCEx_Calibration_Start(&hadc_sensorsense, ADC_SINGLE_ENDED) != HAL_OK)
  {
    /* Calibration Error */
//    _Error_Handler(__FILE__, __LINE__);
  }
  
    ReadAnalogChannels();
    
//    sConfig.Channel      = ANALOG_EMGR_CH2;   //PC3     ------> ADC1_IN9
//    sConfig.Rank         = ADC_REGULAR_RANK_4;
//    sConfig.SingleDiff   = ADC_SINGLE_ENDED;
//    sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
//    sConfig.OffsetNumber = ADC_OFFSET_NONE;
//    sConfig.Offset = 0;
//    
//    if (HAL_ADC_ConfigChannel(&hadc_sensorsense, &sConfig) != HAL_OK)
//    {
//      _Error_Handler(__FILE__, __LINE__);
//    }
    
//    HAL_ADC_Start(&hadc_sensorsense);
//    HAL_ADC_Start_DMA(&hadc_sensorsense, sensorDataBuff, 4);
}


/*----public functions-----*/
void TriggerSensorADC(void)
{
    /* Start ADC conversion */
    /* Since sequencer is enabled in discontinuous mode, this will perform    */
    /* the conversion of the next rank in sequencer.                          */
    /* Note: For this example, conversion is triggered by software start,     */
    /*       therefore "HAL_ADC_Start()" must be called for each conversion.  */
    /*       Since DMA transfer has been initiated previously by function     */
    /*       "HAL_ADC_Start_DMA()", this function will keep DMA transfer      */
    /*       active.                                                          */
    HAL_ADC_Start(&hadc_sensorsense);
      
    /* Wait for conversion completion before conditional check hereafter */
    HAL_ADC_PollForConversion(&hadc_sensorsense, 1);
}

 
void ReadSensorValues(uint32_t* sensorAdcValPtr)
{
  memcpy(sensorAdcValPtr,sensorDataBuff,sizeof(sensorDataBuff) );  //4
}


