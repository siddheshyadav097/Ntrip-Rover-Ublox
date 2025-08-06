#ifndef __INPUT_HANDLER_H
#define __INPUT_HANDLER_H

 #include "stm32g0xx_hal.h"



#define NUM_TOTAL_INPUTS		2

typedef enum
{
    INPUT_OFF,
    INPUT_ON
}inputState_e;

typedef inputState_e (*getKeyValFn_t)(void);

typedef struct
{
    getKeyValFn_t getKeyFnPtr;
    uint16_t onTimeInMs;
    uint16_t offTimeInMs;
    uint8_t filterTimeInMs;
}input_s;

uint8_t AddInput(const input_s *input);
inputState_e GetInputState(uint8_t keyId);
void InputHandler(void);


#endif