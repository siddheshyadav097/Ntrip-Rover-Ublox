#include "buzzer_app.h"
#include "debug_log.h"

//TIM_HandleTypeDef buzzerTimer;
TIM_HandleTypeDef htim3;

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

void BuzzerInit(void)
{
// /* USER CODE BEGIN TIM1_Init 0 */

//  /* USER CODE END TIM1_Init 0 */

//  TIM_MasterConfigTypeDef sMasterConfig = {0};
//  TIM_OC_InitTypeDef sConfigOC = {0};
//  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

//  /* USER CODE BEGIN TIM1_Init 1 */

//  /* USER CODE END TIM1_Init 1 */
//  htim3.Instance = TIM3;
//  htim3.Init.Prescaler = PRESCALER_VALUE;
//  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
//  htim3.Init.Period = PERIOD_VALUE;
//  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
//  htim3.Init.RepetitionCounter = 0;
//  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
//  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
//  {
//   // Error_Handler();
//  }
//  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
//  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
//  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
//  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
//  {
//    //Error_Handler();
//  }
//  sConfigOC.OCMode = TIM_OCMODE_PWM1;
//  sConfigOC.Pulse = PULSE1_VALUE;
//  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
//  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
//  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
//  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
//  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
// 
//  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
//  {
//    //Error_Handler();
//  }
// 
//  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
//  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
//  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
//  sBreakDeadTimeConfig.DeadTime = 0;
//  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
//  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
//  sBreakDeadTimeConfig.BreakFilter = 0;
//  sBreakDeadTimeConfig.BreakAFMode = TIM_BREAK_AFMODE_INPUT;
//  sBreakDeadTimeConfig.Break2State = TIM_BREAK2_DISABLE;
//  sBreakDeadTimeConfig.Break2Polarity = TIM_BREAK2POLARITY_HIGH;
//  sBreakDeadTimeConfig.Break2Filter = 0;
//  sBreakDeadTimeConfig.Break2AFMode = TIM_BREAK_AFMODE_INPUT;
//  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
//  if (HAL_TIMEx_ConfigBreakDeadTime(&htim3, &sBreakDeadTimeConfig) != HAL_OK)
//  {
//    //Error_Handler();
//  }
//  /* USER CODE BEGIN TIM1_Init 2 */

//  /* USER CODE END TIM1_Init 2 */
//  HAL_TIM_MspPostInit(&htim3);
//   

//      
//    GPIO_InitTypeDef GPIO_InitStruct;
//    
//  /* GPIO Ports Clock Enable */
//    __HAL_RCC_GPIOC_CLK_ENABLE();
//  /*Configure GPIO pin Input Level */
//    GPIO_InitStruct.Pin = BUZZER_ENABLE_PIN;
//    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//    GPIO_InitStruct.Pull = GPIO_NOPULL;
//    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//    HAL_GPIO_Init(BUZZER_ENABLE_PORT, &GPIO_InitStruct);
//    
////#if defined (ELARA_PLUS_IOT) ||  defined (ELARA_PLUS_IOT_UVLED)   
//  /*Configure GPIO pin Input Level */
////    GPIO_InitStruct.Pin = GPIO_PIN_11;
////    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
////    GPIO_InitStruct.Pull = GPIO_NOPULL;
////    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
////    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
////#endif
//   
////    DisableBuzzer();
}

static uint32_t buzzerControlTick = 0;
 beepType_t currentBeep = BEEP_OFF;
static uint8_t beepOnState = 0;                     // beep off
uint8_t errorSoundCount = 0, errorBuzzerCount = 0;


void SetBuzzerBeep(beepType_t beep)
{
//    if(currentBeep == beep)
//    {
//        return; 
//    }
//    currentBeep = beep;
//    
//    switch(currentBeep)
//    {
//        case BEEP_OFF:
//          StopBuzzerPWM();
//          //LOG_INFO(CH_GSM,"BUZZER OFF ");
//          break;
//         
//        case BEEP_ON:
//          BUZZER_SetHigh();      //buzzer on
//          StartBuzzerPWM();
//          buzzerControlTick = GetStartTime();
//          
//          break;  
//          
//        case BEEP_DIMMING:
//          BUZZER_SetLow();      //buzzer off
//          buzzerControlTick = GetStartTime();
//          break;
//          
//        case KEY_DENIED_BEEP:  
//          errorSoundCount = 0;
//          break;
//          
//        case BUZZER_ERROR:
//          errorBuzzerCount = 0;
//          LOG_INFO(CH_GSM,"BUZZER ERROR ");
//          break;  
//          
//        case SINGLE_BEEP:
//          buzzerControlTick = GetStartTime();
//          beepOnState = 0;
//          BUZZER_SetHigh();      //buzzer on
//          StartBuzzerPWM();
//          break;
//          
//        case LQC_BEEP:
//          BUZZER_SetHigh();      //buzzer on
//          StartBuzzerPWM();
//          break;
//    }
}

uint8_t buzzerFlag = 0;
//extern displayFault_et curFaultState;
void BuzzerControlHandler(void)
{
////  if((GetSystemFailureALert() == 1) && (buzzerFlag == 0))
////  {
////    buzzerFlag = 1;
////    SetBuzzerBeep(BUZZER_ERROR);
////  }
////  else if((GetSystemFailureALert() == 0) && (buzzerFlag == 1))
////  {
////    buzzerFlag = 0;
////    SetBuzzerBeep(BEEP_OFF);
////  }
////  if(((curFaultState != NO_ERROR)  || (GetRoLife() ==  RO_LIFE_EOL_TDS) || (GetFilterLife() == FILTER_LIFE_EOL) || \
////    (GetFilterLife() == FILTER_LIFE_NEL) || (GetRoLife() ==  RO_LIFE_EOL_FR) || (GetFilterLife() == FILTER_LIFE_BAD_PUMP)) || (GetUvLedState() == UV_LED_ERROR) && (buzzerFlag == 0))
////  {
////    buzzerFlag = 1;
////    SetBuzzerBeep(BUZZER_ERROR);
////  }
////  if((curFaultState == NO_ERROR) && (buzzerFlag == 1))
////  {
////    buzzerFlag = 0;
////    SetBuzzerBeep(BEEP_OFF);
////  }
//    switch(currentBeep)
//    {
//        case BEEP_OFF:
//          
//          break;
//          
//        case BEEP_ON:
//          if((TimeSpent(buzzerControlTick,2000)) )
//          {
//                SetBuzzerBeep(BEEP_DIMMING);
//          }

//          break;  
//          
//        case BEEP_DIMMING:
//          if(TimeSpent(buzzerControlTick,600))
//          {
//                SetBuzzerBeep(BEEP_OFF);
//          }
//          break;
//          
//        case KEY_DENIED_BEEP:  
//          if(errorSoundCount < 2)
//            {
//                if(TimeSpent(buzzerControlTick,80))
//                {
//                    if(beepOnState)
//                    {           
//                        errorSoundCount++;
//                        beepOnState = 0;         
//                        BUZZER_SetLow();
//                        StopBuzzerPWM();
//                    }
//                    else
//                    {   
//                        beepOnState = 1;                       
//                        BUZZER_SetHigh();
//                        StartBuzzerPWM();
//                    }
//                    buzzerControlTick = GetStartTime();
//                }
//            }
//            else
//            {
//                SetBuzzerBeep(BEEP_OFF);
//            }
//          break;
//          
//        case BUZZER_ERROR:
//            if(errorBuzzerCount <= 9)
//            {
//                if(TimeSpent(buzzerControlTick,1000))
//                {
//                    if(beepOnState)
//                    {           
//                        errorBuzzerCount++;
//                        beepOnState = 0;         
//                        BUZZER_SetLow();
//                        StopBuzzerPWM();
//                    }
//                    else
//                    {   
//                        beepOnState = 1;                       
//                        BUZZER_SetHigh();
//                        StartBuzzerPWM();
//                    }
//                    buzzerControlTick = GetStartTime();
//                }
//            }
//            else
//            {
//                SetBuzzerBeep(BEEP_OFF);
//            }
//                break;        
//            
//        case SINGLE_BEEP:
//          if(TimeSpent(buzzerControlTick,SINGLE_500MSEC_BEEP_ON_TIME_MS))
//          {
//                SetBuzzerBeep(BEEP_OFF);//BEEP_DIMMING);
//          }
//          break;
//        
//    case LQC_BEEP:
//      break;
//    }
}
//uint32_t buzzerOnTick = 0;
uint8_t buzzerOnFlag = 0;
void StartBuzzerPWM(void)
{
//#if defined (ELARA_PLUS_IOT) ||  defined (ELARA_PLUS_IOT_UVLED)
//  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_11,GPIO_PIN_SET);
//#endif
//    
//#if defined (ELARA_PLUS_BASE)
    if (HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2) != HAL_OK)
    {

    }
//#endif
}



void StopBuzzerPWM(void)
{
  
////    HAL_TIM_PWM_Stop(&buzzerTimer, TIM_CHANNEL_4);
//#if defined (ELARA_PLUS_IOT) ||  defined (ELARA_PLUS_IOT_UVLED)
//     HAL_GPIO_WritePin(GPIOA,GPIO_PIN_11,GPIO_PIN_RESET);
//#endif
//    
//#if defined (ELARA_PLUS_BASE)
    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_2);
//#endif
  
}