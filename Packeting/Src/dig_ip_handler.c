#include "dig_ip_handler.h"
#include "qtimespent.h"

typedef struct
{
    const input_s  *input;
    inputState_e filteredInput;
    inputState_e currentState;
    uint8_t inputOnCounter;
    uint8_t inputOffCounter;
    uint32_t startTick;
}inputHandle_s;


inputHandle_s inputHdl[NUM_TOTAL_INPUTS];
uint32_t inputStartTick = 0;
static uint8_t numInputs = 0;

/**
 *  @brief  : add input to initialise the key handle
 *  @param  : pointer of input 
 *  @return : key id will always be less than NUM_TOTAL_Q_INPUTS 
 *            returns 0xff when error adding key
 */
uint8_t AddInput(const input_s *input)
{
    if(numInputs < NUM_TOTAL_INPUTS)
    {
        inputHdl[numInputs].input = input;
	    inputHdl[numInputs].filteredInput = INPUT_OFF;
        inputHdl[numInputs].currentState = INPUT_OFF;
	    inputHdl[numInputs].inputOnCounter = 0;
	    inputHdl[numInputs].inputOffCounter = 0;
        inputHdl[numInputs].startTick = GetStartTime();
        numInputs++;
        return(numInputs-1);
    }
    return 0xff;
}

inputState_e GetInputState(uint8_t keyId)
{
	return inputHdl[keyId].currentState;
}

inputState_e inputState;
// this function should be called in while loop
void InputHandler(void)
{
	uint8_t i;
    if(TimeSpent(inputStartTick,1))
	{
		for(i = 0; i < NUM_TOTAL_INPUTS; i++)
		{
			inputState = inputHdl[i].input->getKeyFnPtr();
			if(inputState == INPUT_ON)
			{
				inputHdl[i].inputOnCounter++;
				if(inputHdl[i].inputOnCounter > inputHdl[i].input->filterTimeInMs)
				{
					inputHdl[i].inputOnCounter = 0;
					inputHdl[i].inputOffCounter = 0;
					inputHdl[i].filteredInput = INPUT_ON;
				}
			}
			else
			{
				inputHdl[i].inputOffCounter++;
				if(inputHdl[i].inputOffCounter > inputHdl[i].input->filterTimeInMs)
				{
					inputHdl[i].inputOnCounter = 0;
					inputHdl[i].inputOffCounter = 0;
					inputHdl[i].filteredInput = INPUT_OFF;
				}
			}
			
			switch(inputHdl[i].currentState)
			{
				case INPUT_ON:
					if(inputHdl[i].filteredInput == INPUT_OFF)
					{
						if(TimeSpent(inputHdl[i].startTick,inputHdl[i].input->offTimeInMs))
						{
							inputHdl[i].currentState = INPUT_OFF;
							inputHdl[i].startTick = GetStartTime();
						}
					}
					else
					{
						inputHdl[i].startTick = GetStartTime();
					}
				break;
				
				case INPUT_OFF:
					if(inputHdl[i].filteredInput == INPUT_ON)
					{
						if(TimeSpent(inputHdl[i].startTick,inputHdl[i].input->onTimeInMs))
						{
							inputHdl[i].currentState = INPUT_ON;
							inputHdl[i].startTick = GetStartTime();
						}
					}
					else
					{
						inputHdl[i].startTick = GetStartTime();
					}
				break;
			}
		}
	}
}