#ifndef __BUZZER_APP_H
#define __BUZZER_APP_H

#include "stm32g0xx_hal.h"
#include "qtimespent.h"

#define SINGLE_500MSEC_BEEP_ON_TIME_MS  300//500       
#define SINGLE_20MSEC_BEEP_ON_TIME_MS   20


//#define         BUZZER_ENABLE_PIN       GPIO_PIN_6
//#define         BUZZER_ENABLE_PORT      GPIOC

#define         BUZZER_SetHigh()         HAL_GPIO_WritePin(BUZZER_ENABLE_PORT, BUZZER_ENABLE_PIN, GPIO_PIN_SET)
#define         BUZZER_SetLow()          HAL_GPIO_WritePin(BUZZER_ENABLE_PORT, BUZZER_ENABLE_PIN, GPIO_PIN_RESET)

/* Compute the prescaler value to have TIM1 counter clock equal to 64000000 Hz */

#define PRESCALER_VALUE     (uint32_t)(((SystemCoreClock) / 64000000) - 1)

/* -----------------------------------------------------------------------
TIM1 Configuration: generate 4 PWM signals with 4 different duty cycles.

    In this example TIM1 input clock (TIM1CLK) is set to APB1 clock (PCLK1),
    since APB1 prescaler is equal to 1.
      TIM1CLK = PCLK1
      PCLK1 = HCLK
      => TIM1CLK = HCLK = SystemCoreClock

    To get TIM1 counter clock at SystemCoreClock, the prescaler is set to 0

    To get TIM1 output clock at 24 KHz, the period (ARR)) is computed as follows:
       ARR = (TIM1 counter clock / TIM1 output clock) - 1
           = 2666

    TIM1 Channel1 duty cycle = (TIM1_CCR1/ TIM1_ARR + 1)* 100 = 50%
    TIM1 Channel2 duty cycle = (TIM1_CCR2/ TIM1_ARR + 1)* 100 = 37.5%
    TIM1 Channel3 duty cycle = (TIM1_CCR3/ TIM1_ARR + 1)* 100 = 25%
    TIM1 Channel4 duty cycle = (TIM1_CCR4/ TIM1_ARR + 1)* 100 = 12.5%

    Note:
     SystemCoreClock variable holds HCLK frequency and is defined in system_stm32g0xx.c file.
     Each time the core clock (HCLK) changes, user had to update SystemCoreClock
     variable value. Otherwise, any configuration based on this variable will be incorrect.
     This variable is updated in three ways:
      1) by calling CMSIS function SystemCoreClockUpdate()
      2) by calling HAL API function HAL_RCC_GetSysClockFreq()
      3) each time HAL_RCC_ClockConfig() is called to configure the system clock frequency
  ----------------------------------------------------------------------- */

/* Initialize TIMx peripheral as follows:
   + Prescaler = (SystemCoreClock / 64000000) - 1
   + Period = (2667 - 1)
   + ClockDivision = 0
   + Counter direction = Up
*/
#define  PERIOD_VALUE       (uint32_t)(14221 - 1)//FOR 3 KHZ(uint32_t)(2667 - 1)              /* Period Value  */
#define  PULSE1_VALUE       (uint32_t)(14221 / 2)              /* Capture Compare 1 Value  */
//#define  PULSE2_VALUE       (uint32_t)(2667 * 37.5 / 100)     /* Capture Compare 2 Value  */
//#define  PULSE3_VALUE       (uint32_t)(2667 / 4)              /* Capture Compare 3 Value  */
//#define  PULSE4_VALUE       (uint32_t)(2667 * 12.5 /100)      /* Capture Compare 4 Value  */

typedef enum
{
    BEEP_OFF,
    BEEP_ON,
    BEEP_DIMMING,
    KEY_DENIED_BEEP,
    BUZZER_ERROR,
    SINGLE_BEEP,
    LQC_BEEP
}beepType_t;

void BuzzerInit(void);
void BuzzerControlHandler(void);
void SetBuzzerBeep(beepType_t beep);
void StartBuzzerPWM(void);
void StopBuzzerPWM(void);
void BuzzerLoop(void);


#endif